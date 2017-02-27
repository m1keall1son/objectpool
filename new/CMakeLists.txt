cmake_minimum_required (VERSION 2.6)
project (objectpool)
add_executable(objectpool 
	PoolAllocator/main.cpp 
	PoolAllocator/Allocator.hpp 
	PoolAllocator/AllocatorTraits.hpp 
	PoolAllocator/Allocator.hpp
	PoolAllocator/ObjectTraits.hpp
	PoolAllocator/HeapPolicy.hpp
	PoolAllocator/ListPoolPolicy.hpp
	PoolAllocator/SmallObjectPoolPolicy.hpp
	PoolAllocator/MemoryPool.h
	PoolAllocator/MemoryPool.cpp
) 