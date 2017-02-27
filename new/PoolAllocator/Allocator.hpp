//
//  AllocatorHost.h
//  PoolAllocator
//
//  Created by Mike Allison on 2/26/17.
//  Copyright (c) 2017 Mike Allison. All rights reserved.
//

#pragma once

#include "ObjectTraits.hpp"
#include "AllocatorTraits.hpp"
#include "HeapPolicy.hpp"

#define FORWARD_ALLOCATOR_TRAITS(C)                  \
typedef typename C::value_type      value_type;      \
typedef typename C::pointer         pointer;         \
typedef typename C::const_pointer   const_pointer;   \
typedef typename C::reference       reference;       \
typedef typename C::const_reference const_reference; \
typedef typename C::size_type       size_type;       \
typedef typename C::difference_type difference_type; \

template<typename T,
	typename PolicyT = heap_policy<T>,
	typename TraitsT = basic_object_traits<T> >
class Allocator : public PolicyT, public TraitsT
{
public:
	
	// Template parameters
	typedef PolicyT Policy;
	typedef TraitsT Traits;
	
	FORWARD_ALLOCATOR_TRAITS(Policy)
	
	template<typename U>
	struct rebind
	{
		typedef Allocator<U,
		typename Policy::template rebind<U>::other,
		typename Traits::template rebind<U>::other
		> other;
	};
	
	template<typename...Args>
	Allocator(Args&&...args) : Policy(std::forward<Args>(args)...) {}
	
	// Copy Constructor
	template<typename U,
	typename PolicyU,
	typename TraitsU>
	Allocator(Allocator<U,
			  PolicyU,
			  TraitsU> const& other) :
	Policy(other),
	Traits(other)
	{}
};

// Two allocators are not equal unless a specialization says so
template<typename T, typename PolicyT, typename TraitsT,
typename U, typename PolicyU, typename TraitsU>
bool operator==(Allocator<T, PolicyT, TraitsT> const& left,
				Allocator<U, PolicyU, TraitsU> const& right)
{
	return false;
}

// Also implement inequality
template<typename T, typename PolicyT, typename TraitsT,
typename U, typename PolicyU, typename TraitsU>
bool operator!=(Allocator<T, PolicyT, TraitsT> const& left,
				Allocator<U, PolicyU, TraitsU> const& right)
{
	return !(left == right);
}

// Comparing an allocator to anything else should not show equality
template<typename T, typename PolicyT, typename TraitsT,
typename OtherAllocator>
bool operator==(Allocator<T, PolicyT, TraitsT> const& left,
				OtherAllocator const& right)
{
	return false;
}

// Also implement inequality
template<typename T, typename PolicyT, typename TraitsT,
typename OtherAllocator>
bool operator!=(Allocator<T, PolicyT, TraitsT> const& left,
				OtherAllocator const& right)
{
	return !(left == right);
}

// Specialize for the heap policy
template<typename T, typename TraitsT,
typename U, typename TraitsU>
bool operator==(Allocator<T, heap_policy<T>, TraitsT> const& left,
				Allocator<U, heap_policy<U>, TraitsU> const& right)
{
	return true;
}

// Also implement inequality
template<typename T, typename TraitsT,
typename U, typename TraitsU>
bool operator!=(Allocator<T, heap_policy<T>, TraitsT> const& left,
				Allocator<U, heap_policy<U>, TraitsU> const& right)
{
	return !(left == right);
}
