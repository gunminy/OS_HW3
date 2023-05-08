#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include "bmalloc.h" 

bm_option bm_mode = BestFit ;
bm_header bm_list_head = {0, 0, 0x0 } ;

void * sibling (void * h)
{	
	// TODO
	bm_header_ptr temp = (bm_header_ptr)h;
	return temp->next;
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
	bm_header_ptr selectedHeader;
	bm_header_ptr itr,prev_itr;
	void* addressPtr;

	//Check if a fitting block exists 
	//And search a smallest block among all unused blocks
	int pageNeeded = 1;
	int fitSize = fitting(s);
	int existMinSize = 13; //max is 12
	for (itr = bm_list_head.next ; itr != 0x0 ; itr = itr->next) {
		prev_itr = itr;// for search last block
		if (fitSize <= itr->size && itr->used==0) {
			if (itr->size < existMinSize) {
				existMinSize = itr->size; 
				selectedHeader = itr;
			}
			pageNeeded = 0;
			if (bm_mode == FirstFit) {
				break;
			}
		}
	}
	
	// If fitting block doesn't exist, get 4KB page.
	if (pageNeeded == 1) {
		addressPtr = (void *)mmap(NULL, 4096,
									PROT_READ | PROT_WRITE,
									MAP_ANONYMOUS | MAP_SHARED, -1, 0
									);
		if (addressPtr== MAP_FAILED) {
			return 0x0;
		}
		selectedHeader = (bm_header_ptr)addressPtr;
		selectedHeader->used = 0;
		selectedHeader->size = 12;
		selectedHeader->next = 0x0;
		if (bm_list_head.next != 0x0) {
			prev_itr->next = selectedHeader;
		}
		else {
			bm_list_head.next = selectedHeader;
		}
	}

	//divides selected block to get a fitting block.
	bm_header_ptr temp;
	while (selectedHeader->size > fitSize) {
		selectedHeader->size = selectedHeader->size - 1;
		int blockBytes = 1 << selectedHeader->size; // 2^(N-1)
		temp = (bm_header_ptr)((char*)selectedHeader + blockBytes);
		temp->used = 0;
		temp->size = selectedHeader->size;
		temp->next = selectedHeader->next;
		selectedHeader->next = temp;
	}

	selectedHeader->used = 1;
	return (void*)((char*)selectedHeader+9); //bm_header is 9 bytes. 
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

