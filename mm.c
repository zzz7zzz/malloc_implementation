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
#define END 0

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

/*Compute the address of the next block or previous block in the heap in free list. Returns bp of block*/ 
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

/*pointers to the next and previous free blocks. Returns bp*/
#define succ(bp) GET(bp)
#define pred(bp) GET(bp + WSIZE)

/*rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT) -1) & ~(ALIGNMENT- 1))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*global variables*/
static char *heap_listp; /*pointer to the first block*/
static char *freelist_start; /* points to bp */
static char *freelist_end; /* points to start of heap space*/
static int count;

/*Function prototypes for helper routines*/
static void *extend_heap(size_t words);
static void place(void *bp, size_t size);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp, char*msg);
static void checkblock(void *bp);
static void add_free(void *bp);
static void remove_free(void *bp);
static int mm_check(void);
static int inbounds(void *ptr);
static void checkfreelist();

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    void * heap;
  count = 0;
    /*create double linked free list*/
    if((freelist_start = mem_sbrk(4*WSIZE)) == (void*)-1){
        return -1;
    }
    
   /* if((freelist_end = mem_sbrk(4*WSIZE)) == (void*)-1){
        return -1;
    }*/
    
    /*Set freelist_end and freelist_start to point at bp of block*/
    freelist_end = freelist_start;
    freelist_end += WSIZE;
    PUT(HDRP(freelist_end), PACK(0, 1)); // epilogue
    
    freelist_start += DSIZE;
    
    /*Initialize freelist_start block */
    PUT(freelist_start - WSIZE,  PACK(DSIZE,0)); 
   
    PUT(freelist_start, (char *) freelist_end);    // set succ 
   // PUT(freelist_start + (1* WSIZE), END);   
  
    
    //printf("free list succ is %p and pack is %x\n", GET(freelist_start), PACK(2*WSIZE,0));
    //printblock(freelist_start,"freelist_start");
    //printblock(freelist_end,"freelist_end");
    
   
    
    /* create heap and set header and footer of heap*/
    heap = extend_heap(CHUNKSIZE/WSIZE);
    
    /* add heap to free list */
    add_free(heap);
    
////printblock(heap, "heap");
//abort();
    return 0;
}


/* 
 * mm_malloc - Allocate a block.
 *     Always allocate a block whose size is a multiple of the alignment.
 *     size refers to payload. 
 *     returns a pointer to the payload
 */
void *mm_malloc(size_t size) {
  count++;
  if (count == 3) {

    //checkfreelist();
    //abort();
    
  }
    size_t newsize; /* adjusted block size that includes header and footer*/
    size_t increasesize;
    char *bp;
    printf("\n\n\n\nenter mm_malloc whee***********************************\n");
    
    // put this in remove_free
    if (succ(freelist_start) == freelist_end){
    printf("adding heap in mm_malloc\n");
      /* create heap and set header and footer of heap*/
    void * heap; 
    heap = (void *) extend_heap(CHUNKSIZE/WSIZE);
    
    /* add heap to free list */
    add_free(heap);
    }
    
    if(size <= 0){
        return NULL;
    }
    
    newsize = ALIGN(size + DSIZE); /* include header and footer */
    
    
    /* Search a fit in the free list and place it in the block*/
    /* bp points to payload */
    if ((bp = find_fit(newsize)) != NULL) {                     //if we can fit it anywhere in the free list
      ////printblock(bp);
     
      //printblock(bp,"In mm_malloc, find fit returns this bp");
      place(bp, newsize); //will call free list stuff           //call place function to add it
      printf("In mm_malloc, place Returns this pointer**************************:\n\n\n\n\n");
      //printblock(bp,"return");
     //checkfreelist();
      //abort();
      return bp;                                                //return the block pointer to the newly allocated block
    }

    /* No fit found. Extend the heap size */
    printf("no fit found in mm_malloc after while loop 1\n");
    increasesize = MAX(newsize, CHUNKSIZE);                     //and if the needed space is larger than our normal chunksize
     void * heap; 
    heap = (void *) extend_heap(increasesize/WSIZE);
    
    /* add heap to free list */
    add_free(heap);                                             //error catching
    
    // set extra space to front of free list 
    
    /*allocate the needed space from the new chunk*/
    if ((bp = find_fit(newsize)) != NULL) {                     //if we can fit it anywhere in the free list
      ////printblock(bp);
     
      //printblock(bp,"In mm_malloc, find fit returns this bp");
      place(bp, newsize); //will call free list stuff           //call place function to add it
      printf("In mm_malloc, place Returns this pointer**************************:\n\n\n\n\n");
      //printblock(bp,"return");
      
      //abort();
      return bp;                                                //return the block pointer to the newly allocated block
    }
    printf("no fit found in mm_malloc after while loop 2\n");

    return NULL;
    
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
    return;
  
  PUT((char *)HDRP(ptr), PACK(size, 0)); 
  PUT((char *)FTRP(ptr), PACK(size, 0)); 
  add_free(ptr);
  coalesce(ptr);
  
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
 * block's header and footer should be set
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
    
    size_t size;
    
    if(!bp){
        return;
    }
    
    if((size = GET_SIZE(HDRP(bp))) == 0) {
     printf("bp has no size in add_free\n"); 
     return;
    }
    
    if (size != ALIGN(size)) {
     printf("Not aligned in add free\n");
     return;
    }
    
   
    
      char * temp;
      temp = succ(freelist_start);
      
      // if free list is empty
      if (temp == freelist_end) {
	PUT(freelist_start, bp); 
	PUT(bp, temp); // succ of bp points to temp
	 PUT(bp + WSIZE, freelist_start); // prev of bp points to freelist_start
	 //printf("entered here\n");
	 //printblock(bp,"add free");
	 //printblock(freelist_start, "freelist_start");
	
	return;
      }
      
      PUT(freelist_start, bp); 
      PUT(bp, temp); // succ of bp points to temp
      PUT(bp + WSIZE, freelist_start); // prev of bp points to freelist_start
      PUT(temp + WSIZE, bp);  // prev of temp points to bp
      
     // //printblock(bp, "bp in add free");
      ////printblock((succ(bp)), "succ of bp");
      
      return;
    
}

