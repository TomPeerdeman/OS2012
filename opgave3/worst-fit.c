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
	new_block(2, (MEM_SIZE - ADMIN_SIZE - 2), 0, 0);
	print_blocks();
}

long  mem_get(long request){
	long l = request;
	long lidx = 0;
	long idx = 2;
	printf("\nRequest %ld\n", request);
	
	while(idx != 0){
		if(get_length(idx) >= l && get_free(idx)){
			lidx = idx;
			l = get_length(idx);
		}
		
		idx = get_next(idx);
	}
	
	if(l == request && lidx != 0){
		/* Gat precies groot genoeg. */
		printf("Assign: %ld\n", l);
		set_free(lidx, 0);
		return lidx + ADMIN_SIZE;
	}else if(l < request || lidx == 0){
		/* Geen gat groot genoeg. */
		printf("Can't assign: %ld/%ld\n", l, request);
		print_blocks();
		return -1;
	}else{
		/* Alleen een gat > request. */
		int ret = split_block(lidx, request);
		print_blocks();
		return ret;
	}
}

void mem_free(long index){
	printf("\nFree %ld\n", index - ADMIN_SIZE);
	free_block(index - ADMIN_SIZE);
	print_blocks();
}

double mem_internal(){
	/* NOTE: Wordt niet meer aangeroepen door de simulator. */
	return (double) mem[1] / (double) mem[0];
}

void mem_available(long *empty, long *large, long *n_holes){
	/* TODO: finish */
	*empty = MEM_SIZE - mem[0] - mem[1];
	(void)(large);
	(void)(n_holes);
}

void mem_exit(){
}
