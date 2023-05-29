#ifndef HELPERS_H
#define HELPERS_H

#include "icsmm.h"


#define ALIGNMENT 16
#define MIN_BLOCK_SIZE 32

#define MAX_PAGES 5
#define PAGE_SIZE 4096

#define HEADER_SIZE sizeof(ics_header)
#define FOOTER_SIZE sizeof(ics_footer)

#define PROLOGUE_SIZE sizeof(ics_header)
#define EPILOGUE_SIZE sizeof(ics_footer)
#define GET_EPILOGUE_ADDR(newPageStart) ( (ics_footer*)(newPageStart + PAGE_SIZE - EPILOGUE_SIZE) )

#define CALC_ACTUAL_BLOCK_SIZE(size) ((size < ALIGNMENT) ? (ALIGNMENT << 1) : (((size + HEADER_SIZE + FOOTER_SIZE + ALIGNMENT - 1) >> 4) << 4))

#define SET_ALLOCATED_FLAG(blockSize) (blockSize | 0x1)
#define SET_FREE_FLAG(blockSize) (blockSize & ~0x1)
#define IS_ALLOCATED(blockSize, requestedSize) ( ((blockSize & 0x1) == 1) && (requestedSize != 0) )

#define GET_HEADER(currPlayload) ( (ics_free_header*)((char*)(currPlayload) - HEADER_SIZE) )
#define GET_PLAYLOAD(currHeader) ( (void*)((char*)(currHeader) + HEADER_SIZE) )
#define GET_FOOTER(currHeader, currBlockSize) ( (ics_footer*)((char*)(currHeader) + currBlockSize - FOOTER_SIZE) )

#define GET_NEXT_HEADER(currHeader, currBlockSize) ( (ics_free_header*)((char*)(currHeader) + currBlockSize) )
#define GET_NEXT_FOOTER(nextHeader, nextBlockSize) ( (ics_footer*)((char*)(nextHeader) + nextBlockSize - FOOTER_SIZE) )

#define GET_PREV_HEADER(prevFooter, prevBlockSize) ( (ics_free_header*)((char*)(prevFooter) - prevBlockSize + HEADER_SIZE) )
#define GET_PREV_FOOTER(currHeader) ( (ics_footer*)((char*)(currHeader) - FOOTER_SIZE) )


extern unsigned int pagesCount;
extern ics_header *prologue;


int8_t initHeap();

ics_free_header* findNextFit(size_t requestedSize);

ics_free_header* extendHeap(size_t requestedSize);

ics_free_header* getFreelistTail();

void splitBlock(ics_free_header *targetBlock, size_t blockSize);

void* allocateBlock(ics_free_header *targetBlock, size_t blockSize, size_t requestedSize);

int8_t isBlockValid(ics_free_header *block, ics_footer *footer);

int8_t isInHeap(char *block);

int8_t coalesceBlocks(ics_free_header **currBlock);

int8_t coalescePrevBlock(ics_free_header **currBlock, ics_free_header *prevBlock);

int8_t coalesceNextBlock(ics_free_header **currBlock, ics_free_header *nextBlock);

int8_t coalesceBothBlocks(ics_free_header **currBlock, ics_free_header *prevBlock, ics_free_header *nextBlock);

ics_free_header* findBlockInFreelist(ics_free_header *block);

void insertInOrderToFreelist(ics_free_header *block);

ics_footer* getFooter(ics_free_header *block);

#endif
