//
//  ObjectTraits.hpp
//  PoolAllocator
//
//  Created by Mike Allison on 2/26/17.
//  Copyright (c) 2017 Mike Allison. All rights reserved.
//

#pragma once

template<typename T>
class basic_object_traits
{
public:
	
	typedef T type;
	
	template<typename U>
	struct rebind
	{
		typedef basic_object_traits<U> other;
	};
	
	// Constructor
	basic_object_traits(void){}
	
	// Copy Constructor
	template<typename U>
	basic_object_traits(basic_object_traits<U> const& other){}
	
	// Address of object
	type*       address(type&       obj) const {return &obj;}
	type const* address(type const& obj) const {return &obj;}
	
	// Construct object
	void construct(type* ptr, type const& ref) const
	{
		// In-place copy construct
		new(ptr) type(ref);
	}
	
	// Destroy object
	void destroy(type* ptr) const
	{
		// Call destructor
		ptr->~type();
	}
};

template<typename T>
class disabled_construction_destruction_object_traits
{
public:

	typedef T type;

	template<typename U>
	struct rebind
	{
		typedef basic_object_traits<U> other;
	};

	// Constructor
	basic_object_traits(void) {}

	// Copy Constructor
	template<typename U>
	basic_object_traits(basic_object_traits<U> const& other) {}

	// Address of object
	type*       address(type&       obj) const { return &obj; }
	type const* address(type const& obj) const { return &obj; }

	// Construct object
	void construct(type* ptr, type const& ref) const
	{
		/* do nothing */
	}

	// Destroy object
	void destroy(type* ptr) const
	{
		/* do nothing */
	}
};