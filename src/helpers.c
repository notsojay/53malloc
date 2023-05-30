#include "helpers.h"
#include "debug.h"


int8_t
initHeap() 
{
    char *firstPageStart = NULL;
    ics_footer *epilogue = NULL, *footer = NULL;

    if ( ( firstPageStart = (char*)ics_inc_brk() ) == (void*)-1 ) return -1;
    ++pagesCount;

    prologue = (ics_header*)firstPageStart;
    prologue->block_size = 0;
    prologue->block_size = SET_ALLOCATED_FLAG(0);
    prologue->hid = HEADER_MAGIC;
    prologue->requested_size = 0;

    epilogue = GET_EPILOGUE_ADDR(firstPageStart);
    epilogue->block_size = 0;
    epilogue->block_size = SET_ALLOCATED_FLAG(0);
    epilogue->fid = FOOTER_MAGIC;
    epilogue->requested_size = 0;

    freelist_head = (ics_free_header*)(firstPageStart + PROLOGUE_SIZE);
    freelist_head->header.block_size = PAGE_SIZE - PROLOGUE_SIZE - EPILOGUE_SIZE;
    freelist_head->header.hid = HEADER_MAGIC;
    freelist_head->header.requested_size = 0;
    freelist_head->next = NULL;
    freelist_head->prev = NULL;

    footer = initFooter(freelist_head);
    (void)footer;

    freelist_next = freelist_head;

    return 1;
}

ics_footer*
initFooter(ics_free_header *block)
{
    ics_footer *footer = GET_CURR_FOOTER(block, CLEAR_ALLOCATED_FLAG(block->header.block_size));
    footer->block_size = block->header.block_size;
    footer->fid = FOOTER_MAGIC;
    footer->requested_size = block->header.requested_size;
    return footer;
}

ics_free_header*
findNextFit(size_t requestedSize) 
{
    ics_free_header *temp = freelist_next;

    while(freelist_next) 
    {
        if(freelist_next->header.block_size >= requestedSize) return freelist_next;
        freelist_next = freelist_next->next;
    }

    freelist_next = freelist_head;
    while(freelist_next != temp) 
    {
        if(freelist_next->header.block_size >= requestedSize) return freelist_next;
        freelist_next = freelist_next->next;
    }

    return NULL;
}

ics_free_header*
extendHeap(size_t requestedSize) 
{
    if(pagesCount >= MAX_PAGES) return NULL;

    char *newPageStart = NULL;
    ics_free_header *freelist_tail = NULL, *lastBlock = NULL;
    ics_footer *newEpilogue = NULL, *newFooter = NULL, *lastFooter = NULL;
    size_t newFreeBlockSize = 0;

    lastFooter = (ics_footer*)((char*)(ics_get_brk()) - EPILOGUE_SIZE - FOOTER_SIZE);
    lastBlock = (ics_free_header*)((char*)(lastFooter) - lastFooter->block_size + HEADER_SIZE);
    freelist_tail = getFreelistTail();

    if( (newPageStart = (char*) ics_inc_brk() ) == (void*)-1 ) return NULL;
    ++pagesCount;

    if(freelist_tail != lastBlock) 
    {
        freelist_tail = NULL;
        newFreeBlockSize = PAGE_SIZE;
    }
    else
    {
        newFreeBlockSize = freelist_tail->header.block_size + PAGE_SIZE;
    }

    while(newFreeBlockSize < requestedSize) 
    {
        if(pagesCount >= MAX_PAGES) return NULL;
        if( (newPageStart = (char*) ics_inc_brk() ) == (void*)-1 ) return NULL;
        ++pagesCount;
        newFreeBlockSize += PAGE_SIZE;
    }

    if(!freelist_tail) 
    {
        freelist_tail = (ics_free_header*)(newPageStart - EPILOGUE_SIZE);
        freelist_tail->header.hid = HEADER_MAGIC;
        freelist_tail->header.requested_size = 0;
        freelist_tail->prev = NULL;
        freelist_tail->next = NULL;
        insertInOrderToFreelist(freelist_tail);
    }
    freelist_tail->header.block_size = newFreeBlockSize;

    newFooter = initFooter(freelist_tail);
    (void)newFooter;

    newEpilogue = GET_EPILOGUE_ADDR(newPageStart);
    newEpilogue->block_size = SET_ALLOCATED_FLAG(0);
    newEpilogue->fid = FOOTER_MAGIC;
    newEpilogue->requested_size = 0;

    return freelist_tail;
}