/*
 * remove_free removes a block from the free list by updating succ and pred pointers, and freelist_start and freelist_end. 
 * Not in charge of setting header and footer.
 */
void remove_free(void *bp){
    printf("enter remove free\n");
  if (!bp)
      return;
  
    // if nothing in free list
    if (succ(freelist_start) == freelist_end) {
      /* create heap and set header and footer of heap*/
    void * heap;
      heap = extend_heap(CHUNKSIZE/WSIZE);
    
    /* add heap to free list */
    add_free(heap);
    }
  //printblock(bp, "in remove_free");
  
   
  
  printf("whee! what fun\n");
    void * prev = pred(bp);
    printf("whee! what fun222\n");
   void * next = succ(bp);
   
   
   if (prev == freelist_start && next == freelist_end) {
    PUT(freelist_start, next); 
    return;
   }
   

    
   PUT(prev, next);
  PUT(next + WSIZE, prev);
    
    
    
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
    
    /* create heap */
    if((bp = mem_sbrk(size)) == (void*)-1){
        return -1;
    }
    
    /*Initialize heap*/
    PUT(HDRP(bp),  PACK((size),0));                            
    //PUT(heap, freelist_end);    
    //PUT(heap + (1* WSIZE), freelist_start);   
    PUT(FTRP(bp), PACK((size),0));
   
    return bp;
    //1024
}

