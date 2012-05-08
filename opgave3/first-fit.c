#include "mem_alloc.h"

#include <stdio.h>
#include <stdlib.h>

void new_block(long index, long length, long prev);

int split_block(long index, long leng);

int get_free_size(long index);

void set_free(long index);

void set_taken(long index);

int is_free(long index);

/* 
 * Blok formaat:
 * 
 * long lengte_blok
 * long index_vorige_blok 	(wijst naar lengte_blok long van vorige blok)
 * long[lengte_blok] data
 *
 */

long *mem;

void mem_init(long memory[MEM_SIZE]){
	mem = memory;
	
	/* Aantal toegewezen woorden. */
	mem[0] = 0;
	/* Aantal loze woorden. */
	mem[1] = 2;
	
	/* Eerste vrije blok. */
	new_block(2, (MEM_SIZE - 2), 0);
	set_free(2);
}

long  mem_get(long request){
	/* TODO */
	return request;
}

void mem_free(long index){
	/* TODO */
	(void)(index);
}

double mem_internal(){
	return (double) mem[1] / (double) mem[0];
}

void mem_available(long *empty, long *large, long *n_holes){
	/* TODO */
	(void)(empty);
	(void)(large);
	(void)(n_holes);
}

void mem_exit(){
}


void new_block(long index, long length, long prev){
	mem[index] = length;
	mem[index + 1] = prev;
	mem[1] += 2;
}

int split_block(long index, long leng){
	if(mem[index] < leng + 2){
		return 0;
	}
	long new_idx = index + leng + 2;
	new_block(new_idx, mem[index], index);
	mem[index] = leng;
	
	/* Zet de prev van het volgende blok goed naar het nieuwe blok. */
	mem[mem[index] + index + 3] = new_idx;
	return index;
}

int get_free_size(long index){
	if(is_free(index)){
		return mem[index] & 0xFFFF;
	}else{
		return 0;
	}
}

void set_free(long index){
	/* Zet bit 24 naar 1. */
	mem[index] |= 0x800000;
}

void set_taken(long index){
	mem[index] &= 0xFFFF;
}

int is_free(long index){
	/* Groter dan 16 bits dus het vrij bit moet wel gezet zijn. */
	return (mem[index] > MEM_SIZE);
}
