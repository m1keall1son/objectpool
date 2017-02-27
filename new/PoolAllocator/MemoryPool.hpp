#pragma once

#include <iostream>
#include <memory>
#include <mutex>
//#define _DEBUG

namespace mem {

	template<size_t object_size>
	class MemoryPool
	{

	public:

		constexpr static const size_t OBJECT_SIZE = object_size;
		constexpr static const size_t CHUNK_HEADER_SIZE = (sizeof(unsigned char*));

		static MemoryPool<OBJECT_SIZE>* get() {
			if (!sInstance)
				sInstance.reset(new MemoryPool<OBJECT_SIZE>);
			return sInstance.get();
		}

		// construction
		~MemoryPool() {
			destroy();
		}

		bool init(unsigned int num_objects = 1024) {

			//reinit if necessary
			if (mRawMemoryArray)
				destroy();

			// fill out our size & number members
			mNumObjects = num_objects;

			// attempt to grow the memory array
			if (growMemoryArray()) {
				mIsInitialized = true;
				return true;
			}

			return false;
		}

		void destroy() {
			// dump the state of the memory pool
#ifdef _DEBUG
			if (mIsInitialized) {
				std::string str;
				if (mNumAllocs != 0)
					str = "***(" + std::to_string(mNumAllocs) + ") ";
				unsigned long totalNumChunks = mNumObjects * mMemArraySize;
				unsigned long wastedMem = (totalNumChunks - mAllocPeak) * mNumObjects;
				str += "Destroying memory pool: [ MemoryPool :" + std::to_string((unsigned long)OBJECT_SIZE) + "] = " + std::to_string(mAllocPeak) + "/" + std::to_string((unsigned long)totalNumChunks) + " (" + std::to_string(wastedMem) + " bytes wasted)\n";
				std::cout << str << std::endl;
			}
#endif
			// free all memory
			for (unsigned int i = 0; i < mMemArraySize; ++i){
				free(mRawMemoryArray[i]);
			}
			free(mRawMemoryArray);

			// update member variables
			reset();
		}

		void* alloc(void) {
			// If we're out of memory chunks, grow the pool.  This is very expensive.

			if (!mHead)
			{
				// if we don't allow resizes, return NULL
				if (!mAllowResize)
					return nullptr;

				// attempt to grow the pool
				if (!growMemoryArray())
					return nullptr;  // couldn't allocate anymore memory
			}

#ifdef _DEBUG
			// update allocation reports
			++mNumAllocs;
			if (mNumAllocs > mNumAllocs)
				mAllocPeak = mNumAllocs;
#endif

			// grab the first chunk from the list and move to the next chunks
			unsigned char* ret = mHead;
			mHead = getNext(mHead);

			return (ret + CHUNK_HEADER_SIZE);  // make sure we return a pointer to the data section only
		}

		void  free(void* ptr) {
			if (ptr != nullptr)  	// calling Free() on a NULL pointer is perfectly valid
			{
				// The pointer we get back is just to the data section of the chunk.  This gets us the full chunk.
				unsigned char* pBlock = ((unsigned char*)ptr) - CHUNK_HEADER_SIZE;

				
				// push the chunk to the front of the list
				setNext(pBlock, mHead);
				mHead = pBlock;
#ifdef _DEBUG
				// update allocation reports
				--mNumAllocs;
				//GCC_ASSERT(m_numAllocs >= 0);
#endif
				

			}
		}

		unsigned int getObjectSize() const { return OBJECT_SIZE; }
		bool isInitialized() const { return mIsInitialized; }
		void setAllowResize(bool allow) { mAllowResize = allow; }

	private:

		static std::shared_ptr<MemoryPool<OBJECT_SIZE>> sInstance;

		MemoryPool() {
			reset();
			init(1024);
		}

