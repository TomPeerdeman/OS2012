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
	
	block = &blocks[FIRST_BLOCK];
	block->free = 1;
	block->length = MEM_SIZE - admin->loos - FIRST_BLOCK;
	block->next = 0;
	block->prev = 0;
	
	print_blocks();
}

int split_block(long index, long length){
	block_t *block = &blocks[index];

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
		return index;
	}
	return 0;
}

block_t *new_block(long index, long leng, long next, long prev){
	block_t *block = &blocks[index];
	
	block->free = 1;
	block->length = (short) leng;
	block->next = (short) next;
	block->prev = (short) prev;
	admin->loos += ADMIN_SIZE;
	return block;
}

void fuse_block(long index){
	block_t *block = &blocks[index];
	
	if(block->prev != 0 && blocks[block->prev].free){
		block_t *prevblock = &blocks[block->prev];
		
		prevblock->next = block->next;
		prevblock->length += block->length;
		
		if(block->next != 0 && blocks[block->next].free){
			(&blocks[block->next])->prev = block->prev;
		}
		
		admin->loos -= ADMIN_SIZE;
	}
}

void print_blocks(){
	block_t *block;
	long idx = FIRST_BLOCK;
	while(idx != 0){
		block = &blocks[idx];
		printf("Block: idx: %ld; leng: %d; free: %d\n", idx, block->length, block->free);
		idx = block->next;
	}
}

long  mem_get(long request){
	/* TODO */
	(void)(request);
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
