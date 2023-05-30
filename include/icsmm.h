#ifndef ICSMM_H
#define ICSMM_H


#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define HEADER_MAGIC 0x0badbee5UL
#define FOOTER_MAGIC 0xfaceba5eUL

#define REQUEST_SIZE_BITS 16
#define HID_SIZE_BITS 32
#define BLOCK_SIZE_BITS 16
#define FID_SIZE_BITS 32

#define ALIGNMENT 16
#define MIN_BLOCK_SIZE 32

#define MAX_PAGES 5
#define PAGE_SIZE 4096

#define PROLOGUE_SIZE sizeof(ics_header)
#define EPILOGUE_SIZE sizeof(ics_footer)
#define GET_EPILOGUE_ADDR(newPageStart) ( (ics_footer*)(newPageStart + PAGE_SIZE - EPILOGUE_SIZE) )

#define HEADER_SIZE sizeof(ics_header)
#define FOOTER_SIZE sizeof(ics_footer)
#define CALC_ACTUAL_BLOCK_SIZE(size) ((size < ALIGNMENT) ? (ALIGNMENT << 1) : (((size + HEADER_SIZE + FOOTER_SIZE + ALIGNMENT - 1) >> 4) << 4))

#define SET_ALLOCATED_FLAG(blockSize) (blockSize | 0x1)
#define CLEAR_ALLOCATED_FLAG(blockSize) (blockSize & ~0x1)
#define IS_ALLOCATED(blockSize, requestedSize) ( ((blockSize & 0x1) == 1) && (requestedSize != 0) )

#define GET_CURR_HEADER(currPlayload) ( (ics_free_header*)((char*)(currPlayload) - HEADER_SIZE) )
#define GET_CURR_PLAYLOAD(currHeader) ( (void*)((char*)(currHeader) + HEADER_SIZE) )
#define GET_CURR_FOOTER(currHeader, currBlockSize) ( (ics_footer*)((char*)(currHeader) + currBlockSize - FOOTER_SIZE) )

#define GET_NEXT_HEADER(currHeader, currBlockSize) ( (ics_free_header*)((char*)(currHeader) + currBlockSize) )
#define GET_NEXT_FOOTER(nextHeader, nextBlockSize) ( (ics_footer*)((char*)(nextHeader) + nextBlockSize - FOOTER_SIZE) )

#define GET_PREV_HEADER(prevFooter, prevBlockSize) ( (ics_free_header*)((char*)(prevFooter) - prevBlockSize + HEADER_SIZE) )
#define GET_PREV_FOOTER(currHeader) ( (ics_footer*)((char*)(currHeader) - FOOTER_SIZE) )


typedef struct __attribute__((__packed__)) {
    uint64_t block_size: BLOCK_SIZE_BITS;
    uint64_t hid: HID_SIZE_BITS;
    uint64_t requested_size: REQUEST_SIZE_BITS;
} ics_header;

typedef struct __attribute__((__packed__)) ics_free_header {
    ics_header header;
    struct ics_free_header *next;
    struct ics_free_header *prev;
} ics_free_header;

typedef struct __attribute__((__packed__)) ics_footer {
    uint64_t block_size: BLOCK_SIZE_BITS;
    uint64_t fid: FID_SIZE_BITS;
    uint64_t requested_size: REQUEST_SIZE_BITS;
} ics_footer;


extern ics_free_header *freelist_head;
extern ics_free_header *freelist_next;
extern unsigned int pagesCount;
extern ics_header *prologue;


void *ics_malloc(size_t size);

void *ics_realloc(void *ptr, size_t size);

int ics_free(void *ptr);

void ics_mem_init();

void ics_mem_fini();

void *ics_get_brk();

void *ics_inc_brk();

void ics_freelist_print();

int ics_header_print(void *header);

int ics_payload_print(void *payload);

void ics_freelist_print_compact();

int ics_header_print_compact(void *header);

int ics_payload_print_compact(void *payload);


#endif