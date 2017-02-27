//
//  PoolAllocator.h
//  PoolAllocator
//
//  Created by Mike Allison on 2/26/17.
//  Copyright (c) 2017 Mike Allison. All rights reserved.
//

#pragma once
#include <exception>
#include "MemoryPool.hpp"
#include "AllocatorTraits.hpp"
#include "Allocator.hpp"
#include <memory>
#include <iostream>

template<typename T>
class list_pool_policy
{
public:
		
	ALLOCATOR_TRAITS(T)
	
	template<typename U>
	struct rebind
	{
		typedef list_pool_policy<U> other;
	};
	
	// Default Constructor
	list_pool_policy(){
		mem::MemoryPool<sizeof(T)>::get();
	}
	
	// Copy Constructor
	template<typename U>
	list_pool_policy(list_pool_policy<U> const& other){
		mem::MemoryPool<sizeof(T)>::get();
	}
	
	// Allocate memory
	pointer allocate(size_type count, const_pointer hint = 0)
	{

		if (count == 1)
			return reinterpret_cast<pointer>(mem::MemoryPool<sizeof(T)>::get()->alloc());
		else
		{
			throw std::runtime_error("pool can only do one at a time");
		}
	}
	
	// Delete memory
	void deallocate(pointer ptr, size_type count )
	{
		if (count == 1) 
			mem::MemoryPool<sizeof(T)>::get()->free(ptr);
		else
		{
			throw std::runtime_error("pool can only do one at a time");
		}
	}
	
	// Max number of objects that can be allocated in one call
	size_type max_size(void) const { return max_allocations<T>::value; }
	
};


// Specialize for the heap policy
template<typename T, typename TraitsT,
typename U, typename TraitsU>
bool operator==(Allocator<T, list_pool_policy<T>, TraitsT> const& left,
				Allocator<U, list_pool_policy<U>, TraitsU> const& right)
{
	return false;
}

// Also implement inequality
template<typename T, typename TraitsT,
typename U, typename TraitsU>
bool operator!=(Allocator<T, list_pool_policy<T>, TraitsT> const& left,
				Allocator<U, list_pool_policy<U>, TraitsU> const& right)
{
	return !(left == right);
}