/*
 * coalesce - boundary tag coalescing, return ptr to coalesced block
 */
 static void *coalesce(void *bp)
 {
    
   
   
   //check for memory in bounds here
    //if you coalesce you need to remove the block from the free list and reset its pointers
    //then create header and footer of block
    size_t prev_alloc = GET_ALLOC(FTRP(NEXT_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
printf("hi\n");
    if(prev_alloc && next_alloc){
      
        return bp;
    }
    else if(prev_alloc && !next_alloc){

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size,0));
        return bp;
    }
    else if(!prev_alloc && next_alloc){
      
        void * prevbp;
        prevbp = PREV_BLKP(bp);
        size += GET_SIZE(HDRP(prevbp));
        PUT(HDRP(prevbp), PACK(size,0));
        PUT(FTRP(prevbp),PACK(size,0));
        return prevbp;
    }
    else{
      void * prevbp;

	prevbp = PREV_BLKP(bp);
	size += GET_SIZE(HDRP(prevbp)) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(prevbp), PACK(size,0));
        PUT(FTRP(prevbp), PACK(size,0));
        return prevbp;
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
   if (succ(freelist_start) == freelist_end) {
     printf("free list is empty\n");
     return NULL;
   }
   // check size is aligned;
   if (size != ALIGN(size)) {
     printf("size %d is not aligned!\n", size);
     size = ALIGN(size);
     return NULL;
   }
   
   current = freelist_start;
   //printblock(succ(current), "next of freelist_start");
   //abort();
   //current = succ(freelist_start);
   //printf("find fit 2\n");
   // if free list finds non-null block with right size, return address of block
   while ((current = succ(current)) != freelist_end) {
     printf("find fit while loop\n");
      if (GET_SIZE(HDRP(current)) == size) {
	printf("in while loop of find_fit()\n");
	remove_free(current);
////printblock(current);
             return current; //return pointer to payload 
      }
      
     // printf("find fit 3\n");
      if (GET_SIZE(HDRP(current)) > size) {
	// split block
	size_t sizeDiff;
	printf("in find_fit(), split block bp %x with size %d\n", current, size);
        sizeDiff = GET_SIZE(HDRP(current)) - size;
	printf("sizeDiff is %d\n", sizeDiff);
        PUT(HDRP(current), PACK(sizeDiff,0));  	/* update header of block that remains in free list*/
        PUT(FTRP(current), PACK(sizeDiff,0));  	/* update footer of block that remains in free list*/
	//printf("in find_fit(), free block is:\n");
        //printblock(current, "in find_fit(), free block is:\n");
	current += (unsigned int) sizeDiff; /* current points to 1 word after the header */
	printf("find fit returns this splitted current %x\n", current);
	 
	 PUT(HDRP(current), PACK(size,0));
         PUT(FTRP(current), PACK(size,0));
	 //printblock(current, "find fit returns");
	 return current;
      } 
      
      
      
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
     ////printblock(bp);
     size = ALIGN(size);
   }
  
  if (GET_SIZE(HDRP(bp)) < size) {
     printf("in place(), size should have been determined by find_fit()\n", size);
    // //printblock(bp);
     return; //block is no longer large enough to accomodate for the size
  }
  
  if (GET_SIZE(HDRP(bp)) == size) {
    printf("in place(), perfect fit with size %d\n", size);
    //remove_free(bp);			// remove block from free list
    printf("perfect fit after remove free\n");
   // //printblock(bp);
    PUT(HDRP(bp), PACK(size,1));  	// set header of bp
    PUT(FTRP(bp), PACK(size,1)); 	// set footer of bp
    return;
  }

  // if free block has remaining space, split the block. keep first block in free list and update its header and footer (no change to succ and prev). 
  //set header, footer of the remaining block. set bp to remaining block
  if (GET_SIZE(HDRP(bp)) > size) {
    printf("Should not enter here!!! find_fit should give perfect fitting block\n");
    abort();
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

void printblock(void *bp, char * msg){
  size_t headerSize, headerAlloc, footerSize, footerAlloc;
  printf("fault2?\n");
  
  if (bp == freelist_end) {
   printf("freelist_end\n");
   return;
  }
  
  headerSize = GET_SIZE(HDRP(bp));
  headerAlloc = GET_ALLOC(HDRP(bp));
  footerSize = GET_SIZE(FTRP(bp));
  footerAlloc = GET_ALLOC(FTRP(bp));
 
  if (headerSize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }
  printf("fault2?\n");
  printf("----//printblock %s\nat address %p: header[size|alloc]: [%d:%d] footer: [%d:%d]\n",
	 msg, bp, (int) headerSize, headerAlloc, (int) footerSize,footerAlloc);
  printf("bp header addres is %x\n bp footer address is %x\n bp address is %x\n", HDRP(bp),FTRP(bp),bp);  
  unsigned int difference = FTRP(bp) + (~((unsigned int) bp)+1); 
  printf("size without header and footer is %d\n", difference);
  
  if(headerAlloc == 0) {
    printf("succ address %p,\npred address: %p\n", succ(bp), pred(bp));
  }
  
  printf("--------------------end //printblock\n");
  return;
 }

 void checkfreelist(){
   int num = 0; 
   printf("checkblock\n");
   //printblock(freelist_start, "freelist-start");
   //printblock(freelist_end, "Freelist-end");
   void * current = freelist_start;
   
   while((current = succ(freelist_start)) != freelist_end) {
     printf("fault?\n");
    //printblock(current, "lah");  
    num++;
    printf(num);
   }
   
    return;
 }
 
  void checkblock(void *bp){


    return;
 }

