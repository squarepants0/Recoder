#include <stddef.h>
#include <linux/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MIMCHUNK 	0x20
#define WSIZE 		8
#define DSIZE 		0x10

extern int errno;
extern char *mem_init(void);
extern char *mem_sbrk(int incr);

static int mm_malloc_init(void);
void *mm_malloc(size_t rsize);


/*
	chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Size of previous chunk                            |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	`head:' |             Size of chunk, in bytes                         |P|
	  mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Forward pointer to next chunk in list             |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Back pointer to previous chunk in list            |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Unused space (may be 0 bytes long)                .
		    .                                                               .
		    .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	`foot:' |             Size of chunk, in bytes                           |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct chunk{ 	/*basic elem*/
	size_t prev_size;
	size_t size;
	struct chunk *fd;
	struct chunk *bk;
};

/*some define about size section*/
#define CHUNKINUSE		1
#define CHUNKFREED		0

#define Request2size(req)							\
	(req) <= DSIZE ? MIMCHUNK :						\
	DSIZE*(((req) + (DSIZE) + (DSIZE-1)) / DSIZE)	

#define Docheck_request2size(req, sz_addr)		\
	if((size_t)(req) > 0x100000){	\
		errno = ENOMEM;			\
		return 0;				\
	}							\
	*(sz_addr) = Request2size(req);	

/*stuffs about chunk cp---chunkptr  up---userptr*/ 
typedef struct chunk *chunkptr;
#define Pack(size, state)	((size) | (state))
#define Get(p)				(*(unsigned long *)(p))
#define Put(p, val)			(*(unsigned long *)(p) = (val))
#define Mem2chunk(up)		((unsigned long *)(bp)-DSIZE)
#define Chunk2mem(cp)		(void *)((unsigned long)(cp)+DSIZE)
#define Getcpsize(cp)		(cp->size & ~0x7)
#define Getcpstate(cp)		(cp->size & 0x1)

/*heap control struct*/
typedef struct chunk *fchunkptr;
#define MAX_FAST_SIZE 	0x70
#define NFASTBINS		6

typedef struct chunk *schunkptr;
#define MIM_SMALL_SIZE	0x80
#define NSMALLBINS 		0x40
typedef struct chunk *binptr;
typedef struct chunk *unsortedbinptr;
struct mm_malloc_state{
	fchunkptr fastbinsY[NFASTBINS];
	schunkptr smallbins[NSMALLBINS];
	unsortedbinptr ub[2];
	chunkptr top;
}mstate;

/*stuffs about fastbin*/
#define Fastbin_idx(sz)	\
	(((sz) >> 4) - 2)

#define Getfastbinlist(idx)	\
	(&(mstate.fastbinsY[idx]))

/*stuffs about smallbin*/
#define Smallbin_idx(sz)	\
	(((sz) >> 4) - 8)

#define Getsmallbinlist(idx)	\
	((unsigned long)&(mstate.smallbins[idx*2]) - DSIZE)

/*stuffs about ub*/
#define Getublist()	\
	((unsigned long)&(mstate.ub) - DSIZE)

/*stuffs about top*/

static int mm_malloc_init(void){
	mstate.top = (chunkptr)mem_init();  /*get top chunk and init heap area*/

	for(int i=0; i<NFASTBINS ; i++){
		mstate.fastbinsY[i] = NULL;
	}

	for(int i=0; i<NSMALLBINS ;i+=2){
		mstate.smallbins[i] = mstate.smallbins[i+1] = (chunkptr)((unsigned long)&(mstate.smallbins[i]) - DSIZE);
	}

	mstate.ub[0] = mstate.ub[1] = (chunkptr)((unsigned long)&(mstate.ub) - DSIZE);
	mstate.top->prev_size = 0;
	mstate.top->size = 0x10000;
	mstate.top->fd = NULL;
	mstate.top->bk = NULL;
}

void *mm_malloc(size_t rsize){
	size_t nb;				/*normalized request size */
	unsigned int idx;		/*associated bin index*/

	binptr bin;				/*associated bin */
	chunkptr victim;		/* inspected/selected chunk */
	size_t size; 			/* its size */
	int victim_index;		/* its bin index */

	chunkptr fwd;			/*misc tmp for linking*/
	chunkptr bck;			/*misc tmp for linking*/


	Docheck_request2size(rsize, &nb); /*check and normalize rsize*/

	/*if init the heap??*/
	if(mstate.top == NULL)
		mm_malloc_init();

	/*find best fit from fastbins do not split*/
	if(nb <= MAX_FAST_SIZE){
		idx = Fastbin_idx(nb);
		chunkptr *fd = Getfastbinlist(idx);
		if((victim = *fd) != NULL){
			*fd = victim->fd;
			victim->size = Getcpsize(victim) | CHUNKINUSE;

			return Chunk2mem(victim);
		}
	}

	/*find chunk from smallbins and chunk do not split*/
	else if(nb <= 0x280){
		idx = Smallbin_idx(nb);
		schunkptr head = (chunkptr)Getsmallbinlist(idx);
		if((bck=head->bk) != head){
			//unlink
			head->bk = bck->bk;
			bck->bk->fd = head;

			bck->size = Getcpsize(bck) | CHUNKINUSE;
			return Chunk2mem(bck);
		}
	}
	else{
		/*if there is chunk in UB then use it if it`s size is big enough*/
		unsortedbinptr head = (chunkptr)(Getublist());
		if((bck=head->bk) != head){
			for(fwd=head->fd; bck->size>=nb ;bck=bck->bk){
				if(bck == head)
					break;
				//unlink
				bck->bk->fd = bck->fd;
				bck->fd->bk = bck->bk;

				if(bck->size==nb || (bck->size-nb)<MIMCHUNK)
					return Chunk2mem(bck);
				else{
					/*if split a big chunk, then put the left chunk into UB and insert to first one*/
					size = Getcpsize(bck);
					bck->size = nb | CHUNKINUSE;
					unsortedbinptr ptr = (unsortedbinptr)((unsigned long)bck+nb);
					ptr->prev_size = 0;
					ptr->size = (size-nb) | CHUNKFREED;

					ptr->bk = fwd->bk;
					ptr->fd = fwd;
					fwd->bk->fd = ptr;
					fwd->bk = ptr;

					return Chunk2mem(bck);
				}
			}
		}
	}

	/*split the top chunk or get heap from sbrk and mmap*/
	victim = mstate.top;
	size = Getcpsize(mstate.top);
	if(nb < size){
		if(size-nb >= MIMCHUNK){
			mstate.top = (chunkptr)((unsigned long)victim + nb);
			mstate.top->size = (size-nb) | CHUNKFREED;

			victim->size = nb | CHUNKINUSE;
			return Chunk2mem(victim);
		}
	}
	/*if nb is not that big so I gonna put the big chunk into UB the last one*/
	else if(nb <= 0x10000){
		unsortedbinptr head = (chunkptr)Getublist();
		fwd = head->fd;
		bck = head->bk;
		victim->fd = head;
		victim->bk = bck;
		head->bk = victim;
		bck->fd = victim;

		victim = mstate.top = (chunkptr)mem_sbrk(0x10000);
		if(victim < 0){
			return (void *)-1;
		}
		mstate.top = (chunkptr)((unsigned long)(mstate.top) + nb);
		mstate.top->size = (0x10000 - nb) | CHUNKFREED;
		victim->size = nb | CHUNKINUSE;

		return Chunk2mem(victim);
	}



}