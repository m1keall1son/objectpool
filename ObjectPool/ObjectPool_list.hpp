//#pragma once
//#include <type_traits>
//#include <memory>
//#include <functional>
//
//class IObjectPool {
//public:
//
//	using SerialNumber = size_t;
//
//	IObjectPool() = default;
//	virtual ~IObjectPool() = default;
//
//	virtual std::weak_ptr<IObjectPool> getWeakPtr() = 0;
//	virtual void destroyObject(void*) = 0;
//
//};
//
//class Handle {
//public:
//
//	Handle() = default;
//	~Handle() = default;
//
//	//created by pool
//	Handle(void* object, IObjectPool::SerialNumber serial, const std::weak_ptr<IObjectPool>& pool ):mObject(object), mSerialNumber(serial), mPool(pool) {}
//
//	//created by pooled type
//	Handle(void* data_ptr) { 
//		auto pool = reinterpret_cast<IObjectPool*>(data_ptr);
//		mPool = (--pool)->getWeakPtr();
//		auto serial_ptr = reinterpret_cast<IObjectPool::SerialNumber*>(pool);
//		mSerialNumber = *--serial_ptr;
//		mObject = serial_ptr;
//	}
//
//	bool operator==(const Handle& rhs) {
//		return mSerialNumber == rhs.mSerialNumber && rhs.mObject == rhs.mObject;
//	}
//
//	const bool isInitialized() const { return mSerialNumber > 0; }
//	const bool isValid() const {
//		if (mObject) {
//			auto serial = *reinterpret_cast<IObjectPool::SerialNumber*>(mObject);
//			return mSerialNumber == serial;
//		}
//		else
//			return false;
//	}
//
//	template<typename T>
//	inline T* get() const { 
//		if (!mObject)return nullptr;
//		auto obj = static_cast<typename ObjectPool<T>::Object*>(mObject);
//		mSerialNumber == obj->serial ? return obj->data : return nullptr;
//	}
//
//	bool destroy() { 
//		if (!isInitialized() || !isValid() || !mObject)return false;
//		if (auto pool = mPool.lock() ) {
//			pool->destroyObject(mObject);
//			reset();
//			return true;
//		}
//		else{
//			return false;
//		}
//	}
//
//	void reset() { mObject = nullptr; mSerialNumber = 0; mPool.reset(); }
//
//private:
//	void* mObject{nullptr};
//	IObjectPool::SerialNumber mSerialNumber{0};
//	std::weak_ptr<IObjectPool> mPool;
//};
//
//template<size_t test>
//struct IsPowerOf2 {
//	constexpr static const bool value = ((test != 0) && !(test & (test - 1)));
//};
//
//template<typename T, size_t blocksize = 1024, typename = std::enable_if_t< IsPowerOf2<blocksize>::value > >
//class ObjectPool : public IObjectPool, public std::enable_shared_from_this<ObjectPool<T, blocksize>> {
//
//	struct Object {
//
//		static Object* getObjectFromData(T* data) {
//			auto pool = reinterpret_cast<IObjectPool*>(data);
//			auto serial_ptr = reinterpret_cast<IObjectPool::SerialNumber*>(--pool);
//			return reinterpret_cast<Object*>(--serial_ptr);
//		}
//
//		void reset() { serial = 0; pool = nullptr; }
//
//		SerialNumber serial{ 0 };
//		IObjectPool* pool{nullptr}; //this is annoyingly redundant
//		T data;
//		Object* next;
//		Object* prev;
//	};
//
//public:
//
//	constexpr static const size_t BLOCK_SIZE = blocksize;
//	constexpr static const size_t OBJECTS_PER_BLOCK = BLOCK_SIZE / sizeof(Object);
//	constexpr static const size_t OBJECT_STRIDE = sizeof(Object);
//
//private:
//
//	struct MemoryBlock {
//
//		MemoryBlock() : next(nullptr), block(::operator new(BLOCK_SIZE)) { memset(block, 0, BLOCK_SIZE); }
//		MemoryBlock(MemoryBlock* _next) : next(_next), block(::operator new(BLOCK_SIZE)) { memset(block, 0, BLOCK_SIZE); }
//		~MemoryBlock() { 
//			::operator delete(block); 
//			if(next)delete next;
//		}
//
//		static T* nextAvailable( MemoryBlock*& tail ) {
//			if (tail->count >= OBJECTS_PER_BLOCK) {
//				try {
//					tail->next = new MemoryBlock();
//				}catch( std::bad_alloc ){
//					return nullptr;
//				}
//				tail = tail->next;
//				return MemoryBlock::nextAvailable(tail);
//			}
//			else {
//				auto ptr = static_cast<char*>(tail->block) + OBJECT_STRIDE*tail->count++;
//				auto obj = reinterpret_cast<Object*>(ptr);
//				return &obj->data;
//			}
//		}
//
//		MemoryBlock* next{nullptr};
//		size_t count{0};
//		void* block{nullptr};
//
//	};
//
//public:
//
//	class Iterator {
//	public:
//
//		void destroy() { mPool->destroyObject(mObject); }
//
//		Iterator& operator++() {
//			mObject = mObject->next;
//			return *this;
//		}
//
//		Iterator& operator--() {
//			mObject = mObject->prev;
//			return *this;
//		}
//
//
//		Iterator& operator++(int) {
//			mObject = mObject->next;
//			return *this;
//		}
//
//		Iterator& operator--(int) {
//			mObject = mObject->prev;
//			return *this;
//		}
//
//		T& operator->() {
//			return mObject->data;
//		}
//
//		T& operator*() {
//			return mObject->data;
//		}
//
//		Iterator& operator+(int advance) {
//			int iterations = 0;
//			while (iterations < advance) {
//				if (mObject == nullptr)throw std::out_of_range("iterator incremented past the end of the object linked list");
//				if (iterations++ >= advance || !mObject->next)break;
//				else ++(*this);
//			}
//			return *this;
//		}
//
//		Iterator& operator-(int reverse) {
//			while (iterations < reverse) {
//				if (mObject == nullptr)throw std::out_of_range("iterator decremented past the beginning of the object linked list");
//				if (iterations++ >= reverse || !mObject->prev)break;
//				else --(*this);
//			}
//			return *this;
//		}
//
//		Iterator& operator+=(int advance) {
//			int iterations = 0;
//			while (iterations < advance) {
//				if (mObject == nullptr)throw std::out_of_range("iterator incremented past the end of the object linked list");
//				if (iterations++ >= advance || !mObject->next)break;
//				else ++(*this);
//			}
//			return *this;
//		}
//
//		Iterator& operator-=(int reverse) {
//			while (iterations < reverse) {
//				if (mObject == nullptr)throw std::out_of_range("iterator decremented past the beginning of the object linked list");
//				if (iterations++ >= reverse || !mObject->prev)break;
//				else --(*this);
//			}
//			return *this;
//		}
//
//		~Iterator() = default;
//
//	private:
//
//		explicit Iterator( IObjectPool* pool, Object* object) : mPool(pool), mObject(object) {}
//
//		Object* mObject;
//		IObjectPool* mPool;
//
//		friend ObjectPool<T,blocksize>;
//	};
//
//	static std::shared_ptr<ObjectPool<T, blocksize>> create() { return std::shared_ptr<ObjectPool<T, blocksize>>( new ObjectPool<T, blocksize> ); }
//
//	template<typename...Args>
//	Handle createObject(Args...args){ 
//		if ( mFreeSpace ) {
//			auto next = mFreeSpace;
//			mFreeSpace = *reinterpret_cast<T**>(mFreeSpace);
//
//			try {
//				new(next) T(args...);
//			}
//			catch (std::bad_alloc) {
//				return Handle();
//			}
//			auto obj = Object::getObjectFromData(next);
//			obj->prev = mLastCreated;
//			if (mLastCreated)
//				mLastCreated->next = obj;
//			mLastCreated = obj;
//			obj->next = nullptr;
//			obj->pool = this;
//			auto handle =  Handle(obj, obj->serial, obj->pool->getWeakPtr());
//			++mSize;
//			if (mOnCreateHandlerfn)
//				mOnCreateHandlerfn(*next);
//			return std::move(handle);
//		}
//		else {
//			T* new_data = MemoryBlock::nextAvailable(mTail);
//			if (!new_data)
//				return Handle();
//			try {
//				new(new_data) T(args...);
//			}catch(std::bad_alloc){
//				return Handle();
//			}
//			auto obj = Object::getObjectFromData(new_data);
//			obj->prev = mLastCreated;
//			if (mLastCreated)
//				mLastCreated->next = obj;
//			mLastCreated = obj;
//			obj->next = nullptr;
//			obj->serial++;
//			obj->pool = this;
//			++mSize;
//			if (mOnCreateHandlerfn)
//				mOnCreateHandlerfn(*new_data);
//			return Handle(obj, obj->serial, obj->pool->getWeakPtr());
//		}
//	}
//
//	size_t size() const { return mSize; }
//	void connectObjectCreationHandler(const std::function<void(const T&)>& fn) { mOnCreateHandlerfn = fn; }
//	void connectObjectDestructionHandler(const std::function<void(const T&)>& fn) { mOnDestoryHandlerfn = fn; }
//
//	Iterator begin() { return Iterator(this, static_cast<Object*>(mHead->block)); }
//	Iterator end()   { return Iterator(this, mLastCreated); }
//
//	~ObjectPool() { delete mHead; }
//
//private:
//
//	ObjectPool() : mHead( new MemoryBlock ){
//		mTail = mHead;
//	}
//
//	std::weak_ptr<IObjectPool> getWeakPtr()override { return shared_from_this(); }
//
//	void destroyObject(void* object) override {
//		auto obj = static_cast<Object*>(object);
//		if (obj == mLastCreated) {
//			mLastCreated = obj->prev;
//		}
//		if (mOnDestoryHandlerfn)
//			mOnDestoryHandlerfn(obj->data);
//		obj->data.~T();
//		obj->serial++;
//		if (obj->prev) {
//			obj->prev->next = obj->next;
//			if(obj->next)
//				obj->next->prev = obj->prev;
//		}
//		else if(obj->next) {
//			obj->next->prev = nullptr;
//		}
//		--mSize;
//		*reinterpret_cast<T**>(&obj->data) = mFreeSpace;
//		mFreeSpace = &obj->data;
//
//		_ASSERT(*reinterpret_cast<T**>(&obj->data) != mFreeSpace);
//
//	}
//
//	MemoryBlock* mHead{nullptr};
//	MemoryBlock* mTail{nullptr};
//	Object* mLastCreated{nullptr};
//	size_t mSize{0};
//	T* mFreeSpace{nullptr};
//	std::function<void(const T&)> mOnCreateHandlerfn{nullptr};
//	std::function<void(const T&)> mOnDestoryHandlerfn{nullptr};
//
//	friend Handle;
//
//};