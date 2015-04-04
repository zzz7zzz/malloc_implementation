// hi!
//http://csapp.cs.cmu.edu/public/ics2/code/vm/malloc/mm.c
/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Kate and Anna's Team",
    /* First member's full name */
    "Zhiming Zeng",
    /* First member's email address */
    "zzeng9@u.rochester.edu",
    /* Second member's full name (leave blank if none) */
    "Anna Thornton",
    /* Second member's email address (leave blank if none) */
    "athornt3@u.rochester.edu"
};

/* 16 byte alignment */
#define ALIGNMENT 16      /*alignment in bytes*/
#define WSIZE 4           /*Word size in bytes*/
#define DSIZE 8           /*Double word size in bytes*/
#define CHUNKSIZE (1<<12) /*Initial heap size*/

//MINBLOCKSIZE is 16;
#define MIN_BLOCK_SIZE 16

#define MAX(x,y) ((x) >(y)? (x) : (y))

/*Pack a size and allocated bit into a word*/
#define PACK(size,alloc) ((size | alloc))

/*Reading and writing a word at address p*/
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/*Read the size and allocated bit from the address p*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/*Compute the address of the header and footer given block pointer bp*/
#define HDRP(bp) ((char *)(bp) -3*WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - 2*DSIZE)

/*Compute the address of the next block or previous block in the heap*/
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - 3*WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - 4*WSIZE)))

/*pointers to the next and previous free blocks*/
#define succ(bp) (HDRP((char *)(bp)) + WSIZE) 
#define pred(bp) (HDRP((char *)(bp)) + DSIZE) 

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT) -1) & ~(ALIGNMENT- 1))


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*global variables*/
static char *heap_listp; /*pointer to the first block*/
static char *freelist_start;
static char *freelist_end;
int verbose = 1;

/*Function prototypes for healper routines*/
static void *extend_heap(size_t words);
static void place(void *bp, size_t size);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkblock(void *bp);
static void add_free(void *bp);
static void remove_free(void *bp);
static int mm_check(void);
static int inbounds(void *ptr);





/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {

    /*create initial empty heap*/
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1){
        return -1;
    }
    PUT(heap_listp, 0);                            /*alignment padding*/
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE,1));    /*prologue header*/
    PUT(heap_listp + (2* WSIZE), PACK(DSIZE,1));   /*prologue footer*/
    PUT(heap_listp + (3*WSIZE), PACK(0,1));        /*epilogue header*/
    heap_listp += (2*WSIZE);

    /*Extend the empty heap with a free block of CHUNKSIZE bytes*/
    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
        return -1;

    freelist_start = NULL;
    freelist_end = NULL;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    
    if(size <= 0){
        return NULL;
    }
    int newsize = ALIGN(size + DSIZE); //DSIZE = overhead of header and footer


    //if we can fit it anywhere in the free list
        //find the first block
        //remove from free list
        //mark as allocated
        //set footer to point to the header
            //if the block is larger than the needed space
                //the new split portion of the block needs a header and footer
                //add this new block to the free list
    //otherwise request more memory and return the block pointer
        //create header and footer and add this block to the free list, add previous and next


    // if (p == (void *)-1) {
	   //  return NULL;
    // } else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) 
{
    //if pointer is invalid return
    //else get the header of the pointer
    //mark the header as free
    //coalesce with adjacent blocks
    //add block to the free list
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL) {
      return NULL;
    }

    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize) {
      copySize = size;
    }

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);

    return newptr;
}

/*Internal helper routines*/

/*
 * add_free adds the block to the front of the add list
 */
void add_free(void *bp){
    if(!bp){
        return;
    }

    /*If the free list is empty*/
    if(freelist_start == NULL){
        freelist_start = bp;                            //set the start of the list to bp
        PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 0));   //change the header of bp to indicate it is free
        PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 0));   //change the footer of bp to indicate it is free
        PUT((char *)succ(bp), NULL);                    //set the succesor function to point to NULL since it is end of list
        PUT((char *)pred(bp), NULL);                    //set the predecessor function to point to NULL since it is start of list
    }

    /*otherwise the freelist is not empty*/
    else{
        char *temp = freelist_start;                    //keep track of old start of the free list
        PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 0));   //change the header of bp to indicate it is free
        PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 0));   //change the footer of bp to indicate it is free
        PUT((char *)succ(bp), temp);                    //set the successor function to point to the old start of the list
        PUT((char *)pred(bp), NULL);                    //set the predecessor function to point to NULL since it is start of list
        freelist_start = bp;                            //set the start of the list to bp
    }

}

/*
 * remove_free removes a block from the free list
 */
void remove_free(void *bp){

    if(bp == freelist_start){

        char *temp1 = succ(bp);

        PUT((char *)pred(temp1), NULL);
        
        freelist_start = temp1;
        
        PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 1));
        PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 1));
        
        PUT((char *)succ(bp), NULL);
        PUT((char *)pred(bp), NULL);

    }
    if(bp == freelist_end){
        char *temp2 = pred(bp);

        PUT((char *)succ(temp2), NULL);

        freelist_end = temp2;

        PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 1));
        PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 1));
        
        PUT((char *)succ(bp), NULL);
        PUT((char *)pred(bp), NULL);

    }
    else{
    
    //change the pointers that are pointing to it
    char *temp1 = succ(bp);
    char *temp2 = pred(bp);

    PUT((char *)succ(temp2), temp1);
    PUT((char *)pred(temp1), temp2);

    //clear its pointers
    PUT((char *)succ(bp), NULL); //NULL OR 0?
    PUT((char *)pred(bp), NULL);
    

    //set the allocated bits to 1
    PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 1));
    PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 1));

    
    }
    return;
}

/*
 * heap checker checks:
 *      is every block in the free list marked as free
 *      are there any contiguous free blocks that somehow escaped coalescing
 *      is every free block actually in the free list
 *      do the pointers in the free list point to valid free blocks
 *      do any allocated blocks overlap
 *      do the pointers in a heap block point to valid heap addresses
 */
static int mm_check(void){
    return 0;
}

/*
 * inbounds checks if the pointer points to a valid heap address
 */
int inbounds(void *ptr){
    return(ptr >= mem_heap_lo() && ptr < mem_heap_hi());
}

/*
 * extend_heap -Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    /*Allocate an even number of words to maintain alignment*/
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /*Initialize free block header and footer and the epilouge header*/
    PUT(HDRP(bp), PACK(size,0));            /*free block header*/
    PUT(FTRP(bp), PACK(size,0));            /*free block footer*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));    /*new epilogue header*/

    /*Coalesce if the previous block was free*/
    return coalesce(bp);
}

/*
 * coalesce - boundary tag coalescing, return ptr to coalesced block
 */
 static void *coalesce(void *bp)
 {
    //check for memory in bounds here
    //if you coalesce you need to remove the block from the free list and reset its pointers
    //then create header and footer of block
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){
        return bp;
    }
    else if(prev_alloc && !next_alloc){
        size+= GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size,0));
        return(bp);
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        return(PREV_BLKP(bp));
    }
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        return(PREV_BLKP(bp));
    }
 }

/*
 * find_fit finds the first fitting block from the free list
 */
 void *find_fit(size_t asize){

    char *current = freelist_start;         // get the start of the free list
    while(current!=NULL){
        if(GET_SIZE(current) >= asize){
            return current;                 //return that block if it is a fit
        }
        current = (char *) GET((char *)succ(current));       // else go to the next block in the free list
    }

    return NULL;
 }

 void place(void *bp, size_t size){
    return;
 }

 void printblock(void *bp){
    return;
 }

 void checkblock(void *bp){
    return;
 }
