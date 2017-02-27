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
#include "MemoryPool.hpp"

template<typename T>
class small_object_pool_policy
{
public:

	constexpr static const size_t MAX_SMALL_OBJECT_SIZE = 256;

	ALLOCATOR_TRAITS(T)

	template<typename U>
	struct rebind
	{
		typedef small_object_pool_policy<U> other;
	};

	// Default Constructor
	small_object_pool_policy() {
		mem::MemoryPool<sizeof(T)>::get();
	}

	// Copy Constructor
	template<typename U>
	small_object_pool_policy(small_object_pool_policy<U> const& other) {}

	// Allocate memory
	pointer allocate(size_type count, const_pointer hint = 0)
	{

		if (sizeof(T) <= MAX_SMALL_OBJECT_SIZE && count == 1) {
			//object is correct size and only one is requested
			return static_cast<pointer>(mem::MemoryPool<sizeof(T)>::get()->alloc());
		}
		else {
			if (count > max_size()) { throw std::bad_alloc(); }
			return static_cast<pointer>(::operator new(count * sizeof(type), ::std::nothrow));
		}

		return nullptr;

	}

	// Delete memory
	void deallocate(pointer ptr, size_type count)
	{
		if (sizeof(T) <= MAX_SMALL_OBJECT_SIZE && count == 1) {
			mem::MemoryPool<sizeof(T)>::get()->free(ptr);
		}
		else {
			::operator delete(ptr);
		}
	}

	// Max number of objects that can be allocated in one call
	size_type max_size(void) const { return max_allocations<T>::value; }
};

// Specialize for the small pool policy
template<typename T, typename TraitsT,
	typename U, typename TraitsU>
	bool operator==(Allocator<T, small_object_pool_policy<T>, TraitsT> const& left,
		Allocator<U, small_object_pool_policy<U>, TraitsU> const& right)
{
	return false;
}

// Also implement inequality
template<typename T, typename TraitsT,
	typename U, typename TraitsU>
	bool operator!=(Allocator<T, small_object_pool_policy<T>, TraitsT> const& left,
		Allocator<U, small_object_pool_policy<U>, TraitsU> const& right)
{
	return !(left == right);
}