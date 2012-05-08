#include "mem_alloc.h"
#include "mem_struct.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ADMIN_SIZE (long) ceil((double) sizeof(block_t)/(double) sizeof(long))
#define FIRST_BLOCK (long) ceil((double) sizeof(admin_t)/(double) sizeof(long))

block_t *new_block(long index, long leng, long next, long prev);

void print_blocks();

long *mem;
admin_t *admin;
block_t *blocks;

void mem_init(long memory[MEM_SIZE]){	
	block_t *block;
	
	mem = memory;
	admin = (admin_t*) mem;
	blocks = (block_t*) mem; 
	
	admin->toegewezen = 0;
	admin->loos = ADMIN_SIZE;
	
	block = (block_t*) &mem[FIRST_BLOCK];
	block->free = 1;
	block->length = MEM_SIZE - admin->loos - FIRST_BLOCK;
	block->next = 0;
	block->prev = 0;
	
	print_blocks();
}

int split_block(long index, long length){
	block_t *block = (block_t*) &mem[index];
	printf("Enter split %ld %ld\n", index, length);

	if(block->free && block->length > length + ADMIN_SIZE + 1){
		long newidx = index + ADMIN_SIZE + length;
		long newleng =  block->length - ADMIN_SIZE - length;
		block_t *newblock = new_block(newidx, newleng, block->next, index);
		
		/* Zet de link naar het nieuwe blok. */
		block->next = newidx;
		if(newblock->next != 0){
			/* Als er een volgend blok bestaat, zet zijn prev dan goed. */
			(&blocks[newblock->next])->prev = newidx;
		}
		puts("Exit split true");
		return index;
	}
	printf("Exit split false %d; %ld %d\n", block->free, length + ADMIN_SIZE + 1, block->length);
	return -1;
}

block_t *new_block(long index, long leng, long next, long prev){
	block_t *block = (block_t*) &mem[index];
	printf("Enter new %p\n", block);
	
	block->free = 1;
	block->length = (short) leng;
	block->next = (short) next;
	block->prev = (short) prev;
	
	puts("Half way");
	admin->loos += ADMIN_SIZE;
	puts("Exit new");
	return block;
}

void fuse_block(long index){
	block_t *block = (block_t*) &mem[index];
	
	if(block->prev != 0 && blocks[block->prev].free){
		block_t *prevblock = (block_t*) &mem[block->prev];
		
		prevblock->next = block->next;
		prevblock->length += block->length;
		
		if(block->next != 0){
			((block_t*) &mem[block->next])->prev = block->prev;
			fuse_block(block->next);
		}
		
		admin->loos -= ADMIN_SIZE;
	}
}

void print_blocks(){
	block_t *block;
	long idx = FIRST_BLOCK;
	while(idx != 0){
		block = (block_t*) &mem[idx];
		printf("Block: idx: %ld; leng: %d; free: %d\n", idx, block->length, block->free);
		idx = block->next;
	}
	puts("");
}

long  mem_get(long request){
	long largest = 0;
	long largestidx = 0;
	long idx = FIRST_BLOCK;
	
	print_blocks();
	while(idx != 0){
		if(blocks[idx].free && blocks[idx].length > largest){
			largest = blocks[idx].length;
			largestidx = idx;
		}
		idx = blocks[idx].next;
	}
	printf("Largest block is %ld with %ld words\n", largestidx, largest);
	
	if(largest < request || largestidx == 0){
		return -1;
	}else{
		int ret = split_block(largestidx, request);
		printf("Alloc %ld ? %d\n", request, ret);
		return ret;
	}
	return -1;
}

void mem_free(long index){
	/* TODO */
	(void)(index);
}

double mem_internal(){
	return (double) admin->loos / (double) admin->toegewezen;
}

void mem_available(long *empty, long *large, long *n_holes){
	/* TODO */
	(void)(empty);
	(void)(large);
	(void)(n_holes);
}

void mem_exit(){
}
