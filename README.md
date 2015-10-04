# HeapStack
A speedy way to allocate millions of short memory blocks

This tool allows you to allocate small chunks of memory out of a pool larger blocks, and clean them up with a single delete, and in the process reduce fragmentation.

It works a little like the stack. A large block is allocated, and when you request a pointer for new data the block is used up sequentially until it is full, at which point, another block is added (linked to the last block made, forming a linked list). 

**Why would I want to do this?**

- because new-ing thousands or millions of small objects is very slow.
- because deleting thousands of millions of samall objects is also very slow.
- If you need an unknown but large number of small structures at run-time and want to clean them up with iterating them, or remembering where they were (i.e. a linked list, tree structure, document structure).
- You are streaming chunks of data in from a source where size isn't necessarily known in advance, and you want all the data in memory (you can use getBytes() or getBlocks() to track how much you've streamed and perhaps do somehting more clever if there is just to much data and an error will not suffice).
- When you need short-term storage.

**When not to use it?**

- When your allocations are potentially large. If the allocation is larger than the block size, it will fail. Also, you can potentially waste memory if a block is getting full and there isn't enough room to at the end of the current block (a new bloc will be made). This in practice isn't a big deal.

**Things to know!**

- HeapStack has no "delete" function, upon destruction of the HeapStack object it will drop all it's blocks and all the suballocated data within in them. This means for thousands of allocations, we may need just a few dozen deletes (The blocks themselves).
- You cannot (well, maybe you can) put objects in a HeapStack, the issue would be deleting and calling the destructors on the objects once  you are done. So if they are objects that don't need to be destructed (allocate no additional heap resources,  or handles of any sort), then you can probably use "placement new" (https://en.wikipedia.org/wiki/Placement_syntax)
  to initialize (call the constructor) on an object made with placement new.
- You can get the total memory allocated with getAllocated(), the total size of the data stored with getBytes(),
  and you can get the number of 4k blocks used with getBlocks()
- flatten will allocate a buffer as large as the content of the blocks, then serialize the blocks into the buffer and return the pointer. Note: You have now just used a lot more memory, and you must delete the pointer when you are done (as in: delete [] thePointer)


**The MIT License (MIT)**

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
