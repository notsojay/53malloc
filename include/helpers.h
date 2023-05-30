#ifndef HELPERS_H
#define HELPERS_H


#include "icsmm.h"


int8_t initHeap();

ics_footer* initFooter(ics_free_header *block);

ics_free_header* findNextFit(size_t requestedSize);

ics_free_header* extendHeap(size_t requestedSize);

ics_free_header* getFreelistTail();

void splitBlock(ics_free_header *targetBlock, size_t blockSize);

void* allocateBlock(ics_free_header *targetBlock, size_t blockSize, size_t requestedSize);

int8_t isBlockValid(ics_free_header *block, ics_footer *footer);

int8_t isInHeap(char *block);

int8_t coalesceBlocks(ics_free_header **currBlock, ics_footer **currFooter);

void coalescePrevBlock(ics_free_header **currBlock, ics_free_header *prevBlock);

void coalesceNextBlock(ics_free_header **currBlock, ics_free_header *nextBlock);

ics_free_header* findBlockInFreelist(ics_free_header *block);

int8_t checkAdjBlockAvailability(ics_free_header *block, ics_footer *footer);

void insertInOrderToFreelist(ics_free_header *block);


#endif
