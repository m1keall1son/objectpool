//
//  SmallObjectPoolPolicy.hpp
//  PoolAllocator
//
//  Created by Mike Allison on 2/27/17.
//  Copyright (c) 2017 Mike Allison. All rights reserved.
//

#pragma once

#include <vector>
#include <list>
#include <memory>
#include <string>
#include <array>
#include "AllocatorTraits.hpp"
#include "ListPoolPolicy.hpp"

using SmallObjectPoolManagerRef = std::shared_ptr < class SmallObjectPoolManager > ;
class SmallObjectPoolManager {
	
public:

	constexpr static const size_t MAX_SMALL_OBJECT_SIZE = 256;
	constexpr static const size_t POOL_SIZE = 1024;

	static SmallObjectPoolManagerRef get(){
		if(!sInstance)
			sInstance.reset(new SmallObjectPoolManager );
		return sInstance;
	}
	
	inline void* allocate( unsigned int object_size, unsigned int count ){
		
		void* ret = nullptr;

		if(object_size == 0)
			return nullptr;
		
		if(object_size <= MAX_SMALL_OBJECT_SIZE && count == 1 ){
			
			//object is correct size and only one is requested
			return getNextAvailable(object_size);

		}else{
			//it was an array or object size was too big
			//use heap
			auto ptr = ::operator new(count * object_size, ::std::nothrow);
			if (ptr) {
				return ptr;
			}else{
				throw std::bad_alloc();
			}
		}
		
		return ret;
	}
	
	inline void free( unsigned int object_size, void* ptr, size_t count ){
		
		if (object_size == 0)
			return;

		if( object_size <= MAX_SMALL_OBJECT_SIZE && count == 1 ){
			auto& pool = mPools[object_size - 1];
			pool.Free(ptr);
		}else{
			::operator delete(ptr);
		}
	}

	void initPool(unsigned int object_size) {
		auto& pool = mPools[object_size - 1];
		if (!pool.isInitialized()) {
			pool.Init(object_size, POOL_SIZE);
		}
	}

	~SmallObjectPoolManager() = default;
	
private:
	
	static SmallObjectPoolManagerRef sInstance;

	void* getNextAvailable(unsigned int object_size){
		auto& pool = mPools[object_size-1];
		if (!pool.isInitialized()) {
			pool.Init(object_size, POOL_SIZE);
		}
		return pool.Alloc();
	}

	SmallObjectPoolManager() = default;
		
	std::array< MemoryPool, MAX_SMALL_OBJECT_SIZE > mPools;
	
};

SmallObjectPoolManagerRef SmallObjectPoolManager::sInstance{nullptr};


template<typename T>
class small_object_pool_policy
{
public:

	ALLOCATOR_TRAITS(T)

	template<typename U>
	struct rebind
	{
		typedef small_object_pool_policy<U> other;
	};

	// Default Constructor
	small_object_pool_policy() {
		SmallObjectPoolManager::get()->initPool(sizeof(T));
	}

	// Copy Constructor
	template<typename U>
	small_object_pool_policy(small_object_pool_policy<U> const& other) {}

	// Allocate memory
	pointer allocate(size_type count, const_pointer hint = 0)
	{
		if (count > max_size()) { throw std::bad_alloc(); }
		return reinterpret_cast<pointer>( SmallObjectPoolManager::get()->allocate( sizeof(T), count ) );
	}

	// Delete memory
	void deallocate(pointer ptr, size_type count)
	{
		SmallObjectPoolManager::get()->free(sizeof(T), ptr, count);
	}

	// Max number of objects that can be allocated in one call
	size_type max_size(void) const { return max_allocations<T>::value; }
};