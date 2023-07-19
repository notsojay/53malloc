# 53malloc: Dynamic Memory Allocator

## Introduction

* 53malloc is a dynamic memory allocator implemented in C. The project aims to optimize both space utilization and throughput using advanced memory management techniques.

## Features

* Explicit Segregated Free List with Next-fit Placement: This improves memory efficiency by grouping free blocks of similar sizes together. The next-fit placement further enhances performance by reusing the last searched free block if it is large enough.
* Address-Ordered with Boundary Tags: This allows efficient coalescing of free blocks by keeping track of the size of the block both at the beginning and at the end of the block.
* Deferred Coalescing: This approach avoids immediate coalescing, offering a better trade-off between throughput and memory utilization.
* Block Splitting without Creating Splinters: This ensures that splitting blocks to satisfy smaller allocation requests do not leave behind unusable memory splinters.
  
## Project Structure

* The project includes the following key components:

  1. initHeap() function: Initializes the heap space when the first memory request arrives.
  2. findNextFit() function: Implements a Next-Fit algorithm to find a suitable free block from the freelist for allocation requests.
  3. extendHeap() function: Requests more pages from the system when there is no suitable free block in the freelist.
  4. getFreelistTail() function: Fetches the tail of the freelist, which is the last free block.
  5. splitBlock() function: If a free block is larger than the requested size, this function splits it and inserts the new free block into the freelist.
  6. allocateBlock() function: Handles allocation of a suitable block, updating its header and footer, and removing it from the freelist.
  7. isBlockValid() function: Checks whether a block is valid (i.e., allocated and within the heap boundary).
  8. isInHeap() function: Verifies if a pointer is within the heap boundaries.
  9. coalesceBlocks() function: Joins two adjacent free blocks in the freelist into one large free block.
  10. findBlockInFreelist() function: Finds a given block in the freelist.
  11. insertInOrderToFreelist() function: Inserts a block into the freelist in an ordered manner.
  12. getFooter() function: Retrieves the footer of a block given the header.

## Usage

* To use this project, you need to include the header files in your C program. You can then use the ics_malloc() function to allocate memory, similar to how you would use the standard malloc() function. Remember to use ics_free() to free up the memory when it's no longer needed.
* For debugging, make use of the functions and macros provided in debug.h.
* Please note that the exact usage and compilation instructions may depend on your specific project structure and requirements.

## Contributions

* Contributions to this project are welcome. Please ensure your code adheres to the existing style and structure of the project. Contributions can be made via pull requests.

