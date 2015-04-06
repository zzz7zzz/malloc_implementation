/* Monday Apr 6 morning Kate*/
/*
 * mm.c 
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
#define ALIGNMENT 16      /*Alignment in bytes*/
#define WSIZE 4           /*Word size in bytes*/
#define DSIZE 8           /*Double word size in bytes*/
#define CHUNKSIZE (1<<12) /*Initial heap size. Divisible by 16 byte alignment*/

/* MINBLOCKSIZE is 16 */
#define MIN_BLOCK_SIZE 16

#define MAX(x,y) ((x) >(y)? (x) : (y))

/*Pack a size and allocated bit into a word*/
#define PACK(size,alloc) ((size | alloc))

/*Reading and writing a word at address p*/
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/*Read the size and allocated bit from the address p*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2) // not used for now
#define GET_ALLOC(p) (GET(p) & 0x1)

/*Compute the address of the header and footer given block pointer bp which points to payload in allocated list so no pred and succ*/
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/*Compute the address of the next block or previous block in the heap in free list. Points to hp of block*/ 
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - WSIZE)))

/*pointers to the next and previous free blocks. p points to the start of the block, not the payload. */
#define succ(hp) ((char *)(hp) + WSIZE) 
#define pred(hp) ((char *)(hp) + DSIZE) 

/*rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT) -1) & ~(ALIGNMENT- 1))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*global variables*/
static char *heap_listp; /*pointer to the first block*/
static char *freelist_start; /* points to hp + WSIZE*/
static char *freelist_end; /* points to epilogue*/

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
    printf("heap_listp = mem_sbrk is %x\n",heap_listp);
    PUT(heap_listp, 0);                            /*alignment padding*/
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE,1));    /*prologue header*/
    PUT(heap_listp + (2* WSIZE), PACK(DSIZE,1));   /*prologue footer*/
    PUT(heap_listp + (3*WSIZE), PACK(0,1));        /*epilogue header. any newly created blocks will set its header here*/

    heap_listp += (2*WSIZE);
    printf("pack(DSIZE,1) gives %x\n", PACK(DSIZE,1));
    printf("heap_listp now is %x\n",heap_listp);
     
    /*Extend the empty heap with a free block of CHUNKSIZE bytes*/
    printf("chunksize is %d and chunksize/wordsize is %d\n",CHUNKSIZE,CHUNKSIZE/WSIZE);
    if((freelist_start = extend_heap(CHUNKSIZE/WSIZE))==NULL)
        return -1;
     printf("freelist_start  is %x\n",freelist_start);
 
    freelist_end = FTRP(freelist_start);
    return 0;
}


/* 
 * mm_malloc - Allocate a block.
 *     Always allocate a block whose size is a multiple of the alignment.
 *     size refers to payload. 
 *     returns a pointer to the payload
 */
void *mm_malloc(size_t size) {
    size_t newsize; /* adjusted block size that includes header and footer*/
    size_t increasesize;
    char *bp;
    printf("enter mm_malloc whee---------------------------------------------------\n");
    
    
    if (freelist_start == NULL){
      printf("freelist_start is null in mm_malloc\n");
      mm_init();
      freelist_start = heap_listp + 2*WSIZE;
    PUT(HDRP(freelist_start),PACK(CHUNKSIZE,0));
    PUT(FTRP(freelist_start),PACK(CHUNKSIZE,0));

    printf("freelist start  address is %x\n", freelist_start);
    printf("freelist start header address is %x\n", HDRP(freelist_start));
    printf("freelist start footer address is %x\n", FTRP(freelist_start));
    }
    
    if(size <= 0){
        return NULL;
    }
    
    newsize = ALIGN(size + 8); /* include header and footer */
    
    
    /* Search a fit in the free list and place it in the block*/
    /* bp points to payload */
    if ((bp = find_fit(newsize)) != NULL) {                     //if we can fit it anywhere in the free list
      printblock(bp);
      printf("In mm_malloc, find fit returns this bp\n");
      printblock(bp);
      place(bp, newsize); //will call free list stuff           //call place function to add it
      printf("In mm_malloc, place Returns this pointer**************************:\n");
      printblock(bp);
      return bp;                                                //return the block pointer to the newly allocated block
    }
    
    /* No fit found. Extend the heap size */
    printf("no fit found in mm_malloc\n");
    increasesize = MAX(newsize, CHUNKSIZE);                     //and if the needed space is larger than our normal chunksize
    if ((bp = extend_heap(increasesize/WSIZE)) == NULL)         //extend the heap by the needed space
      return NULL;                                              //error catching
    
    // set extra space to front of free list 
    
    /*allocate the needed space from the new chunk*/
    place(bp, newsize);
    printf("Returns this pointer**************************:\n");
    printblock(bp);
    return bp;
    
}



