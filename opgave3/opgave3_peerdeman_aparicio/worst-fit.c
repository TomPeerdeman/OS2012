#include "mem_alloc.h"
#include "mem-func.h"

void mem_init(long memory[MEM_SIZE]){
	mem = memory;
	
	/* Aantal toegewezen woorden. */
	mem[0] = 0;
	/* Aantal loze woorden. */
	mem[1] = 0;
	
	/* Eerste vrije blok. */
	new_block(2, (MEM_SIZE - ADMIN_SIZE - 2), 0, 0);
}

long  mem_get(long request){
	long l = request;
	long lidx = 0;
	long idx = 2;
	
	while(idx != 0){
		if(get_length(idx) >= l && get_free(idx)){
			lidx = idx;
			l = get_length(idx);
		}
		
		idx = get_next(idx);
	}
	
	if(l == request && lidx != 0){
		/* Gat precies groot genoeg. */
		set_free(lidx, 0);
		return lidx + ADMIN_SIZE;
	}else if(l < request || lidx == 0){
		/* Geen gat groot genoeg. */
		return -1;
	}else{
		/* Alleen een gat > request. */
		return split_block(lidx, request);
	}
}

void mem_free(long index){
	free_block(index - ADMIN_SIZE);
}

double mem_internal(){
	/* NOTE: Wordt niet meer aangeroepen door de simulator. */
	return (double) mem[1] / (double) mem[0];
}

void mem_available(long *empty, long *large, long *n_holes){
	long idx = 2;
	long next;
	long prev;
	
	*empty = MEM_SIZE - mem[0] - mem[1];
	
	*large = 0;
	*n_holes = 0;
	while(idx != 0){
		if(get_length(idx) > *large){
			*large = get_length(idx);
		}
		next = get_next(idx);
		prev = get_prev(idx);
		if(next && prev && !get_free(next) && !get_free(prev)){
			*n_holes += 1;
		}
		idx = get_next(idx);
	}
}

void mem_exit(){
}
