// HFSM_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include "ObjectPool.hpp"

using namespace std;

class outbuf : public std::streambuf {
public:
	outbuf() {
		setp(0, 0);
	}

	virtual int_type overflow(int_type c = traits_type::eof()) {
		return fputc(c, stdout) == EOF ? traits_type::eof() : c;
	}
};

class Test {
public:

	Test(int val) : mVal(val), mName("test") {}

	int getVal() const { return mVal; }

	~Test() { mName.clear(); }

private:

	int mVal;
	std::string mName;

};


int randInt(int range_min, int range_max)
{
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}

int main()
{
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		SetConsoleTitle(L"Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}
	outbuf ob;
	std::streambuf *sb = std::cout.rdbuf(&ob);

	{
		cout << "------------------------------------" << endl;
		cout << "Test - Create one object in one block, destroy one object and one block" << endl;

		auto pool = ObjectPool<Test>::create();

		pool->connectObjectCreationHandler([](const Test& test) {
			cout << "created test: " << test.getVal() << endl;
		});

		pool->connectObjectDestructionHandler([](const Test& test) {
			cout << "destoryed test: " << test.getVal() << endl;
		});

		auto handle = pool->createObject(1);

		_ASSERT(handle.isInitialized() && handle.isValid());
		_ASSERT(handle.destroy());
		cout << "success!" << endl;

	}

	{
		cout << "------------------------------------" << endl;
		cout << "Test - Handle initialization, copying, validity and reset" << endl;

		auto pool = ObjectPool<Test>::create();

		pool->connectObjectCreationHandler([](const Test& test) {
			cout << "created test: " << test.getVal() << endl;
		});

		pool->connectObjectDestructionHandler([](const Test& test) {
			cout << "destoryed test: " << test.getVal() << endl;
		});

		Handle handle;

		_ASSERT(!handle.isInitialized());
		_ASSERT(!handle.isValid());

		handle = pool->createObject(1);
		_ASSERT(handle.isInitialized() && handle.isValid());

		auto handle_cpy = handle;
		_ASSERT(handle_cpy.isInitialized() && handle_cpy.isValid());

		_ASSERT(handle.destroy());
		_ASSERT(!handle.destroy());

		_ASSERT(!handle.isInitialized());
		_ASSERT(!handle.isValid());
		handle.reset();
		_ASSERT(!handle.isInitialized());
		_ASSERT(!handle.isValid());

		_ASSERT(!handle_cpy.isValid() && handle_cpy.isInitialized());
		_ASSERT(!handle_cpy.destroy());
		cout << "success!" << endl;

	}

	{
		cout << "------------------------------------" << endl;

		cout << "Test - Fill block, empty block" << endl;
		auto pool = ObjectPool<Test>::create();

		pool->connectObjectCreationHandler([](const Test& test) {
			cout << "created test: " << test.getVal() << endl;
		});

		pool->connectObjectDestructionHandler([](const Test& test) {
			cout << "destoryed test: " << test.getVal() << endl;
		});

		cout << "Each object is " << ObjectPool<Test>::OBJECT_STRIDE << " bytes" << endl;

		std::vector<Handle> handles;
		size_t bytes = 0;
		for (int i = 0; i < ObjectPool<Test>::OBJECTS_PER_BLOCK; i++) {
			auto handle = pool->createObject(i);
			_ASSERT(handle.isInitialized() && handle.isValid());
			handles.push_back(handle);
			bytes += ObjectPool<Test>::OBJECT_STRIDE;
		}

		cout << "Filled " << bytes << " of " << ObjectPool<Test>::BLOCK_SIZE <<  endl;
		_ASSERT(bytes <= ObjectPool<Test>::BLOCK_SIZE);

		for (auto & handle : handles) {
			_ASSERT(handle.destroy());
		}
		handles.clear();
		cout << "success!" << endl;
	}


	{
		cout << "------------------------------------" << endl;

		cout << "Test - Fill block and create new block, empty all" << endl;
		auto pool = ObjectPool<Test>::create();

		pool->connectObjectCreationHandler([](const Test& test) {
			cout << "created test: " << test.getVal() << endl;
		});

		pool->connectObjectDestructionHandler([](const Test& test) {
			cout << "destoryed test: " << test.getVal() << endl;
		});

		std::vector<Handle> handles;
		size_t bytes = 0;
		for (int i = 0; i < ObjectPool<Test>::OBJECTS_PER_BLOCK + 1; i++) {
			auto handle = pool->createObject(i);
			_ASSERT(handle.isInitialized() && handle.isValid());
			handles.push_back(handle);
			bytes += ObjectPool<Test>::OBJECT_STRIDE;
		}

		cout << "Filled " << bytes << " of " << ObjectPool<Test>::BLOCK_SIZE*2 << endl;

		for (auto & handle : handles) {
			_ASSERT(handle.destroy());
		}

		handles.clear();
		cout << "success!" << endl;

	}

	{
		cout << "------------------------------------" << endl;

		cout << "Test - Fill 2 block and create a third block, empty all" << endl;
		auto pool = ObjectPool<Test>::create();

		pool->connectObjectCreationHandler([](const Test& test) {
			cout << "created test: " << test.getVal() << endl;
		});

		pool->connectObjectDestructionHandler([](const Test& test) {
			cout << "destoryed test: " << test.getVal() << endl;
		});

		std::vector<Handle> handles;
		size_t bytes = 0;
		for (int i = 0; i < ObjectPool<Test>::OBJECTS_PER_BLOCK*2 + 1; i++) {
			auto handle = pool->createObject(i);
			_ASSERT(handle.isInitialized() && handle.isValid());
			handles.push_back(handle);
			bytes += ObjectPool<Test>::OBJECT_STRIDE;
		}

		cout << "Filled " << bytes << " of " << ObjectPool<Test>::BLOCK_SIZE*3 << endl;

		for (auto & handle : handles) {
			_ASSERT(handle.destroy());
		}

		handles.clear();
		cout << "success!" << endl;

	}

	{
		cout << "------------------------------------" << endl;

		cout << "Test - test pooling" << endl;
		auto pool = ObjectPool<Test>::create();

		pool->connectObjectCreationHandler([](const Test& test) {
			//cout << "created test: " << test.getVal() << endl;
		});

		//pool->connectObjectDestructionHandler([](const Test& test) {
			//cout << "destoryed test: " << test.getVal() << endl;
		//});

		std::vector<Handle> handles;
		size_t bytes = 0;
		size_t test_amt = 50000;
		for (int i = 0; i < test_amt; i++) {
			auto handle = pool->createObject(i);
			_ASSERT(handle.isInitialized() && handle.isValid());
			handles.push_back(handle);
			bytes += ObjectPool<Test>::OBJECT_STRIDE;
		}

		cout << "Filled " << bytes << " of " << test_amt*ObjectPool<Test>::OBJECT_STRIDE << endl;

		_ASSERT(pool->size() == test_amt);
		auto start = std::chrono::system_clock::now();

		int destroy_amt = test_amt*.5f;
		int skipped = 0;
		cout << "destroying " << destroy_amt << " objects" << endl;
		for (int i = 0; i < destroy_amt; i++) {
			auto rand = randInt(0, handles.size());
			auto & handle = handles[rand];
			if (handle.isValid()) {
				_ASSERT(handle.destroy());
			}else{
				cout << "already destroyed #" << rand << endl;
				skipped++;
			}
		}
		auto finish = std::chrono::system_clock::now();
		cout << "projected pool size = " << test_amt - destroy_amt + skipped << endl;
		cout << "pool size = " << pool->size() << endl;
		cout << "time for destroying " << destroy_amt << " objects: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << endl;
		_ASSERT(pool->size() == test_amt - destroy_amt + skipped);

		for (int i = 0; i < destroy_amt - skipped+1; i++) {
			auto handle = pool->createObject(i);
			_ASSERT(handle.isInitialized() && handle.isValid());
			handles.push_back(handle);
		}
		_ASSERT(pool->size() == test_amt + 1);

		for (auto & handle : handles) {
			handle.destroy();
		}

		handles.clear();
		cout << "success!"<< endl;
	}
	/*
	{
		cout << "------------------------------------" << endl;

		cout << "Test - stress" << endl;


		auto pool = ObjectPool<Test, 65536>::create();
		auto handles = ObjectPool<Handle, 65536>::create();

		for (int i = 0; i < 10000; i++) {
			auto handle = pool->createObject(i);
			_ASSERT(handle.isInitialized() && handle.isValid());
			handles->createObject( std::move(handle) );
		}

		bool run = true;
		int iterations = 5;
		int j = 0;
		while (run) {

			auto start = std::chrono::system_clock::now();
			auto rand = randInt(1,10000);
			for (int i = 0; i < rand; i++) {
				auto rand = randInt(0, handles->size());
				auto it = handles->begin();
				it += rand;
				auto & handle = *it;
				handle.destroy();
				it.destroy();
			}

			auto finish = std::chrono::system_clock::now();
			cout << "time for destroying " << rand << " objects: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << endl;

			start = std::chrono::system_clock::now();

			for (int i = 0; i < rand; i++) {
				auto handle = pool->createObject(i);
				_ASSERT(handle.isInitialized() && handle.isValid());
				handles->createObject(std::move(handle));
			}

			finish = std::chrono::system_clock::now();
			cout << "time for creating " << rand << " objects: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << endl;

			if (j++ > iterations) run = false;
		}

	}
	*/

	std::cout.rdbuf(sb);
	return 0;
}
