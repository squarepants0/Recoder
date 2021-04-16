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
int mm_free(void *p);
static int consolidate();
// static void mm_conslodate(struct mm_malloc_state mstate);


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
#define Mem2chunk(up)		(chunkptr)((unsigned long)(up)-DSIZE)
#define Chunk2mem(cp)		(void *)((unsigned long)(cp)+DSIZE)
#define Getcpsize(cp)		(cp->size & ~0x7)
#define Getcpstate(cp)		(cp->size & 0x1)

#define Getnext(cp)			(chunkptr)((unsigned long)(cp) + Getcpsize(cp))
#define Getprev(cp)			(chunkptr)((unsigned long)(cp) - cp->prev_size)

#define unlink(cp, FD, BK){							\
	FD = cp->fd;									\
	BK = cp->bk;									\
	if((FD->bk != cp) || (BK->fd != cp)){					\
		errno = EADDRNOTAVAIL;						\
		perror("***corrupted double-linked list***");\
		return -1;									\
	}												\
													\
	FD->bk = BK;									\
	BK->fd = FD;									\
													\
}			


/*heap control struct*/
typedef struct chunk *fchunkptr;
#define MAX_FAST_SIZE 	0x70
#define NFASTBINS		6

typedef struct chunk *schunkptr;
#define MAX_SMALL_SIZE	0x400
#define NSMALLBINS 		124
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
	(((sz) >> 4) - 2)

#define Getsmallbinlist(idx)	\
	((unsigned long)&(mstate.smallbins[idx*2]) - DSIZE)

/*stuffs about ub*/
#define Getublist()	\
	(chunkptr)((unsigned long)&(mstate.ub) - DSIZE)

/*stuffs about top*/

