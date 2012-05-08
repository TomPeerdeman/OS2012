typedef struct admin_t{
	short loos;
	short toegewezen;
} admin_t;

typedef struct block_t{
	short free;
	unsigned short length;
	unsigned short prev;
	unsigned short next;
} block_t;