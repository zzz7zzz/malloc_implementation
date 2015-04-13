/* 
 * mm_malloc - Allocate a block.
 *     Always allocate a block whose size is a multiple of the alignment.
 *     size refers to payload. 
 *     returns a pointer to the payload
 */
void *mm_malloc(size_t size) {
  count++;
  PRINT("\n\n\n\ncount: %d\n", count);
  
    PRINT("malloc calls for size %d\n", size);
    size_t newsize; /* adjusted block size that includes header and footer*/
    size_t increasesize;
    void *bp;
    
    if(size <= 0){
        return NULL;
    }
    
    newsize = ALIGN(size + 2*WSIZE + NODE_SIZE); /* include header and footer */
    
    PRINT("in malloc before searching for a fit\n");
    /* Search a fit in the free list and place it in the block*/
    if ((bp = find_fit(newsize)) != NULL) {
      PRINT("find it return this:\n");
      printblock(bp);
      PRINT("in malloc called find fit before calling place\n");
      place(bp, newsize);
      PRINT("after place, bp is:\n");
      printblock(bp);
      PRINT("return from malloc after finding fit\n");
      PRINT("check free list start - does it contain split stuff?--------------------------------\n");
      printblock(start);
      return bp;                                               
    }
    PRINT("no heap found extend size\n");
    /* No fit found. Extend the heap size */
    increasesize = MAX(newsize, CHUNKSIZE);                  
    if ((bp = mem_sbrk(increasesize)) == -1)         
      return NULL;                     
    PRINT("extend is %x\n", bp); 
    PUT(FTRP(bp)+WSIZE, PACK(WSIZE,1)); // epilogue
    add_free(bp, increasesize);
    
     if ((bp = find_fit(newsize)) != NULL) {
      PRINT("find it return this:\n");
      printblock(bp);
      PRINT("in malloc called find fit before calling place\n");
      place(bp, newsize);
      PRINT("after place, bp is:\n");
      printblock(bp);
      PRINT("return from malloc after finding fit\n");
      PRINT("check free list start - does it contain split stuff?--------------------------------\n");
      printblock(start);
      PRINT("exit 2nd findfit\n");
      return bp;                                               
    } else {
     PRINT("this is wrong\n"); 
     abort();
    }
   
    
   // void * newbp = coalesce(bp);

    

    /*allocate the needed space from the new chunk*/
    //place(newbp, newsize);
    PRINT("return from malloc after asking for more space\n");
    // check free list - does it contain split stuff?
    PRINT("check free list - does it contain split stuff?--------------------------------\n");
    printblock(start);
    return bp;
    
}

/*
 * remove_free removes a block from the free list by linking the prev and next pointers correctly in the freelist
 */
void inline remove_free(void *bp){
  if (!bp) {
   PRINT("Error in remove_free\n");
   abort();
  }
  
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size,1));
  PUT(FTRP(bp), PACK(size,1));

  if (GET_NODE(bp)->prev == NULL && GET_NODE(bp)->next == NULL) { //if there is only a start node, return the start node
     start = NULL;
    
  }
    else if (GET_NODE(bp)->prev == NULL && GET_NODE(bp)->next != NULL){ // if bp is the start node and it has next nodes 
    start = start->next;
    GET_NODE(start)->prev = NULL;
  
  
    /*
    start= start->next;
    if(start) {
      GET_NODE(start)->prev = NULL;
    } else {
     PRINT("nothing in free list why?\n");
     abort();
      
    }*/
  }
  else if(GET_NODE(bp)->next == NULL){//last node

    (GET_NODE(bp)->prev)->next = NULL;
  }
  else{ //middle case

    (GET_NODE(bp)->prev)->next = GET_NODE(bp)->next;
    (GET_NODE(bp)->next)->prev = GET_NODE(bp)->prev;
  
  }
}


