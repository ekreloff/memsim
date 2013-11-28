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
#define false 2
typedef int bool;

typedef struct node{
	unsigned long long int address;
	bool dirty;
	bool valid;
	struct node *child;
} Node;


int main( int argc, const char* argv[] ){
	
	FILE *config_fp;
	int input = ' ';
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
	
	//this is for DM only, minor changes for other formats
	int l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(l1_cache_size/L1BLOCKSIZE)/log(2));
	int l2_index_bits = (log(L2BLOCKSIZE)/log(2)) + (log(l2_cache_size/L2BLOCKSIZE)/log(2));
	unsigned long long int tagmask1 = 0xFFFFFFFFFFFFFFFF << l1_index_bits;
	unsigned long long int tagmask2 = 0xFFFFFFFFFFFFFFFF << l2_index_bits;
	unsigned long long int indexmask1 = ~tagmask1;
	unsigned long long int indexmask2 = ~tagmask2;
	
	int i;
	Node *next, *l1_root, *l2_root;
	l1_root = NULL;
	l2_root = NULL;
	
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->address = i;
			next->dirty = false;
			next->valid = false;
			next->child = l1_root;
			l1_root = next;
		}
		
	for(i = 0; i < l2_cache_size/L2BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->address = i;
			next->dirty = false;
			next->valid = false;
			next->child = l2_root;
			l2_root = next;
		}

		/*Node *next1, *next2;
		next1 = l1_root;
		next2 = l2_root;
		i = 0;
		while(next2 != NULL){
			i++;
			//printf("\n%i %i", i, next->address);
			printf("\n%i %i", i, next2->address);
			next2 = next2->child; 
		
		}*/
	
	Node *current1 = l1_root;
	Node *current2 = l2_root; 
	
	
	//Data being kept
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
	
	
	
	
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		while(current1 != NULL){
			if((current1->address && indexmask1) == address && indexmask1){
				break;
			}/*else{printf("cache miss\n");}*/
			
			
			current1 = current1->child;
		}
		
		printf("cache hit\n");
		
		 
		
	}
	return 0;
	
	while(input != 'q' && input != 'Q' && INTERACTIVE){
		
		printf("\nWelcome to the cache simulator. Please select from the options below:\n\
				\n[Q]uit - Exits program.\n");
		input = getchar();
	
	}
	
	return 0;
}

//void simulate
