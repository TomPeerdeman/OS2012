#include "mem_alloc.h"
#include "mem-func.h"

#include <stdio.h>

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
