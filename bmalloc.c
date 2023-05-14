#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include "bmalloc.h" 

bm_option bm_mode = BestFit;
bm_header bm_list_head = { 0, 0, 0x0 };

void* sibling(void* h)
{
	// TODO
	// The address of a block's "buddy" is equal to the bitwise exclusive OR (XOR) 
	// of the block's address and the block's size.
	bm_header_ptr temp = (bm_header_ptr)h;
	long int size = 1 << temp->size;
	return (void*)((long int)temp ^ size);
}

int fitting(size_t s)
{
	// TODO
	// header is 9 bytes
	int size;
	if (2048 - sizeof(bm_header) < s) {
		size = 12;
	}
	else if (1024 - sizeof(bm_header) < s) {
		size = 11;
	}
	else if (512 - sizeof(bm_header) < s) {
		size = 10;
	}
	else if (256 - sizeof(bm_header) < s) {
		size = 9;
	}
	else if (128 - sizeof(bm_header) < s) {
		size = 8;
	}
	else if (64 - sizeof(bm_header) < s) {
		size = 7;
	}
	else if (32 - sizeof(bm_header) < s) {
		size = 6;
	}
	else if (16-sizeof(bm_header) < s) {
		size = 5;
	}
	else {
		size = 4;
	}
	return size;
}

void* bmalloc(size_t s)
{
	// TODO

	if(s==0) {
		return 0x0;
	}

	bm_header_ptr selectedHeader;
	bm_header_ptr itr, prev_itr;
	void* addressPtr;

	//Check if a fitting block exists 
	//And search a smallest block among all unused blocks
	int pageNeeded = 1;
	int fitSize = fitting(s);
	int existMinSize = 13; //max is 12
	for (itr = bm_list_head.next; itr != 0x0; itr = itr->next) {
		prev_itr = itr;// for search last block
		if (fitSize <= itr->size && itr->used == 0) {
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
		addressPtr = (void*)mmap(NULL, 4096,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE, -1, 0
		);
		if (addressPtr == MAP_FAILED) {
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
		temp = (bm_header_ptr)((void*)selectedHeader + blockBytes);
		temp->used = 0;
		temp->size = selectedHeader->size;
		temp->next = selectedHeader->next;
		selectedHeader->next = temp;
	}

	selectedHeader->used = 1;
	return ((void*)selectedHeader) + sizeof(bm_header); //bm_header is 9 bytes. 
}

void bfree(void* p)
{
	// TODO
	bm_header_ptr header, temp;
	header = (bm_header_ptr)(p - sizeof(bm_header));
	header->used = 0;

	//If sibling block is unused, Merge this block with sibling
	bm_header_ptr sib = (bm_header_ptr)sibling(header);
	while (header->size < 12 && sib->used == 0) {
		if (sib->next == header) {
			//changes position of header and sib
			temp = sib;
			sib = header;
			header = temp;
		}
		header->next = sib->next;
		header->size++;
		sib = (bm_header_ptr)sibling(header);
	}

	//unmaps (releases) a page if whole page becomes unused.
	int wholePageBecomesUnused = 1;
	bm_header_ptr itr = bm_list_head.next;
	while (itr != 0x0) {
		if (itr->used == 1) {
			wholePageBecomesUnused = 0;
			break;
		}
		itr = itr->next;
	}
	if (wholePageBecomesUnused) {
		itr = bm_list_head.next;
		bm_list_head.next = 0x0;
		while (itr != 0x0) {
			temp = itr;
			itr = itr->next;
			if ((munmap(temp, 4096)) == -1) { //releases
				fprintf(stderr, "munmap failed with error:");
			}
		}
	}
	
}

void* brealloc(void* p, size_t s)
{
	// TODO
	return 0x0; // erase this 
}

void bmconfig(bm_option opt)
{
	// TODO
	bm_mode = opt;
}


void
bmprint()
{
	bm_header_ptr itr;
	int i;

	printf("==================== bm_list ====================\n");
	for (itr = bm_list_head.next, i = 0; itr != 0x0; itr = itr->next, i++) {
		printf("%3d:%p:%1d %8d:", i, ((void*)itr) + sizeof(bm_header), (int)itr->used, (int)itr->size);

		int j;
		char* s = ((char*)itr) + sizeof(bm_header);
		for (j = 0; j < (itr->size >= 8 ? 8 : itr->size); j++)
			printf("%02x ", s[j]);
		printf("\n");
	}
	printf("=================================================\n");

	//TODO: print out the stat's.
	unsigned int givenMem = 0;
	unsigned int givenToUser = 0;
	unsigned int amountHeader = 0;
	unsigned int availableMem = 0;
	for (itr = bm_list_head.next; itr != 0x0; itr = itr->next) {
		givenMem += 1 << itr->size;
		if (itr->used == 1) {
			givenToUser += 1 << itr->size;
			amountHeader += sizeof(bm_header);
		}
		else {
			availableMem += 1 << itr->size;
		}
	}
	printf("The total amount of all given memory: %u bytes\n", givenMem);
	printf("The total amount of used blocks: %u bytes, ", givenToUser);
	printf("Excluding header size: %u bytes\n", givenToUser-amountHeader);
	printf("The total amount of available memory: %u bytes\n", availableMem);
}