static int mm_malloc_init(void){
	mstate.top = (chunkptr)mem_init();  /*get top chunk and init heap area*/

	for(int i=0; i<NFASTBINS ; i++){
		mstate.fastbinsY[i] = NULL;
	}

	for(int i=0; i<(NSMALLBINS) ;i+=2){
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
	chunkptr next;
	size_t size; 			/* its size */
	int victim_index;		/* its bin index */

	chunkptr fwd;			/*misc tmp for linking*/
	chunkptr bck;			/*misc tmp for linking*/


	Docheck_request2size(rsize, &nb); /*check and normalize rsize*/

	/*if init the heap*/
	if(mstate.top == NULL)
		mm_malloc_init();

	/*find best fit from fastbins do not split*/
	if(nb <= MAX_FAST_SIZE){
		idx = Fastbin_idx(nb);
		binptr *fd = (binptr *)Getfastbinlist(idx);
		if((victim = *fd) != NULL){
			*fd = victim->fd;
			victim->size = Pack(Getcpsize(victim), CHUNKINUSE);
			victim = Chunk2mem(victim);
			return victim;
		}
	}
	if(nb <= MAX_SMALL_SIZE){
		idx = Smallbin_idx(nb);
		schunkptr head = (chunkptr)Getsmallbinlist(idx);
		if((bck=head->bk) != head){
			//unlink
			head->bk = bck->bk;
			bck->bk->fd = head;

			bck->size = Getcpsize(bck) | CHUNKINUSE;
			next = Getnext(bck);
			next->prev_size = 0;

			victim = Chunk2mem(victim);
			return victim;
		}
	}
	
	/*if there is chunk in UB then use it if it`s size is big enough*/
	unsortedbinptr head = (chunkptr)(Getublist());
	if((bck=head->bk) != head){
		for(fwd=head->fd; bck->size>=nb ;bck=bck->bk){
			if(bck == head)
				break;
			//unlink
			bck->bk->fd = bck->fd;
			bck->fd->bk = bck->bk;

			/*if lucky nb==size then just return it*/
			if(bck->size==nb || (bck->size-nb)<MIMCHUNK){
				victim = bck;
				victim->size = Getcpsize(victim) | CHUNKINUSE;
				/*set foot*/
				next = Getnext(victim);
				next->prev_size = 0;

				victim = Chunk2mem(victim);
				return victim;
			}
			else{
				/*if split a big chunk, then put the left chunk into UB and insert to last one*/
				size = Getcpsize(bck);

				unsortedbinptr ptr = (unsortedbinptr)((unsigned long)bck+nb);
				ptr->prev_size = 0;
				ptr->size = Pack(size-nb, CHUNKFREED);

				ptr->fd = head;
				ptr->bk = head->bk;;
				head->bk->fd = ptr;
				head->bk = ptr;
				/*set foot*/
				next = Getnext(ptr);
				next->prev_size = ptr->size;

				victim = bck;
				victim->size = Pack(nb, CHUNKINUSE);

				return Chunk2mem(victim);
			}
		}
	}

	consolidate(); /*consolidate fastbin*/

	/*split the top chunk or get heap from sbrk and mmap*/
	victim = mstate.top;
	size = Getcpsize(mstate.top);
	if(nb < size){
		if(size-nb >= MIMCHUNK){
			mstate.top = (chunkptr)((unsigned long)victim + nb);
			mstate.top->size = Pack(size-nb, CHUNKFREED);

			victim->size = Pack(nb, CHUNKINUSE);
			return Chunk2mem(victim);
		}
	}
	/*if nb is not that big so put the big chunk into UB the last one*/
	else if(nb <= 0x10000){
		unsortedbinptr head = (chunkptr)Getublist();
		fwd = head->fd;
		bck = head->bk;
		victim->fd = head;
		victim->bk = bck;
		head->bk = victim;
		bck->fd = victim;

		victim = mstate.top = (chunkptr)mem_sbrk(0x10000);
		if(victim <= 0){
			return (void *)-1;
		}
		mstate.top = (chunkptr)((unsigned long)(mstate.top) + nb);
		mstate.top->size = Pack(0x10000-nb, CHUNKFREED);

		victim->size = Pack(nb, CHUNKINUSE);
		victim->prev_size = size;

		return Chunk2mem(victim);
	}

}

int mm_free(void *p){
	size_t size;		/*target`s size*/
	fchunkptr fbinptr;
	schunkptr sbinptr;
	unsortedbinptr ubinptr;
	int binidx;

	chunkptr next;
	chunkptr prev;
	chunkptr bck;
	chunkptr fwd;
	chunkptr victim;

	victim = Mem2chunk(p);
	size = Getcpsize(victim);
	next = Getnext(victim);

	/*just like ptmalloc my fastbin chunk do not consolidate until the mm_conslodate works*/
	if(size<=MAX_FAST_SIZE){
		binidx = Fastbin_idx(size);
		chunkptr *tmp = Getfastbinlist(binidx);
		fbinptr = *tmp;
		if(victim == fbinptr)
		{
			errno = EADDRNOTAVAIL;
			perror("***fastbin chunk double-freed***");
			return -1;
		}

		victim->fd = fbinptr;
		*tmp = victim;
		/*set state*/
		victim->size = Pack(size, CHUNKFREED);

		return 0;
	}
	/*if prev_chunk`s or next_chunk`s state is FREE then consolidate frist */
	else{
		if(!Getcpstate(victim)){
		errno = EADDRNOTAVAIL;
		perror("***target chunk double-freed***");
		return -1;
		}

		victim->size = Pack(size, CHUNKFREED);
		if(victim->prev_size != 0){
			prev = Getprev(victim);
			size += Getcpsize(prev);
			victim = prev;
			victim->size = Pack(size, CHUNKFREED);
			unlink(victim, fwd, bck);
		}
		
		/*if next isn`t top and not inuse then consolidate*/
		if(next != mstate.top){

			if(!Getcpstate(next)){
				size += Getcpsize(next);
				victim->size = Pack(size, CHUNKFREED);
				unlink(next, fwd, bck); 
			}
			/*put it in UB*/
			ubinptr = Getublist();
			victim->fd = ubinptr;
			victim->bk = ubinptr->bk;
			ubinptr->bk->fd = victim;
			ubinptr->bk = victim;	

			/*set foot*/
			next = Getnext(victim);
			next->prev_size = Getcpsize(victim);

			return 0;
		}
		else{
			size += Getcpsize(mstate.top);
			mstate.top = victim;
			mstate.top->size = size;

			return 0;
		}
		
	}

}

static int consolidate(){
	unsortedbinptr fwd;
	unsortedbinptr bck;
	
	binptr ubptr;
	binptr smptr;
	int idx;

	fchunkptr victim;
	fchunkptr fd;
	chunkptr next;
	chunkptr prev;
	size_t size;

	fchunkptr *tmp=Getfastbinlist(0);
	for(int i=0 ; i<6 ;i++, tmp=Getfastbinlist(i)){
		victim = *tmp;
		
		do{

			if(victim != NULL){
				fd = victim->fd;
				*tmp = fd;
				size = Getcpsize(victim);
				/*consolidate with prev_chunk*/
				if(victim->prev_size != 0){
					prev = Getprev(victim);
					size += Getcpsize(prev);
					victim = prev;
					victim->size = Pack(size, CHUNKFREED);
					unlink(victim, fwd, bck);
				}
				/*consolidate with next_chunk*/
				next=Getnext(victim);
				
				if(next != mstate.top){
					/*if that next is not a free_chunk in fastbin*/
					if(((chunkptr)(Getnext(next)))->prev_size == Getcpsize(next)){
						size += Getcpsize(next);
						victim->size = Pack(size, CHUNKFREED);
						unlink(next, fwd, bck);
					}
					if(size <= MAX_SMALL_SIZE){
						idx = Smallbin_idx(size);
						smptr = (binptr)Getsmallbinlist(idx);

						bck = smptr->bk;
						fwd = smptr->fd;

						victim->fd = smptr;
						victim->bk = bck;
						bck->fd = victim;
						smptr->bk = victim;

						/*set foot*/
						next = Getnext(victim);
						next->prev_size = Getcpsize(victim);
					}
					else{
						ubptr = (binptr)Getublist();

						bck = ubptr->bk;
						fwd = ubptr->fd;

						victim->fd = ubptr;
						victim->bk = bck;
						bck->fd = victim;
						ubptr->bk = victim;

						/*set foot*/
						next = Getnext(victim);
						next->prev_size = Getcpsize(victim);

					}
				}
				else{
					/*if next is top just consolidate~*/
					size += Getcpsize(mstate.top);
					mstate.top = victim;
					mstate.top->size = Pack(size, CHUNKFREED);
				}

				victim = fd;
			}
			else
				break;
		}while(1);
		
	}

}