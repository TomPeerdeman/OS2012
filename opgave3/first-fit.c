#include "mem_alloc.h"

#include <stdio.h>
#include <stdlib.h>


/* TODO: Netter in h file zetten. */
void new_block(long index, long length, long prev, long next);

int split_block(long index, long leng);

void set_free(long index, long free);

void print_blocks();


/* 
 * Blok formaat:
 * 
 * long lengte_blok
 * long index_vorige_blok 	
 * 	(wijst naar lengte_blok long van vorige blok; 0 = eerste blok)
 * long index_volgend_blok 
 * 	(wijst naar lengte_blok long van vorige blok; 0 = laatste blok)
 * long vrij			
 * 	(1 = vrij, 0 = toegewezen)
 * long[lengte_blok] data
 *
 */
 
 /* Lengte van een administratie blok. Aantal long vorafgaande het data
  * blok. */
 #define ADMIN_SIZE 4

long *mem;

void mem_init(long memory[MEM_SIZE]){
	mem = memory;
	
	/* Aantal toegewezen woorden. */
	mem[0] = 0;
	/* Aantal loze woorden. */
	mem[1] = 0;
	
	/* Eerste vrije blok. */
	new_block(2, (MEM_SIZE - ADMIN_SIZE), 0, 0);
	print_blocks();
}

/* Accessor functies. */
void set_length(long index, long length){
	mem[index] = length;
}

void set_prev(long index, long prev){
	mem[index + 1] = prev;
}

void set_next(long index, long next){
	mem[index + 2] = next;
}

void set_free(long index, long free){
	mem[index + 3] = free;
}

long get_length(long index){
	return mem[index];
}

long get_prev(long index){
	return mem[index + 1];
}

long get_next(long index){
	return mem[index + 2];
}

long get_free(long index){
	return mem[index + 3];
}
/* Einde accessor functies. */

void print_blocks(){
	long idx = 2;
	while(idx != 0){
		printf("Block at idx %ld; length: %ld; free: %ld; prev: %ld; next: %ld\n", idx, get_length(idx), get_free(idx), get_prev(idx), get_next(idx));
		idx = get_next(idx);
	}
}


long  mem_get(long request){
	long l = 0;
	long lidx = 0;
	long idx = 2;
	printf("\nRequest %ld\n", request);
	print_blocks();
	while(idx != 0){
		if(get_length(idx) > l && get_free(idx)){
			lidx = idx;
			l = get_length(idx);
		}
		
		idx = get_next(idx);
	}
	
	if(l < request || lidx == 0){
		printf("Can't assign: %ld/%ld\n", l, request);
		return -1;
	}else{
		int ret = split_block(lidx, request);
		if(ret){
			mem[0] += request;
		}
		return ret;
	}
}

void mem_free(long index){
	/* TODO */
	printf("\nFree %ld\n", index - ADMIN_SIZE);
}

double mem_internal(){
	/* NOTE: Wordt niet meer aangeroepen door de simulator. */
	return (double) mem[1] / (double) mem[0];
}

void mem_available(long *empty, long *large, long *n_holes){
	/* TODO: finish */
	*empty = MEM_SIZE - mem[0];
	(void)(large);
	(void)(n_holes);
}

void mem_exit(){
}


void new_block(long index, long length, long prev, long next){
	printf("New block at %ld; length: %ld; prev: %ld; next: %ld\n", index, length, prev, next);
	set_length(index, length);
	set_prev(index, prev);
	set_next(index, next);
	set_free(index, 1);
}

int split_block(long index, long length){
	long blockleng = get_length(index);
	long newidx = index + length + ADMIN_SIZE;
	long newleng = blockleng - length - ADMIN_SIZE;
	
	printf("Split %ld at %ld\n", blockleng, length);
	if(blockleng < length + ADMIN_SIZE + 1){
		return -1;
	}

	new_block(newidx, newleng, index, get_next(index));
	
	if(get_next(index) != 0){
		set_prev(get_next(index), newidx);
	}
	set_next(index, newidx);
	set_length(index, length);
	set_free(index, 0);
	mem[1] += ADMIN_SIZE;
	
	return index + ADMIN_SIZE;
}
