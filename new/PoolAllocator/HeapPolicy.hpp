//
//  HeapPolicy.h
//  PoolAllocator
//
//  Created by Mike Allison on 2/26/17.
//  Copyright (c) 2017 Mike Allison. All rights reserved.
//

#pragma once
#include "AllocatorTraits.hpp"

template<typename T>
class heap_policy
{
public:
	
	ALLOCATOR_TRAITS(T)
	
	template<typename U>
	struct rebind
	{
		typedef heap_policy<U> other;
	};
	
	// Default Constructor
	heap_policy(void){}
	
	// Copy Constructor
	template<typename U>
	heap_policy(heap_policy<U> const& other){}
	
	// Allocate memory
	pointer allocate(size_type count, const_pointer hint = 0)
	{
		if(count > max_size()){throw std::bad_alloc();}
		return static_cast<pointer>(::operator new(count * sizeof(type), ::std::nothrow));
	}
	
	// Delete memory
	void deallocate(pointer ptr, size_type count )
	{
		::operator delete(ptr);
	}
	
	// Max number of objects that can be allocated in one call
	size_type max_size(void) const {return max_allocations<T>::value;}
};
