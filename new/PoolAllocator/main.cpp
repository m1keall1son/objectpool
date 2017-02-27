//
//  main.cpp
//  PoolAllocator
//
//  Created by Mike Allison on 2/26/17.
//  Copyright (c) 2017 Mike Allison. All rights reserved.
//

#include <iostream>
#include <chrono>
#include <list>
#include <random>
#include <memory>

#include "ListPoolPolicy.hpp"
#include "Allocator.hpp"

const int MAX_SIZE = 5000;
const int MAX_ITERATIONS = 5000;

class Test {
public:
	
	Test(){
		//std::cout << "constructing test" << std::endl;
	}
	
	Test(int val) : mVal(val){
		//std::cout << "constructing test with val " << mVal << std::endl;
	}
	
	~Test(){
		//std::cout << "destructing test with val " << mVal << std::endl;
	}
	
	void setVal(int val){ mVal = val; }
	
private:
	int mVal;
	int mAnother;
	float mYetAnother;
	char stuff[36];
};

class Thing {
public:
	
	Thing(){
		//std::cout << "constructing test" << std::endl;
	}
	
	Thing(int val) : mVal(val){
		//std::cout << "constructing test with val " << mVal << std::endl;
	}
	
	~Thing(){
		//std::cout << "destructing test with val " << mVal << std::endl;
	}
	
	void setVal(int val){ mVal = val; }
	
private:
	int mVal;
	char stuff[24];
};

int main(int argc, const char * argv[]) {
	// insert code here...

	auto start = std::chrono::high_resolution_clock::now();
	auto finish = std::chrono::high_resolution_clock::now();
	
	{
		std::vector<Test, Allocator<Test, heap_policy<Test>>> regular_list;
		
		start = std::chrono::high_resolution_clock::now();
		
		for(int j = 0; j < MAX_ITERATIONS; j++ ){
		
			for(int i = 0; i < MAX_SIZE; i++){
				regular_list.emplace_back(i);
			}
			
			for(int i = 0; i < MAX_SIZE; i++){
				regular_list.pop_back();
			}
			
		}
		
		finish = std::chrono::high_resolution_clock::now();
		
		std::cout << "Time to alloc/free regular list: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count()  << "ms" << std::endl;
		
	}
	
	std::cout << "-----------------------------" << std::endl;
	
	{
		std::list<Test, Allocator<Test, list_pool_policy<Test>>> pooled_list;
		
		start = std::chrono::high_resolution_clock::now();
		
		for(int j = 0; j < MAX_ITERATIONS; j++ ){
			
			for(int i = 0; i < MAX_SIZE; i++){
				pooled_list.emplace_back(i);
			}
			
			for(int i = 0; i < MAX_SIZE; i++){
				pooled_list.pop_front();
			}
			
		}
		
		finish = std::chrono::high_resolution_clock::now();
		
		std::cout << "Time to alloc/free pooled list: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count()  << "ms" << std::endl;
		
	}
	
	std::cout << "-----------------------------" << std::endl;

    return 0;
}
