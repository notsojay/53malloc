#include "icsmm.h"
#include "debug.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * The allocator MUST store the head of its free list in this variable. 
 * Doing so will make it accessible via the extern keyword.
 * This will allow ics_freelist_print to access the value from a different file.
 */
ics_free_header *freelist_head = NULL;

/*
 * The allocator MUST use this pointer to refer to the position in the free list to 
 * starting searching from. 
 */
ics_free_header *freelist_next = NULL;

/*
 * This is your implementation of malloc. It acquires uninitialized memory from  
 * ics_inc_brk() that is 16-byte aligned, as needed.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, the pointer to a valid region of memory of at least the
 * requested size is returned. Otherwise, NULL is returned and errno is set to 
 * ENOMEM - representing failure to allocate space for the request.
 * 
 * If size is 0, then NULL is returned and errno is set to EINVAL - representing
 * an invalid request.
 */
void*
ics_malloc(size_t size) 
{
    ics_free_header *targetBlock = NULL;
    size_t blockSize = 0;

    if(size == 0) return errno = EINVAL, NULL;

    if( pagesCount == 0 &&
        initHeap() == -1 ) 
    {
        errno = ENOMEM;
        return NULL;
    }

    blockSize = CALC_ACTUAL_BLOCK_SIZE(size);

    if( !( targetBlock = findNextFit(blockSize) ) &&
        !( targetBlock = extendHeap(size) ) ) 
    {
        errno = ENOMEM;
        return NULL;
    }

    if(targetBlock->header.block_size - blockSize >= MIN_BLOCK_SIZE)
        splitBlock(targetBlock, blockSize);
    else
        blockSize = targetBlock->header.block_size;

    return allocateBlock(targetBlock, blockSize, size);
}

/*
 * Marks a dynamically allocated block as no longer in use and coalesces with 
 * adjacent free blocks (as specified by Homework Document). 
 * Adds the block to the appropriate bucket according to the block placement policy.
 *
 * @param ptr Address of dynamically allocated memory returned by the function
 * ics_malloc.
 * 
 * @return 0 upon success, -1 if error and set errno accordingly.
 * 
 * If the address of the memory being freed is not valid, this function sets errno
 * to EINVAL. To determine if a ptr is not valid, (i) the header and footer are in
 * the managed  heap space, (ii) check the hid field of the ptr's header for
 * special value (iii) check the fid field of the ptr's footer for special value,
 * (iv) check that the block_size in the ptr's header and footer are equal, (v) 
 * the allocated bit is set in both ptr's header and footer, and (vi) the 
 * requested_size is identical in the header and footer.
 */
int
ics_free(void *ptr) 
{
    ics_free_header *block = NULL;
    ics_footer *footer = NULL;

    if(!ptr) return errno = EINVAL, -1;

    block = GET_HEADER(ptr);
    footer = GET_FOOTER(block, SET_FREE_FLAG(block->header.block_size));
    if(isBlockValid(block, footer) == -1) return errno = EINVAL, -1;

    block->header.block_size = SET_FREE_FLAG(block->header.block_size);
    block->header.requested_size = 0;
    block->next = NULL;
    block->prev = NULL;

    if( coalesceBlocks(&block) == -1 ) return errno = ENOMEM, -1;

    footer = getFooter(block);

    insertInOrderToFreelist(block);

    return 0;
}

/*
 * Resizes the dynamically allocated memory, pointed to by ptr, to at least size 
 * bytes. See Homework Document for specific description.
 *
 * @param ptr Address of the previously allocated memory region.
 * @param size The minimum size to resize the allocated memory to.
 * @return If successful, the pointer to the block of allocated memory is
 * returned. Else, NULL is returned and errno is set appropriately.
 *
 * If there is no memory available ics_malloc will set errno to ENOMEM. 
 *
 * If ics_realloc is called with an invalid pointer, set errno to EINVAL. See ics_free
 * for more details.
 *
 * If ics_realloc is called with a valid pointer and a size of 0, the allocated     
 * block is free'd and return NULL.
 */
void*
ics_realloc(void *ptr, size_t size)
{
    ics_free_header *oldBlock = NULL, *newBlock = NULL;
    size_t oldPlayloadSize = 0;
    void *newPtr = NULL;

    if(!ptr) return ics_malloc(size);
    if(size == 0) return ics_free(ptr), NULL;

    oldBlock = GET_HEADER(ptr);
    oldPlayloadSize = oldBlock->header.block_size - HEADER_SIZE - FOOTER_SIZE; // When copying the old data to the new memory block, only the payload part is to be copied, because the header and footer may be different in the new memory block.

    if(oldPlayloadSize == size) return ptr;

    if( (newBlock = ics_malloc(size)) == NULL ) return errno = ENOMEM, NULL;

    if(oldPlayloadSize > size) memcpy(newBlock, ptr, size);
    else memcpy(newBlock, ptr, oldPlayloadSize);
        
    newPtr = newBlock;
    ics_free(ptr);
    
    return newPtr;
}