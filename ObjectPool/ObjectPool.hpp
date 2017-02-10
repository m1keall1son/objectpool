#pragma once
#include <stdint.h>
#include <type_traits>
#include <memory>
#include <functional>
#include <limits>
#include <array>

class IObjectPool {
public:

	using SerialNumber = size_t;
	using IndirectionBlock = uint8_t;
	using IndirectionIndex = uint32_t;

	IObjectPool() = default;
	virtual ~IObjectPool() = default;

	virtual std::weak_ptr<IObjectPool> getWeakPtr() = 0;
	virtual void destroyObject(void*) = 0;

};

class Handle {
public:

	Handle() = default;
	~Handle() = default;

	Handle(Handle&&) = default;
	Handle& operator=(Handle&&) = default;

	Handle(const Handle&) = default;
	Handle& operator=(const Handle&) = default;

	//created by pool
	Handle(void* object, IObjectPool::SerialNumber serial, const std::weak_ptr<IObjectPool>& pool) :mObject(object), mSerialNumber(serial), mPool(pool) {}

	//created by T
	Handle( void* data_ptr ) {
		auto pool_ptr = reinterpret_cast<IObjectPool*>(data_ptr);
		mPool = (--pool_ptr)->getWeakPtr();
		auto indirection_index_ptr = reinterpret_cast<IObjectPool::IndirectionIndex*>(pool_ptr);
		auto indirection_block_ptr = reinterpret_cast<IObjectPool::IndirectionBlock*>(--indirection_index_ptr);
		auto serial_ptr = reinterpret_cast<IObjectPool::IndirectionBlock*>(--indirection_block_ptr);
		mSerialNumber = *--serial_ptr;
		mObject = serial_ptr;
	}

	bool operator==(const Handle& rhs) {
		return mSerialNumber == rhs.mSerialNumber && rhs.mObject == rhs.mObject;
	}

	const bool isInitialized() const { return mSerialNumber > 0; }
	const bool isValid() const {
		if (mObject) {
			auto serial = *reinterpret_cast<IObjectPool::SerialNumber*>(mObject);
			return mSerialNumber == serial;
		}
		else
			return false;
	}

	template<typename T>
	inline T* get() const {
		if (!mObject)return nullptr;
		auto obj = static_cast<typename ObjectPool<T>::Object*>(mObject);
		mSerialNumber == obj->serial ? return obj->data : return nullptr;
	}

	bool destroy() {
		if (!isInitialized() || !isValid() || !mObject)return false;
		if (auto pool = mPool.lock()) {
			pool->destroyObject(mObject);
			reset();
			return true;
		}
		else {
			return false;
		}
	}

	void reset() { mObject = nullptr; mSerialNumber = 0; mPool.reset(); }

private:
	void* mObject{ nullptr };
	IObjectPool::SerialNumber mSerialNumber{ 0 };
	std::weak_ptr<IObjectPool> mPool;
};

template<size_t test>
struct IsPowerOf2 {
	constexpr static const bool value = ((test != 0) && !(test & (test - 1)));
};

template<typename T, size_t blocksize = 4096, typename = std::enable_if_t< IsPowerOf2<blocksize>::value > >
class ObjectPool : public IObjectPool, public std::enable_shared_from_this<ObjectPool<T, blocksize>> {

	struct Object {
		//LOOKUP
		SerialNumber serial{ 0 };
		IndirectionBlock block_id{0};
		IndirectionIndex data_index{0};
		IObjectPool* pool; //used by T to create handles...don't worry, i hate this too
		//SLOT
		T data;
		Object* lookup{nullptr};
	};

public:

	constexpr static const size_t BLOCK_SIZE = blocksize;
	constexpr static const size_t OBJECTS_PER_BLOCK = BLOCK_SIZE / sizeof(Object);
	constexpr static const size_t OBJECT_STRIDE = sizeof(Object);
	constexpr static const size_t MAX_BLOCKS = std::numeric_limits<IObjectPool::IndirectionBlock>::max;
	constexpr static const size_t MAX_OBJECTS = OBJECTS_PER_BLOCK*MAX_BLOCKS;

private:

	struct MemoryBlock {
		MemoryBlock() : block(::operator new(BLOCK_SIZE)) { memset(block, 0, BLOCK_SIZE); }
		~MemoryBlock() {
			::operator delete(block);
		}

		Object& operator[](size_t index) {
			return reinterpret_cast<Object*>(block)[index];
		}

		void* block{ nullptr };
	};

public:

	static std::shared_ptr<ObjectPool<T, blocksize>> create() { return std::shared_ptr<ObjectPool<T, blocksize>>(new ObjectPool<T, blocksize>); }