ics_free_header* 
getFreelistTail() 
{
    if(!freelist_head) return NULL;

    ics_free_header *freelist_tail = freelist_head;

    while(freelist_tail->next) 
    {
        freelist_tail = freelist_tail->next;
    }

    return freelist_tail;
}

void
splitBlock(ics_free_header *targetBlock, size_t blockSize) 
{
    ics_free_header *newBlock = NULL;
    ics_footer *newBlockFooter = NULL;

    newBlock = (ics_free_header*)( (char*)targetBlock + blockSize );
    newBlock->header.block_size = targetBlock->header.block_size - blockSize;
    newBlock->header.hid = HEADER_MAGIC;
    newBlock->header.requested_size = 0;

    newBlockFooter = initFooter(newBlock);
    (void)newBlockFooter;

    newBlock->next = targetBlock->next;
    newBlock->prev = targetBlock;
    if(targetBlock->next) targetBlock->next->prev = newBlock;
    targetBlock->next = newBlock;
}

void*
allocateBlock(ics_free_header *targetBlock, size_t blockSize, size_t requestedSize) 
{
    ics_footer *targetBlockFooter = NULL;

    targetBlock->header.block_size = blockSize;
    targetBlock->header.block_size = SET_ALLOCATED_FLAG(targetBlock->header.block_size);
    targetBlock->header.requested_size = requestedSize;

    targetBlockFooter = initFooter(targetBlock);
    (void)targetBlockFooter;

    if(targetBlock == freelist_head) freelist_head = targetBlock->next;

    if(targetBlock->next) freelist_next = targetBlock->next;
    else freelist_next = freelist_head;

    if(targetBlock->prev) targetBlock->prev->next = targetBlock->next;
    if(targetBlock->next) targetBlock->next->prev = targetBlock->prev;
    targetBlock->next = NULL;
    targetBlock->prev = NULL;
    
    return GET_CURR_PLAYLOAD(targetBlock);
}

int8_t
isBlockValid(ics_free_header *block, ics_footer *footer)
{
    if( isInHeap( (char*)block ) == -1 ||
        isInHeap( (char*)footer ) == -1 ||
        block->header.hid != HEADER_MAGIC ||
        footer->fid != FOOTER_MAGIC ||
        block->header.block_size != footer->block_size ||
        block->header.requested_size != footer->requested_size ||
        !IS_ALLOCATED(block->header.block_size, block->header.requested_size) ||
        !IS_ALLOCATED(footer->block_size, footer->requested_size) )
    {
        return -1;
    }
    return 1;
}

int8_t
isInHeap(char *block)
{
    return ( block >= (char*)((char*)(prologue) + PROLOGUE_SIZE) && 
             block < (char*)((char*)(ics_get_brk()) - EPILOGUE_SIZE) ) ?
                1 : -1;
}

