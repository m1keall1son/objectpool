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
#include <array>
#include "ListPoolPolicy.hpp"

using SmallObjectPoolManagerRef = std::shared_ptr<class SmallObjectPoolManager>;

class SmallObjectPoolManager {
	
public:
	
	SmallObjectPoolManagerRef get(){
		if(!sInstance)
			sInstance.reset(new SmallObjectPoolManager );
		return sInstance;
	}
	
	inline void* allocate( unsigned int object_size, unsigned int count ){
		
		if(object_size == 0)
			return nullptr;
		
		if(object_size <= 256 && count == 1 ){
			
			//object is correct size and only one is requested!
			//allocate from mPools[object_size-1]
			
		}else if( count > 1 && object_size <= 256 && (count * object_size) <= 256 ){
			
			//and array was requested, size of array will fit into a pool, so lets do it!
			//allocate from mPools[count*object_size]
			
		}else{
			
			//use regular memory manager
			
			
		}
		
	}
	
	inline void free( unsigned int object_size, void* ){
		
		if(object_size > 256){
			
			//make sure 
			
		}else{
			
			//we used the regular memory manager
			
		}
		
	}

	~SmallObjectPoolManager(){
		for(auto ptr : mPoolBlocks){
			delete ptr;
		}
	}
	
private:
	
	static SmallObjectPoolPolicyRef sInstance{nullptr};
	
	SmallObjectPoolManager(){ mPoolBlocks.reserve(1024); }
		
	std::array< std::list< void*, list_pool_policy<void*> >, 256 > mPools;
	std::vector<void*> mPoolBlocks;
	std::unordered_map<void*, unsigned int>
	
};


SmallObjectPoolPolicyRef SmallObjectPoolManager::sInstance;


