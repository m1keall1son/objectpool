#pragma once

#include <vector>
#include "Allocator.hpp"
#include "HeapPolicy.hpp"
#include "ObjectTraits.hpp"

using HandleSizeType = uint64_t;

#define POOL_INDEX_BITS 16
#define SLOT_SERIAL_BITS 16
#define SLOT_ALIVE_BITS 1
#define SLOT_INDEX_BITS 31

struct SparseSlotIndex {
	uint64_t : 17; //unused
	uint64_t slot_serial : SLOT_SERIAL_BITS;
	uint64_t dense_slot_index : SLOT_INDEX_BITS;
};

struct DenseSlotIndex {
	uint32_t alive : SLOT_ALIVE_BITS;
	uint32_t sparse_slot_index : SLOT_INDEX_BITS;
};

struct Handle {

	HandleSizeType pool_id : POOL_INDEX_BITS;
	HandleSizeType slot_serial : SLOT_SERIAL_BITS;
	HandleSizeType slot_index : SLOT_INDEX_BITS;

	//explicit conversion
	inline operator HandleSizeType() const {
		return pool_id << POOL_INDEX_BITS | slot_serial << SLOT_SERIAL_BITS | slot_index;
	}

	bool isSet() { return ((pool_id << POOL_INDEX_BITS | slot_serial << SLOT_SERIAL_BITS | slot_index) != std::numeric_limits<uint64_t>::max()); }
	void reset() { pool_id = -1; slot_serial = -1; slot_index = -1; }
};

class IMemoryPolicy {
public:
	virtual bool free(Handle handle) = 0;
	virtual size_t size() const = 0;
	virtual void reserve(size_t count) = 0;
	virtual ~IMemoryPolicy() = default;
};

template<typename T>
class UnorderdSparseSet : public IMemoryPolicy {

public:

	using iterator = std::vector<T>::iterator;
	using const_iterator = std::vector<T>::const_iterator;

	template<typename...Args>
	Handle alloc(Args&&...args) {

	}

	bool free(Handle handle) override {

	}

	bool isValid(Handle handle) {

	}

	T* get(Handle handle) {
		
	}

	size_t size() const override { return mBack; }

	iterator begin() { return mData.begin(); }
	iterator end() { auto end = mData.begin(); std::advance(end, mBack); return end; }

	const_iterator cbegin() { return mData.cbegin(); }
	const_iterator cend() { auto cend = mData.cbegin(); std::advance(cend, mBack); return cend; }

	void reserve(size_t count)override {
		mData.resize(count);
		mSparse.resize(count);
		mDense.resize(count);
	}

private:

	size_t mBack{0};
	std::vector<SparseSlotIndex> mSparse;
	std::vector<DenseSlotIndex> mDense;
	SparseSlotIndex* mFreeSparseList;
	std::vector<T, Allocator<T, heap_policy<T>, disabled_construction_destruction_object_traits<T>>> mData;

};