int8_t
coalesceBlocks(ics_free_header **currBlock, ics_footer **currFooter)
{
    ics_footer *prevFooter = NULL, *nextFooter = NULL;
    ics_free_header *prevBlock = NULL, *nextBlock = NULL;
    char *temp = NULL;
    int8_t isPrevFree = 0, isNextFree = 0;

    temp = (char*)GET_PREV_FOOTER(*currBlock);
    if(isInHeap(temp) == 1) 
    {
        prevFooter = (ics_footer*)temp;
        temp = (char*)GET_PREV_HEADER(prevFooter, CLEAR_ALLOCATED_FLAG(prevFooter->block_size));

        if(isInHeap(temp) == 1) prevBlock = (ics_free_header*)temp;
        else prevFooter = NULL;
    }

    temp = (char*)GET_NEXT_HEADER(*currBlock, (*currBlock)->header.block_size);
    if(isInHeap(temp) == 1) 
    {
        nextBlock = (ics_free_header*)temp;
        temp = (char*)GET_NEXT_FOOTER(nextBlock, CLEAR_ALLOCATED_FLAG(nextBlock->header.block_size));

        if(isInHeap(temp) == 1) nextFooter = (ics_footer*)temp;
        else nextBlock = NULL;
    }

    isPrevFree = checkAdjBlockAvailability(prevBlock, prevFooter);
    isNextFree = checkAdjBlockAvailability(nextBlock, nextFooter);

    if(isPrevFree != -1 && isNextFree != -1)
    {
        if( !findBlockInFreelist(prevBlock) ) return -1;
        if( !findBlockInFreelist(nextBlock) ) return -1;
        if(prevBlock == freelist_head) freelist_head = freelist_head->next;
        if(nextBlock == freelist_head) freelist_head = freelist_head->next;
        if(freelist_next == nextBlock) freelist_next = prevBlock;

        coalescePrevBlock(currBlock, prevBlock);
        coalesceNextBlock(currBlock, nextBlock);
    }
    else if(isPrevFree != -1)
    {
        if( !findBlockInFreelist(prevBlock) ) return -1;
        if(prevBlock == freelist_head) freelist_head = freelist_head->next;

        coalescePrevBlock(currBlock, prevBlock);
    }
    else if(isNextFree != -1)
    {
        if( !findBlockInFreelist(nextBlock) ) return -1;
        if(nextBlock == freelist_head) freelist_head = freelist_head->next;
        if(freelist_next == nextBlock) freelist_next = *currBlock;

        coalesceNextBlock(currBlock, nextBlock);
    }

    *currFooter = initFooter(*currBlock);

    return 1;
}

void
coalescePrevBlock(ics_free_header **currBlock, ics_free_header *prevBlock)
{
    if(prevBlock->prev) prevBlock->prev->next = prevBlock->next;
    if(prevBlock->next) prevBlock->next->prev = prevBlock->prev;
    prevBlock->prev = NULL;
    prevBlock->next = NULL;

    prevBlock->header.block_size += (*currBlock)->header.block_size;
    *currBlock = prevBlock;
}

void
coalesceNextBlock(ics_free_header **currBlock, ics_free_header *nextBlock)
{
    if(nextBlock->prev) nextBlock->prev->next = nextBlock->next;
    if(nextBlock->next) nextBlock->next->prev = nextBlock->prev;
    nextBlock->prev = NULL;
    nextBlock->next = NULL;

    (*currBlock)->header.block_size += nextBlock->header.block_size;
}

ics_free_header*
findBlockInFreelist(ics_free_header *block)
{
    if(!block || !freelist_head) return NULL;

    ics_free_header *current = freelist_head;
    
    while(current && current != block)
    {
        current = current->next;
    }

    return current;
}

int8_t
checkAdjBlockAvailability(ics_free_header *block, ics_footer *footer)
{
     if( block && footer &&
        !IS_ALLOCATED(footer->block_size, footer->requested_size) && 
        !IS_ALLOCATED(block->header.block_size, block->header.requested_size) )
    {
        return 1;
    }
    return -1;
}

void 
insertInOrderToFreelist(ics_free_header *block)
{
    if(!block) return;

    if(!freelist_head)
    {
        freelist_head = block;
        freelist_next = block;
        return;
    }
    if(block < freelist_head)
    {
        block->next = freelist_head;
        freelist_head->prev = block;
        freelist_head = block;
        return;
    }

    ics_free_header *current = freelist_head;

    while(1)
    {
        if(current && current > block)
        {
            block->next = current;
            block->prev = current->prev;
            current->prev->next = block;
            current->prev = block;
            return;
        }
        if(!current->next)
        {
            block->prev = current;
            block->next = NULL;
            current->next = block;
            return;
        }
        current = current->next;
    }
}