/*
 * mm_free - set the block as free and coalesce
 */
void mm_free(void *ptr) 
{
    
//if pointer is invalid return
    //else get the header of the pointer
    //mark the header as free
    //coalesce with adjacent blocks
  // se prev and next pointers to point to 
    //add block to the free list
  printf("enter mm_free\n");
  size_t size = GET_SIZE(HDRP(ptr));
  
  if (ptr == NULL)
    return NULL;
  
  PUT((char *)HDRP(ptr), PACK(size, 0)); 
  PUT((char *)FTRP(ptr), PACK(size, 0)); 
  add_free(ptr);
  //coalesce(ptr);
  
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
    /*if(!bp){
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    //If the free list is empty
    if(freelist_start == NULL){
        printf("freelist_start is NULL in add_free\n");
        freelist_start = bp;                            //set the start of the list to bp
        PUT((char *)HDRP(bp), PACK(size, 0));   //change the header of bp to indicate it is free
        PUT((char *)FTRP(bp), PACK(size, 0));   //change the footer of bp to indicate it is free
        
        PUT((char *)pred(bp), heap_listp - WSIZE);                    //set the predecessor function to point to NULL since it is start of list
	freelist_start = bp;
	freelist_end = bp + size - WSIZE;
	PUT((char *)succ(bp), freelist_end);                    //set the succesor function to point to NULL since it is end of list
    }

    //otherwise the freelist is not empty
    else{
      printf("freelist_start is NOT  NULL in add_free\n");
        char *temp;
	temp = freelist_start - WSIZE;                    //keep track of old start of the free list
        PUT((char *)HDRP(bp), PACK(size, 0));   //change the header of bp to indicate it is free
        PUT((char *)FTRP(bp), PACK(size, 0));   //change the footer of bp to indicate it is free
        PUT((char *)succ(bp), temp);                    //set the successor function to point to the old start of the list
        PUT((char *)pred(bp), heap_listp - WSIZE);                    //set the predecessor function to point to NULL since it is start of list
        freelist_start = bp;                            //set the start of the list to bp
    }*/
    
    if(!bp){
        return;
    }
size_t size = GET_SIZE(HDRP(bp));
    /*If the free list is empty*/
    if(freelist_start == NULL){
        freelist_start = bp;                            //set the start of the list to bp
        PUT((char *)HDRP(bp), PACK(size, 0));   //change the header of bp to indicate it is free
        PUT((char *)FTRP(bp), PACK(size, 0));   //change the footer of bp to indicate it is free
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
 * remove_free removes a block from the free list by updating succ and pred pointers, and freelist_start and freelist_end. 
 * Not in charge of setting header and footer.
 */
void remove_free(void *bp){
    
    if (pred(bp) == (heap_listp - WSIZE) && succ(bp) == freelist_end) {
      printf("remove entire free list?!\n"); 
      freelist_start = NULL; // ???
       freelist_end = NULL;
       return;
    }
  
    //  bp == freelist_start
    if(pred(bp) == (heap_listp - WSIZE)){
	printf("remove free block at the start\n"); 
        char *temp1 = succ(bp);

        PUT((char *) (pred(temp1) + DSIZE), heap_listp - WSIZE);
        
        freelist_start = temp1 + WSIZE;
        return;
        //PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 1));
        //PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 1));

    }
    
    //bp == freelist_end
    if(succ(bp) == freelist_end){
	printf("remove free block at the end\n"); 
        char *temp2 = pred(bp);

        PUT((char *)succ(temp2 + WSIZE), freelist_end);

        //freelist_end = temp2;

        //PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 1));
        //PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 1));
        
       // PUT((char *)succ(bp), NULL);
       // PUT((char *)pred(bp), NULL);
        return;
    }
 
 
    
    //change the pointers that are pointing to it
    char *temp1 = succ(bp);
    char *temp2 = pred(bp);

    PUT((char *)succ(temp2 + WSIZE), temp1);
    PUT((char *)pred(temp1 + WSIZE), temp2);

    //clear its pointers
    //PUT((char *)succ(bp), NULL); //NULL OR 0?
    //PUT((char *)pred(bp), NULL);
    

    //set the allocated bits to 1
    //PUT((char *)HDRP(bp), PACK(GET_SIZE(bp), 1));
    //PUT((char *)FTRP(bp), PACK(GET_SIZE(bp), 1));

    
    
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
    size_t remainder;
    
    remainder = words % 4;
    
    /*Allocate words (multiple of 4) to maintain alignment*/
    size = (remainder) ? (words+ (WSIZE - remainder)) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /*Initialize free block header and footer and the epilouge header*/
    PUT(HDRP(bp), PACK(size,0));            /*free block header*/
    PUT(FTRP(bp), PACK(size,0));            /*free block footer*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));    /*new epilogue header*/
    freelist_end = HDRP(NEXT_BLKP(bp));     /* set freelist_end to new epilogue*/
    PUT(succ(bp - WSIZE), freelist_end);	    /* set succ of extended block to hp of epilogue*/
    PUT(pred(bp - WSIZE), heap_listp - WSIZE);      /* set pred of extended block to hp of prologue*/
    printf("word is %d and size of extend heap is %d and size is %d\n",words, (int)GET_SIZE(FTRP(bp)),size); // size of extend heap is 0 and size is 4096???

    printf("bp header addres is %x and bp footer address is %x and bp address is %x\n", HDRP(bp),FTRP(bp),bp); 
    printf("header of next block is %x\n",HDRP(NEXT_BLKP(bp)));
    printblock(bp);
   
 
   return bp;
    
    /*Coalesce if the previous block was free*/
    /*char * ptr = (char *) coalesce(bp);
    
    printf("header address of coalesce bp is %x and bp address is:\n");
    printblock(ptr);
    abort();
    return ptr;*/
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
 * find_fit finds the first fit block and returns a pointer to the start of the payload
 * size is the no. of bytes needed for header, footer and payload in allocated list.
 * size should be aligned. 
 */
 void *find_fit(size_t size){
   char * current;
   char * succ_address;
   printf("enter find_fit\n");
   // if free list is empty, return NULL
   if (freelist_start == NULL) {
    printf("free list start null\n");
     return NULL;
   }
   // check size is aligned;
   if (size != ALIGN(size)) {
     printf("size %d is not aligned!\n", size);
     size = ALIGN(size);
   }
   
   current = freelist_start;
   printf("find fit 2\n");
   // if free list finds non-null block with right size, return address of block
   while (current != NULL && GET_ALLOC(HDRP(current)) == 0) {
     printf("find fit while loop\n");
      if (GET_SIZE(HDRP(current)) == size) {
	printf("in while loop of find_fit()\n");
//printblock(current);
             return current; //return pointer to payload 
      }
      printf("find fit 3\n");
      if (GET_SIZE(HDRP(current)) > size) {
	// split block
	size_t sizeDiff;
	printf("in find_fit(), split block bp %x with size %d\n", current, size);
        sizeDiff = GET_SIZE(HDRP(current)) - size;
	printf("sizeDiff is %d\n", sizeDiff);
        PUT(HDRP(current), PACK(sizeDiff,0));  	/* update header of block that remains in free list*/
        PUT(current + sizeDiff - DSIZE, PACK(sizeDiff,0));  	/* update header of block that remains in free list*/
	printf("in find_fit(), free block is:\n");
	printblock(current);
	 current += (unsigned int) sizeDiff; /* current points to 1 word after the header */
	 printf("find fit returns this splitted current %x\n", current);
	 
	 PUT(HDRP(current), PACK(size,0));
        PUT(FTRP(current), PACK(size,0));
	 return current;
      } 
      printf("find fit 4\n");
      if (succ(current) != freelist_end) {
        current = (char *) GET((char *)succ(current)) + WSIZE;
      } else {
	printf("reach end of free list in find_fit()\n");
	break;
      }
      printf("find fit 5\n");
   }
   fflush(stdout);
   printf("find fit return NULL\n");
   fflush(stdout);
   // else when free list finds no appropriate block, return NULL  
   return NULL;
  
   /*void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
	if (!GET_ALLOC(HDRP(bp)) && (size <= GET_SIZE(HDRP(bp)))) {
	    return bp;
	}
    }
    printf("return NULL\n");
    return NULL;*/
 }
 
/*
 * place sets the header and footer of bp
 * bp points to the start of the payload in allocated list
 * size is the aligned no. of bytes needed for header, footer and payload in allocated list
 */
void place(void *bp, size_t size) {
  char* splitAddress; /* address of the start of the split block */
  size_t sizeDiff;
  
printf("enter place\n");  
  
  if (size <= 0)
    return;
  
  if (bp == NULL || !inbounds(bp))
    return;
    
   // check size is aligned;
   if (size != ALIGN(size)) {
     printf("in place(), size %d is not aligned!\n", size);
     printblock(bp);
     size = ALIGN(size);
   }
  
  if (GET_SIZE(HDRP(bp)) < size) {
     printf("in place(), size should have been determined by find_fit()\n", size);
     printblock(bp);
     return; //block is no longer large enough to accomodate for the size
  }
  
  if (GET_SIZE(HDRP(bp)) == size) {
    printf("in place(), perfect fit with size %d\n", size);
    remove_free(bp);			// remove block from free list
    printf("perfect fit after remove free\n");
    printblock(bp);
    PUT(HDRP(bp), PACK(size,1));  	// set header of bp
    PUT(FTRP(bp), PACK(size,1)); 	// set footer of bp
    return;
  }

  // if free block has remaining space, split the block. keep first block in free list and update its header and footer (no change to succ and prev). 
  //set header, footer of the remaining block. set bp to remaining block
  if (GET_SIZE(HDRP(bp)) > size) {
    printf("Should not enter here!!! find_fit should give perfect fitting block\n");
   //printf("in place(), split block bp %x with size %d\n", bp, size);
    sizeDiff = GET_SIZE(HDRP(bp)) - size;
    PUT(HDRP(bp), PACK(sizeDiff,0));  	/* update header of block that remains in free list*/
    PUT(FTRP(bp), PACK(sizeDiff,0));  	/* update header of block that remains in free list*/
    
    // update remaining block that will become allocated
    //char * allocAddress;
    //allocAddress = (bp + sizeDiff); /* allocAddress points to 1 word after the header */
    //PUT(HDRP(allocAddress), PACK(size,1));
    //PUT(FTRP(allocAddress), PACK(size,1));
    //bp = allocAddress;
    
/* allocAddress points to 1 word after the header */
    /*bp += sizeDiff; 
    PUT(HDRP(bp), PACK(size,1));
    PUT(FTRP(bp), PACK(size,1));
    printf("new bp address is %x and sizeDiff is %d\n", bp, sizeDiff);
    */
    return;
  }
  
  return;
}

void printblock(void *bp){
  size_t headerSize, headerAlloc, footerSize, footerAlloc;
  
  headerSize = GET_SIZE(HDRP(bp));
  headerAlloc = GET_ALLOC(HDRP(bp));
  footerSize = GET_SIZE(FTRP(bp));
  footerAlloc = GET_ALLOC(FTRP(bp));
 
  if (footerSize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }
  
  printf("----printblock\nat address %p: header[size|alloc]: [%d:%d] footer: [%d:%d]\n",
	 bp, (int) headerSize, headerAlloc, (int) footerSize,footerAlloc);
  printf("bp header addres is %x and bp footer address is %x and bp address is %x\n", HDRP(bp),FTRP(bp),bp);  
  unsigned int difference = FTRP(bp) + (~((unsigned int) bp)+1); 
  printf("size without header and footer is %d\n----end printblock\n", difference);
  
  return;
 }

 void checkblock(void *bp){
    return;
 }
