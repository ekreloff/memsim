/*
 * Cache Memory Simulator
 * by Ethan Kreloff
 * 
 * Created November 25, 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//Set to 1 to allow doing multiple simulations
#define INTERACTIVE 0

//cache constants in bytes
#define L1BLOCKSIZE 32
#define L1BUSSIZE 4
#define L2BLOCKSIZE 64
#define L2BUSWIDTH 16

//In bits
#define ADDRSIZE 48

//cache constants in cycles
#define L1HITTIME 1
#define L1MISSTIME 1

#define L2HITTIME 4
#define L2MISSTIME 6
#define L2TRANSFERTIME 6

#define MEMSENDADDR 10
#define MEMREADY 50
#define MEMCHUNKTIME 20

#define true 1
#define false 0
typedef int bool;

typedef struct node{
	unsigned long long int address;
	unsigned long long int tag;
	unsigned int block_number;
	bool dirty;
	bool valid;
	struct node *child;
} Node;


int main( int argc, const char* argv[] ){
	
	FILE *config_fp;
	//int input = ' '; for interactive mode
	char op;
	unsigned long long int address;
	unsigned int bytesize;
	
	//Initialize parameters
	int l1_cache_size = 8192; 
	int l1_assoc = 1;
	int l2_cache_size = 65536;
	int l2_assoc = 1;
	int mem_chunk_size = 16;
	
	//Process config file to change defaults
	if((config_fp = fopen(argv[argc-1], "r")) != NULL){
		while(fscanf(config_fp, "%i %i %i %i %i", 
			&l1_cache_size, &l1_assoc, &l2_cache_size, &l2_assoc, &mem_chunk_size) == 5){
			//printf("\n%i %i %i %i %i\n", l1_cache_size, l1_assoc, l2_cache_size, l2_assoc, mem_chunk_size);
	  }
	}
	
	int i;
	Node *next, *l1d_root, *l1i_root, *l2_root;
	l1d_root = NULL;
	l1i_root = NULL;
	l2_root = NULL;
	
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l1_cache_size/L1BLOCKSIZE - i - 1; 
			next->dirty = false;
			next->valid = false;
			next->child = l1d_root;
			l1d_root = next;
		}
		
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l1_cache_size/L1BLOCKSIZE - i - 1; 
			next->dirty = false;
			next->valid = false;
			next->child = l1i_root;
			l1i_root = next;
		}
		
	for(i = 0; i < l2_cache_size/L2BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l2_cache_size/L2BLOCKSIZE - i - 1;
			next->dirty = false;
			next->valid = false;
			next->child = l2_root;
			l2_root = next;
		}
		

		/*Node *next1, *next2;
		next1 = l1_root;
		next2 = l2_root;
		i = 0;
		while(next1 != NULL){
			i++;
			//printf("\n%i %i", i, next->address);
			printf("\n%i %i", i, next1->block_number);
			next1 = next1->child; 
		
		}
		
		return 0;*/
	
	Node *current1/*, *current2*/; 
	
	
	/*//Data being kept
	unsigned long long int execution_time = 0;
	unsigned long long int data_read_refs = 0;
	unsigned long long int data_write_refs = 0;
	unsigned long long int inst_refs = 0;
	unsigned long long int read_cycles = 0;
	unsigned long long int write_cycles = 0;
	unsigned long long int inst_cycles = 0;
	
	//Cache specific data
	unsigned long long int l1d_hit_count = 0;
	unsigned long long int l1d_miss_count = 0;
	unsigned long long int l1d_kickouts = 0;
	unsigned long long int l1d_dirty_kickouts = 0;
	unsigned long long int l1d_transfers = 0;
	
	unsigned long long int l1i_hit_count = 0;
	unsigned long long int l1i_miss_count = 0;
	unsigned long long int l1i_kickouts = 0;
	unsigned long long int l1i_dirty_kickouts = 0;
	unsigned long long int l1i_transfers = 0;
	
	unsigned long long int l2_hit_count = 0;
	unsigned long long int l2_miss_count = 0;
	unsigned long long int l2_kickouts = 0;
	unsigned long long int l2_dirty_kickouts = 0;
	unsigned long long int l2_transfers = 0;
*/
	
	
	//this is for DM only, minor changes for other formats
	int l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(l1_cache_size/L1BLOCKSIZE)/log(2));
	//int l2_index_bits = (log(L2BLOCKSIZE)/log(2)) + (log(l2_cache_size/L2BLOCKSIZE)/log(2));
	unsigned long long int maskliteral = 0xFFFFFFFFFFFF;
	unsigned long long int tagmask1 = (maskliteral << l1_index_bits) & maskliteral;
	//unsigned long long int tagmask2 = (maskliteral << l2_index_bits) & maskliteral;
	/*unsigned long long int indexmask1 = ~tagmask1;
	unsigned long long int indexmask2 = ~tagmask2;
	
	
	//printf("%i", sizeof(tagmask1));*/
	
	unsigned int references;
	unsigned int counter = 0; 
	unsigned int addresscounter=0;
	
	
	NEW_ADDRESS:
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		addresscounter++;
		//op == 'I' ? current1 = l1i_root : current1 = l1d_root; Oddly this doesnt
		// work and is equivalent to the code below.
		
		
		printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter);
		printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
			l1_index_bits, tagmask1, address, address & tagmask1);
		
		references = (int)(ceil((address%4 + bytesize)/4.0)); 
		address =  address - (address%4);
		printf("refs %u\n",references);
		
			//op == 'W' ? printf("Write\n") : printf("Read or Instruction\n");
		counter = 0;
		NEW_WORD:
		while(counter < references){ 
			if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}	
			//address = address + counter*4;
			while(current1 != NULL){
				if(current1->block_number == address%(l1_cache_size/L1BLOCKSIZE)){
					printf("blocks match: %i %llu\n", current1->block_number, 
											  address%(l1_cache_size/L1BLOCKSIZE));
					printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
					printf("current address:%#llX\n", address);
					if(current1->tag == (address & tagmask1)){
						if(current1->valid){
							printf("Cache Hit\n");
							if(op == 'W'){
								current1->dirty = true; printf("entry marked dirty\n");
								current1->address = address;
								current1->tag = address & tagmask1;
								current1->valid = true;
							}else{
								current1->valid = true;
							}
							if(!((counter + 1) < references)){
								goto NEW_ADDRESS;
							}else{
								counter++;
								address += 4;
								goto NEW_WORD;
							}
						}
					}
					printf("Cache Miss\n");
					if(current1->dirty){
						printf("need to write to level 2\n");
					}
				
					printf("Need to return data from lower level memory\n");
					current1->address = address;
					current1->tag = address & tagmask1;
					current1->valid = true;
					if(op == 'W'){
						current1->dirty = true;
					}else{current1->dirty = false;}
					
				}
				current1 = current1->child;
			}
			counter++;
			address += 4;
		} 
	}
	return 0;
	
	/*while(input != 'q' && input != 'Q' && INTERACTIVE){
		
		printf("\nWelcome to the cache simulator. Please select from the options below:\n\
				\n[Q]uit - Exits program.\n");
		input = getchar();
	
	}
	
	return 0;*/
}


