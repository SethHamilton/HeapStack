/*
	This tool allows you to allocate small chunks of memory out of a pool larger blocks, 
	and clean them up with a single delete, and in the process reduce fragmentation.

	It works a little like the stack. A large block is allocated, and when you request a pointer 
	for new data the block is used up sequentially until it is full, at which point, another 
	block is added (linked to the last block made, forming a linked list). 

	Why would I want to do this?

	- because new-ing thousands or millions of small objects is very slow.
	- because deleting thousands of millions of samall objects is also very slow.
	- If you need an unknown but large number of small structures at run-time and want to clean them up
	  with iterating them, or remembering where they were (i.e. a linked list, tree structure, document structure).
	- You are streaming chunks of data in from a source where size isn't necessarily known in advance, and you
	  want all the data in memory (you can use getBytes() or getBlocks() to track how much you've streamed and
	  perhaps do somehting more clever if there is just to much data and an error will not suffice).
	- When you need short-term storage.

	When not to use it?

	- When your allocations are potentially large. If the allocation is larger than the 
	  block size, it will fail. Also, you can potentially waste memory if a block is getting full
	  and there isn't enough room to at the end of the current block (a new bloc will be made). This
	  in practice isn't a big deal.

	Things to know!

	- HeapStack has no "delete" function, upon destruction of the HeapStack object it will drop all it's blocks
	  and all the suballocated data within in them. This means for thousands of allocations, we may need just
	  a few dozen deletes.
	- You cannot (well, maybe you can) put objects in a HeapStack, the issue would be deleting the objects once
	  you are done. So if they are objects that don't need to be destructed (allocate no additional heap resources,
	  or handles of any sort), then you can probably use "placement new" (https://en.wikipedia.org/wiki/Placement_syntax)
	  to initialize (call the constructor) on an object made with placement new.
	- You can get the total memory allocated with getAllocated(), the total size of the data stored with getBytes(),
	  and you can get the number of 4k blocks used with getBlocks()
	  	  
*/

#ifndef heapstack_define
#define heapstack_define

#include <cstdio>
#include <cstdint>
#include <iostream>

class HeapStack 
{

private:

	// this is the block structure, blocks of heap memory cast to this type will ultimately
	// become our stack(s). 
	// Note: The pragma ensures the struture is packed and that none of the members are padded to any 
	// particular alignment scheme

#pragma pack(push,1)
	struct block_s
	{
		block_s*  nextBlock;
		int64_t   endOffset;
		char      data[1]; // fake size, we will be casting this over a buffer
	};
#pragma pack(pop)

	int64_t  headerSize = sizeof(block_s) - 1LL; // size of block header, minus the 1 byte 'data' array
	int64_t  blockSize;
	int64_t  dataSize;

	int64_t  blocks;
	int64_t  bytes;

	block_s* head;
	block_s* tail;

public:

	// constructor, default allocates 4 meg blocks. 
	HeapStack( int64_t blocks4k = 1024 ) : // block4k is the number of 4k sections that make a block
		blockSize( blocks4k * 4096LL ),
		dataSize( blockSize - headerSize ),
		blocks(0),
		bytes(0),
		head(NULL),
		tail(NULL)
	{
	}

	~HeapStack() 
	{
		
		block_s* block = head;
		block_s* t;

		while (block) 
		{
			t = block->nextBlock;
			delete[]block;
			block = t;
		}

	}

	// newPtr - returns a pointer to a block of memory of "size"
	char* newPtr(int64_t size)
	{

		if (!tail || tail->endOffset + size >= dataSize)
			newBlock();

	
		char* t = tail->data + tail->endOffset;
		tail->endOffset += size;

		bytes += size;
		 
		return t;

	}

	// currentData - returns a pointer to current memory block
	char* currentData()
	{
		return tail->data;
	}

	// getBytes - returns how many bytes are being used by DATA in the block stack.
	int64_t getBytes()
	{
		return bytes;
	}

	// getAllocated - returns how many bytes are used by the raw blocks in the block stack
	int64_t getAllocated()
	{
		return blocks * blockSize;
	}

	// getBlocks - returns how many blocks are within the block stack
	int64_t getBlocks()
	{
		return blocks;
	}

	// flatten - returns a contiguous block of memory containing the data within all the blocks.
	//
	// Important remember to delete[] the returned char* after you are done with it
	char* flatten() {

		char* buff = new char[bytes];
		char* write = buff;

		block_s* block = head;

		while (block)
		{
			memcpy(write, block->data, block->endOffset);
			write += block->endOffset;

			block = block->nextBlock;
		}

		return buff;

	}

	// flatten - same as basic flatten but returns length via reference param
	char* flatten(int64_t &length) {

		length = bytes;
		return flatten();

	}


private:

	// newBlock - adds a new block to the list of blocks, updates the block links.
	void newBlock() {

		block_s* block = (block_s*)new char[blockSize];

		block->nextBlock = NULL;
		block->endOffset = 0;

		blocks++;

		// link up the blocks, assign the head if needed, move the tail along
		if (tail)
			tail->nextBlock = block;

		if (!head)
			head = block;
		
		tail = block;

	}
	
};

#endif //heapstack_define