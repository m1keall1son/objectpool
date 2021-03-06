//
//  main.cpp
//  ObjectPool
//
//  Created by MC on 2/10/17.
//  Copyright © 2017 MC. All rights reserved.
//

#include <iostream>
#include <assert.h>
#include <vector>
#include "ObjectPool.hpp"
#include <random>
#include <list>
#include <algorithm>
#include <chrono>

using namespace std;

class Test {
public:
    
    Test(int val) : mVal(val), mName("test") {}
    
    int getVal() const { return mVal; }
    
    ~Test() { mName.clear(); }
    
private:
    
    int mVal;
    std::string mName;
    
};

int randomInt( int max ){
    auto r = rand() / (float)RAND_MAX;
    return r * max;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    
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
        
        assert(handle.isInitialized() && handle.isValid());
        assert(handle.destroy());
        cout << "success!" << endl;
        
    }
    
    {
        cout << "------------------------------------" << endl;
        cout << "Test - creating handle from handled object" << endl;
        
        auto pool = ObjectPool<Test>::create();
        
        pool->connectObjectCreationHandler([](const Test& test) {
            cout << "created test: " << test.getVal() << endl;
        });
        
        pool->connectObjectDestructionHandler([](const Test& test) {
            cout << "destoryed test: " << test.getVal() << endl;
        });
        
        for(int i = 0; i < 20; i++){
            auto handle_pool = pool->createObject(i);
        
            auto test = handle_pool.get<Test>();
            
            Handle handle_obj(test);
        
            assert( handle_pool == handle_obj );
        }

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
        
        assert(!handle.isInitialized());
        assert(!handle.isValid());
        
        handle = pool->createObject(1);
        assert(handle.isInitialized() && handle.isValid());
        
        auto handle_cpy = handle;
        assert(handle_cpy.isInitialized() && handle_cpy.isValid());
        
        assert(handle.destroy());
        assert(!handle.destroy());
        
        assert(!handle.isInitialized());
        assert(!handle.isValid());
        handle.reset();
        assert(!handle.isInitialized());
        assert(!handle.isValid());
        
        assert(!handle_cpy.isValid() && handle_cpy.isInitialized());
        assert(!handle_cpy.destroy());
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
            assert(handle.isInitialized() && handle.isValid());
            handles.push_back(handle);
            bytes += ObjectPool<Test>::OBJECT_STRIDE;
        }
        
        cout << "Filled " << bytes << " of " << ObjectPool<Test>::BLOCK_SIZE <<  endl;
        assert(bytes <= ObjectPool<Test>::BLOCK_SIZE);
        
        for (auto & handle : handles) {
            assert(handle.destroy());
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
            assert(handle.isInitialized() && handle.isValid());
            handles.push_back(handle);
            bytes += ObjectPool<Test>::OBJECT_STRIDE;
        }
        
        cout << "Filled " << bytes << " of " << ObjectPool<Test>::BLOCK_SIZE*2 << endl;
        
        for (auto & handle : handles) {
            assert(handle.destroy());
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
            assert(handle.isInitialized() && handle.isValid());
            handles.push_back(handle);
            bytes += ObjectPool<Test>::OBJECT_STRIDE;
        }
        
        cout << "Filled " << bytes << " of " << ObjectPool<Test>::BLOCK_SIZE*3 << endl;
        
        for (auto & handle : handles) {
            assert(handle.destroy());
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
        
        pool->connectObjectDestructionHandler([](const Test& test) {
           //cout << "destoryed test: " << test.getVal() << endl;
        });
        
        std::vector<Handle> handles;
        size_t bytes = 0;
        size_t test_amt = 1000;
        for (int i = 0; i < test_amt; i++) {
			Handle handle;
			try {
				handle = pool->createObject(i);
				assert(handle.isInitialized() && handle.isValid());
			}
			catch (std::bad_alloc) {
				cout << "reached maximum capactity: " << pool->size() << endl;
				test_amt = pool->size();
				break;
			}
			handles.push_back(handle);
			bytes += ObjectPool<Test>::OBJECT_STRIDE;
		}
        
        cout << "Filled " << bytes << " of " << test_amt*ObjectPool<Test>::OBJECT_STRIDE << endl;
        
        assert(pool->size() == test_amt);
        auto start = std::chrono::system_clock::now();
        
        int destroy_amt = pool->size()*.5f;
        int skipped = 0;
        cout << "destroying " << destroy_amt << " objects" << endl;
        for (int i = 0; i < destroy_amt; i++) {
            auto rand = randomInt( handles.size()-1 );
            auto & handle = handles[rand];
            if (handle.isValid()) {
				auto res = handle.destroy();
				assert(res);
            }else{
                skipped++;
            }
        }
        
        auto finish = std::chrono::system_clock::now();
		cout << "skipped = " << skipped << endl;
        cout << "projected pool size = " << test_amt - destroy_amt + skipped << endl;
        cout << "pool size = " << pool->size() << endl;
        cout << "time for destroying " << destroy_amt << " objects: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << endl;
        assert(pool->size() == test_amt - destroy_amt + skipped);
        
		start = std::chrono::system_clock::now();

        for (int i = 0; i < destroy_amt - skipped+1; i++) {
            auto handle = pool->createObject(i);
            assert(handle.isInitialized() && handle.isValid());
            handles.push_back(handle);
        }
        assert(pool->size() == test_amt + 1);
        
		finish = std::chrono::system_clock::now();
		cout << "time for re-creating " << destroy_amt << " objects: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << endl;

		start = std::chrono::system_clock::now();

        for (auto & handle : handles) {
            handle.destroy();
        }

		finish = std::chrono::system_clock::now();
		cout << "time for destroying all objects " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << endl;

        handles.clear();
        cout << "success!"<< endl;
    }
    
    return 0;
}
