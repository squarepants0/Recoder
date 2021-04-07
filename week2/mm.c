#include <stddef.h>
#include <linux/types.h>

extern char *mem_sbrk(int incr);
int mm_init(void);
static void *extend_heap(size_t words);
void mm_free(void *bp);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
extern void mem_init(void);
/*Basic constants and macros*/
#define WSIZE		4	/*Basic section of one chunk*/
#define DSIZE		8
#define CHUNKSIZE	(1<<12) /*Extend heap by this amount(bytes)*/

#define MAX(x, y)	((x) > (y))? (x) : (y) 

/*pack a size and allocated bit into a word*/
#define PACK(size, alloc) 	((size) | (alloc))

/*get/put an int value from/to address p*/
#define GET(p)			(*(unsigned int *)(p))
#define PUT(p, val)		(*(unsigned int *)(p) = (val))

/*get size/allocated_bit from address p*/
#define GET_SIZE(p)		((GET(p)) & ~0X7)
#define GET_ALLOC(p)	((GET(p)) & 0X1)

/*translate user_area ptr to header_ptr/footer_ptr*/
#define HDRP(bp)		((char *)(bp) - WSIZE)
#define FTRP(bp)		((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/*get next/previous user_area ptr*/
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/*exchange between BP and HDRP*/
#define BP2HDRP(bp) 	((char *)(bp) - WSIZE)
#define HDRP2BP(hdrp)	((char *)(hdrp) + WSIZE)

/*struct of the heaplist*//*private global variables*/
/*
 ----------------------------------------------------------
|padd|8/1|8/1|---Nomall block---|  ......	| ----0/1----- |
|ings|	 |	 |hdr|			|ftr|  ......	| ending block |	
 ----------------------------------------------------------
*/

/*it always point to the prologue block*/
static char *heap_listp;

int mm_init(void){
	/*get the Prologue part`s area*/
	if((heap_listp=mem_sbrk(4*WSIZE)) == (void *)-1)
		return -1;
	/*initialize the part*/
	PUT(heap_listp, 0);							/*Padding part*/
	PUT(heap_listp + WSIZE, PACK(DSIZE, 1));	/*Prologue header*/
	PUT(heap_listp + WSIZE*2, PACK(DSIZE, 1)); 	/*Prologue footer*/
	PUT(heap_listp + WSIZE*3, PACK(0, 1));		/*Epilogue header*/
	heap_listp += 2*WSIZE;

	/*extend the heap by CHUNKSIZE*/
	if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
		return -1;
	return 0;
}

static void *extend_heap(size_t words){
	char *bp;
	size_t size;

	/*maintain size alignment and allocate memery*/
	size = (words%2)? (words+1)*WSIZE : words*WSIZE;
	if((long)(bp=mem_sbrk(size)) == -1)
		return NULL;

	/*initialize the first heap area*/
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

	/*coalesce every time when heap got expand*/
	return coalesce(bp);

}

/*set the allocated field to 0 and coalesce*/
void mm_free(void *bp){
	size_t size = GET_SIZE(HDRP(bp));

	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	coalesce(bp);
}

static void *coalesce(void *bp){
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	if(prev_alloc && next_alloc)	/*if both inuse*/
		return bp;

	else if(prev_alloc && !next_alloc){   /*coalesce with the next*/
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}

	else if(!prev_alloc && next_alloc){	 /*coalesce with the previous*/
		size += GET_SIZE(FTRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}

	else{
		size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);

	}

	return bp;

}

void *mm_malloc(size_t size){
	size_t asize;		/*size after adjusted*/
	size_t extendsize;	/*amount to extend heap if no fit*/
	char *bp;

	if(heap_listp==NULL){
		mem_init();
		mm_init();
	}

	if(size == 0)
		return NULL;

	if(size <= DSIZE)
		asize = 2*DSIZE;
	else
		asize = DSIZE*((size + (DSIZE) + (DSIZE-1)) / DSIZE);

	/*find the free_fit block*/
	if((bp=find_fit(asize)) != NULL){
		return bp;
	}

	extendsize = MAX(CHUNKSIZE, asize);
	if((bp=extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;
	return bp;

}

/*from heap_listp to the end find the fit block*/
static void *find_fit(size_t size){
	char *smallfit, *current;
	size_t mim_size;
	size_t tmp_size;

	current = heap_listp+DSIZE;
	mim_size = 0x10000;
	smallfit = NULL;
	/*traverse the heap_list find the bestfit then retrun else find the smallfit  */
	while((tmp_size=GET_SIZE(BP2HDRP(current))) != 0){
		if(tmp_size>=size && !GET_ALLOC(BP2HDRP(current))){  //if satisfy the block
			if(tmp_size==size){
				PUT(HDRP(current), PACK(tmp_size, 1));
				PUT(FTRP(current), PACK(tmp_size, 1));
				return current; 			//best fit
			}
			else if(GET_SIZE(BP2HDRP(current)) <= mim_size){
				mim_size = GET_SIZE(BP2HDRP(current));
				smallfit = current;
			}
			
			current = NEXT_BLKP(current);

		}else
			current = NEXT_BLKP(current);
	}
	
	/*split smallfit block or not*/
	if(smallfit != NULL){
		if((tmp_size = (GET_SIZE(BP2HDRP(smallfit))-size)) >= DSIZE){
			PUT(HDRP(smallfit), PACK(size, 1));
			PUT(FTRP(smallfit), PACK(size, 1));
			PUT(HDRP(smallfit)+size, PACK(tmp_size, 0));
			PUT(FTRP(smallfit+size), PACK(tmp_size, 0));

			return smallfit;
		}

		else{
			PUT(HDRP(smallfit), PACK(GET_SIZE(BP2HDRP(smallfit)), 1));
			PUT(FTRP(smallfit), PACK(GET_SIZE(BP2HDRP(smallfit)), 1));
			return smallfit;
		}
	}else

		return smallfit;
}