		unsigned char** mRawMemoryArray;  // an array of memory blocks, each split up into chunks and connected
		unsigned char* mHead;  // the front of the memory chunk linked list
		unsigned int mNumObjects;  // the size of each chunk and number of chunks per array, respectively
		unsigned int mMemArraySize;  // the number elements in the memory array
		bool mAllowResize;  // true if we resize the memory pool when it fills up
		bool mIsInitialized;

#ifdef _DEBUG
		unsigned long mAllocPeak, mNumAllocs;
#endif

		// resets internal vars
		void reset() {
			mRawMemoryArray = nullptr;
			mHead = nullptr;
			mNumObjects = 0;
			mMemArraySize = 0;
			mAllowResize = true;
			mIsInitialized = false;
#ifdef _DEBUG
			mAllocPeak = 0;
			mNumAllocs = 0;
#endif
		}

		// internal memory allocation helpers
		bool growMemoryArray() {
#ifdef _DEBUG
			std::string str("Growing memory pool: [" + std::to_string((unsigned long)OBJECT_SIZE) + "] = " + std::to_string((unsigned long)mMemArraySize + 1) + "\n");
			std::cout << str << std::endl;
#endif

			// allocate a new array
			size_t allocationSize = sizeof(unsigned char*) * (mMemArraySize + 1);
			unsigned char** ppNewMemArray = (unsigned char**)malloc(allocationSize);

			// make sure the allocation succeeded
			if (!ppNewMemArray)
				return false;

			// copy any existing memory pointers over
			for (unsigned int i = 0; i < mMemArraySize; ++i)
			{
				ppNewMemArray[i] = mRawMemoryArray[i];
			}

			// allocate a new block of memory
			ppNewMemArray[mMemArraySize] = allocateNewMemoryBlock();  // indexing m_memArraySize here is safe because we haven't incremented it yet to reflect the new size

																	   // attach the block to the end of the current memory list
			if (mHead)
			{
				unsigned char* pCurr = mHead;
				unsigned char* pNext = getNext(mHead);
				while (pNext)
				{
					pCurr = pNext;
					pNext = getNext(pNext);
				}
				setNext(pCurr, ppNewMemArray[mMemArraySize]);
			}
			else
			{
				mHead = ppNewMemArray[mMemArraySize];
			}

			// destroy the old memory array
			if (mRawMemoryArray)
				free(mRawMemoryArray);

			// assign the new memory array and increment the size count
			mRawMemoryArray = ppNewMemArray;
			++mMemArraySize;

			return true;
		}

		unsigned char* allocateNewMemoryBlock() {
			// calculate the size of each block and the size of the actual memory allocation
			size_t blockSize = OBJECT_SIZE + CHUNK_HEADER_SIZE;  // chunk + linked list overhead
			size_t trueSize = blockSize * mNumObjects;

			// allocate the memory
			unsigned char* pNewMem = (unsigned char*)malloc(trueSize);
			if (!pNewMem)
				return NULL;

			// turn the memory into a linked list of chunks
			unsigned char* pEnd = pNewMem + trueSize;
			unsigned char* pCurr = pNewMem;
			while (pCurr < pEnd)
			{
				// calculate the next pointer position
				unsigned char* pNext = pCurr + blockSize;

				// set the next & prev pointers
				unsigned char** ppChunkHeader = (unsigned char**)pCurr;
				ppChunkHeader[0] = (pNext < pEnd ? pNext : NULL);

				// move to the next block
				pCurr += blockSize;
			}

			return pNewMem;
		}

		// internal linked list management
		static unsigned char* getNext(unsigned char* pBlock) {
			unsigned char** ppChunkHeader = (unsigned char**)pBlock;
			return ppChunkHeader[0];
		}

		static void setNext(unsigned char* pBlockToChange, unsigned char* pNewNext) {
			unsigned char** ppChunkHeader = (unsigned char**)pBlockToChange;
			ppChunkHeader[0] = pNewNext;
		}

		// don't allow copy constructor
		MemoryPool(const MemoryPool& memPool) = delete;
	};

}

template <size_t size>
std::shared_ptr<mem::MemoryPool<size>> mem::MemoryPool<size>::sInstance;