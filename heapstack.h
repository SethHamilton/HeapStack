/*

Heap Stack - a convenient way to allocation millions of small structures
             quickly and make them go away just as fast.

The MIT License (MIT)

Copyright (c) 2015 Seth A. Hamilton

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

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
	HeapStack(int64_t blocks4k = 1024 ) : // block4k is the number of 4k sections that make a block
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
	inline char* newPtr(int64_t size)
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