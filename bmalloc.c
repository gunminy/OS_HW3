#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include "bmalloc.h" 

bm_option bm_mode = BestFit ;
bm_header bm_list_head = {0, 0, 0x0 } ;

void * sibling (void * h)
{
	// TODO
}

int fitting (size_t s) 
{
	// TODO
	// header is 9 bytes
	int size;
	if(2039 < s) {
		size = 12 ;
	}
	else if(1015 < s) {
		size = 11 ;
	}
	else if (503 < s) {
		size = 10 ;
	}
	else if (247 < s) {
		size = 9;
	}
	else if (119 < s) {
		size = 8;
	}
	else if (55 < s) {
		size = 7;
	}
	else if (23 < s) {
		size = 6;
	}
	else if (7 < s) {
		size = 5;
	}
	else {
		size = 4;
	}
	return size;
}

void * bmalloc (size_t s) 
{
	// TODO
	bm_header_ptr header;

	header = (bm_header_ptr)mmap(NULL, 4096, 
								PROT_READ | PROT_WRITE,
								MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if (header == MAP_FAILED) {
		return 0x0;
	}
	header->size = 12;
	header->used = 0;
	header->next = 0x0;
	
	bm_list_head.next = header;
	

	return (void*)(header+1); 
}

void bfree (void * p) 
{
	// TODO 
}

void * brealloc (void * p, size_t s) 
{
	// TODO
	return 0x0 ; // erase this 
}

void bmconfig (bm_option opt) 
{
	// TODO
}


void 
bmprint () 
{
	bm_header_ptr itr ;
	int i ;

	printf("==================== bm_list ====================\n") ;
	for (itr = bm_list_head.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%1d %8d:", i, ((void *) itr) + sizeof(bm_header), (int)itr->used, (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(bm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("=================================================\n") ;

	//TODO: print out the stat's.
}