	template<typename...Args>
	Handle createObject(Args...args) {

		Object * lookup;

		IndirectionBlock block_id = floor(mBack / OBJECTS_PER_BLOCK);
		IndirectionIndex data_index = mBack % OBJECTS_PER_BLOCK;

		if (block_id > MAX_BLOCKS) {
			throw std::bad_alloc();
		} else if (block_id > mNumBlocks) {
			mBlocks[mNumBlocks++] = new MemoryBlock;
		}

		auto & next_slot = mBlocks[block_id]->operator[](data_index);
		next_slot.pool = this;

		//check if we can reuse a deleted spot 
		if (mDestructionOffset > 0){
			//use the available lookup
			lookup = next_slot.lookup;
			//bring the offset down
			--mDestructionOffset;
		}
		else {
			//use the never before used lookup in this slots memory location
			lookup = &next_slot;
			//enable handles with a serial of 1
			++next_slot.serial;
			//save the location of this lookup in the slot for use later
			next_slot.lookup = lookup;
		}

		//construct new T from last available slot
		
		lookup->block_id = block_id;
		lookup->data_index = data_index;

		new(&next_slot.data) T(args...);

		//notify creation handler
		if (mOnCreateHandlerfn)
			mOnCreateHandlerfn(next_slot.data);

		++mBack;

		auto handle = Handle( lookup, lookup->serial, getWeakPtr() );

		return std::move(handle);

	}

	size_t size() const { return mBack; }

	T& operator[](size_t index) {

		if (index > mBack)
			throw std::out_of_range("attempting to access data from the pool beyond what's available");

		uint32_t block = floor( index / OBJECTS_PER_BLOCK);
		uint32_t block_index = index % OBJECTS_PER_BLOCK;

		return mBlocks[block]->operator[](block_index).data;
	}

	const T& operator[](size_t index)const {

		if (index > mBack)
			throw std::out_of_range("attempting to access data from the pool beyond what's available");

		uint32_t block = floor(index / OBJECTS_PER_BLOCK);
		uint32_t block_index = index % OBJECTS_PER_BLOCK;

		return mBlocks[block]->operator[](block_index).data;
	}

	void connectObjectCreationHandler(const std::function<void(const T&)>& fn) { mOnCreateHandlerfn = fn; }
	void connectObjectDestructionHandler(const std::function<void(const T&)>& fn) { mOnDestoryHandlerfn = fn; }

	~ObjectPool() { 
		for (int i = 0; i < mNumBlocks; i++)
			delete mBlocks[i];
	}

private:

	ObjectPool() {
		mBlocks[0] = new MemoryBlock;
	}

	std::weak_ptr<IObjectPool> getWeakPtr() override { return shared_from_this(); }

	void destroyObject(void* object) override {

		auto & obj = *static_cast<Object*>(object);
		
		//lookup data slot
		auto block = mBlocks[obj.block_id];
		auto & dead_slot = block->operator[](obj.data_index);

		//notify handler
		if (mOnDestoryHandlerfn)
			mOnDestoryHandlerfn(dead_slot.data);

		//disable any remaining handles
		++obj.serial;

		if ( obj.block_id*OBJECTS_PER_BLOCK + obj.data_index < mBack - 1) {

			//"swap and pop"

			auto & living_slot = mBlocks[floor((mBack - 1) / OBJECTS_PER_BLOCK)]->operator[]((mBack - 1) % OBJECTS_PER_BLOCK);
			auto & living_lookup = *living_slot.lookup;

			living_lookup.block_id = obj.block_id;
			living_lookup.data_index = obj.data_index;

			//swap living data to dead data's position so living data is tightly packed, and preseve lookup
			dead_slot.data = std::move(living_slot.data);
			dead_slot.lookup = std::move(living_slot.lookup);

			//store location of available lookup in "popped" data lookup
			living_slot.lookup = &obj;

			//destroy object
			living_slot.data.~T();
		}
		else {
			//no need to swap, just "pop"

			//store location of available lookup in "popped" data lookup
			dead_slot.lookup = &obj;

			//destroy object
			dead_slot.data.~T();
		}

		++mDestructionOffset;
		--mBack;
	}

	std::array< MemoryBlock*, MAX_BLOCKS > mBlocks;
	size_t mBack{ 0 };
	size_t mNumBlocks{ 1 };
	size_t mDestructionOffset{ 0 };
	std::function<void(const T&)> mOnCreateHandlerfn{ nullptr };
	std::function<void(const T&)> mOnDestoryHandlerfn{ nullptr };

	friend Handle;

};