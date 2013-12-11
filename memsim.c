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
//#define INTERACTIVE 0

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
	unsigned int set_number;
	unsigned long long int block_number;
	unsigned int LRU;
	bool LRU_replacment;
	bool dirty;
	bool valid;
	struct node *child;
} Node;

typedef struct data{
	unsigned long long int execution_time;
	unsigned long long int data_read_refs;
	unsigned long long int data_write_refs;
	unsigned long long int inst_refs;
	unsigned long long int read_cycles;
	unsigned long long int write_cycles;
	unsigned long long int inst_cycles;
	unsigned long long int l1d_hit_count;
	unsigned long long int l1d_miss_count;
	unsigned long long int l1d_kickouts;
	unsigned long long int l1d_dirty_kickouts;
	unsigned long long int l1d_transfers;
	unsigned long long int l1i_hit_count;
	unsigned long long int l1i_miss_count;
	unsigned long long int l1i_kickouts;
	unsigned long long int l1i_dirty_kickouts;
	unsigned long long int l1i_transfers;
	unsigned long long int l2_hit_count;
	unsigned long long int l2_miss_count;
	unsigned long long int l2_kickouts;
	unsigned long long int l2_dirty_kickouts;
	unsigned long long int l2_transfers;
}Data;


void l2cache(int l2_assoc, int l2_cache_size, Node *l2_root, unsigned long long int address,
			 char op, Data *data_ptr);

void l2cachewrite(int l2_assoc, int l2_cache_size, Node *l2_root, unsigned long long int address,
                  Data * data_ptr);

int main( int argc, const char* argv[] ){
	
	FILE *config_fp;
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
	  }
	}
	
	
	int i; 
	unsigned int set_number = 0;
    Node *next, *l1d_root, *l1i_root, *l2_root;
	l1d_root = NULL;
	l1i_root = NULL;
	l2_root = NULL;
	
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l1_cache_size/L1BLOCKSIZE - i - 1; 
			next->LRU = 0;
			next->LRU_replacment = false;
			next->dirty = false;
			next->valid = false;
			next->child = l1d_root;
			l1d_root = next;
			if(l1_assoc > 1){
				next->set_number = set_number;
				if((i%l1_assoc) == (l1_assoc - 1)){
					set_number++;
				}
			}
		}
	set_number = 0;
		
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l1_cache_size/L1BLOCKSIZE - i - 1; 
			next->LRU = 0;
			next->LRU_replacment = false;
			next->dirty = false;
			next->valid = false;
			next->child = l1i_root;
			l1i_root = next;
			if(l1_assoc > 1){
				next->set_number = set_number;
				if((i%l1_assoc) == (l1_assoc - 1)){
					set_number++;
				}
			}
		}
	
	set_number = 0;
		
	for(i = 0; i < l2_cache_size/L2BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l2_cache_size/L2BLOCKSIZE - i - 1;
			next->LRU = 0;
			next->LRU_replacment = false;
			next->dirty = false;
			next->valid = false;
			next->child = l2_root;
			l2_root = next;
			if(l2_assoc > 1){
				next->set_number = set_number;
				if((i%l2_assoc) == (l2_assoc - 1)){
					set_number++;
				}
			}
		}
		
	
	Data *data_ptr;
    data_ptr = (Data *)malloc(sizeof(Data));
	//Initialize Data being kept
	data_ptr->execution_time = 0;
	data_ptr->data_read_refs = 0;
	data_ptr->data_write_refs = 0;
	data_ptr->inst_refs = 0;
	data_ptr->read_cycles = 0;
	data_ptr->write_cycles = 0;
	data_ptr->inst_cycles = 0;
	
	//Cache specific data
	data_ptr->l1d_hit_count = 0;
	data_ptr->l1d_miss_count = 0;
	data_ptr->l1d_kickouts = 0;
	data_ptr->l1d_dirty_kickouts = 0;
	data_ptr->l1d_transfers = 0;
	
	data_ptr->l1i_hit_count = 0;
	data_ptr->l1i_miss_count = 0;
	data_ptr->l1i_kickouts = 0;
	data_ptr->l1i_dirty_kickouts = 0;
	data_ptr->l1i_transfers = 0;
	
	data_ptr->l2_hit_count = 0;
	data_ptr->l2_miss_count = 0;
	data_ptr->l2_kickouts = 0;
	data_ptr->l2_dirty_kickouts = 0;
	data_ptr->l2_transfers = 0;


	Node *current1, *setassocsearch1;
	int l1_index_bits, num_of_sets;
	int highest_LRU = 0;
	
	if(l1_assoc == 1){
		l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(l1_cache_size/L1BLOCKSIZE)/log(2));
	}else{if(l1_assoc == 0){
		l1_index_bits = (log(L1BLOCKSIZE)/log(2));
		}else{
			num_of_sets = (l1_cache_size/L1BLOCKSIZE)/l1_assoc;
			l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(num_of_sets)/log(2));
		}
	}
	
	
	unsigned long long int maskliteral = 0xFFFFFFFFFFFF;
	unsigned long long int tagmask1 = (maskliteral << l1_index_bits) & maskliteral;
	unsigned long long int indexmask1 = (maskliteral >> (48 - l1_index_bits)) & 
										(maskliteral << (int)(log(L1BLOCKSIZE)/log(2)));
	
	unsigned int references;
	unsigned int counter = 0; 
	unsigned int addresscounter = 0;
	switch(l1_assoc){
		case 0:
            NEW_ADDRESS_FA:
            while (scanf("%c %llx %d\n", &op, &address, &bytesize) == 3) {
                addresscounter++;
                printf("index mask: %#llX\n", indexmask1);
                printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter - 1);
                printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
                       l1_index_bits, tagmask1, address, address & tagmask1);
		
                references = (int)(ceil((address%4 + bytesize)/4.0));
                printf("refs %u\n",references);
                address =  address - (address%4);
                counter = 0;
		
                NEW_WORD_FA:
                while(counter < references){
                    if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}
                        while(current1 != NULL){
                            if(current1->tag == (address & tagmask1)){
                                printf("tags match: %#llX %#llX\n", current1->tag, (address & tagmask1));
                                printf("current address:%#llX\n", address);
                                if(current1->valid){
                                    printf("Cache Hit\n");
                                    if(op == 'I'){data_ptr->l1i_hit_count++;}else{data_ptr->l1d_hit_count++;}
                                    if(op == 'W'){
                                        current1->dirty = true; printf("entry marked dirty\n");
                                        current1->address = address;
                                        current1->tag = address & tagmask1;
                                        current1->valid = true;
                                    }else{
                                        current1->valid = true;
                                    }
							
                                    if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                    while(setassocsearch1 != NULL){
                                        (setassocsearch1->LRU)++;
                                        setassocsearch1 = setassocsearch1->child;
                                    }
							
                                    current1->LRU = 0;
							
                                    if(!((counter + 1) < references)){
                                        highest_LRU = 0;
                                        goto NEW_ADDRESS_FA;
                                    }else{
                                        highest_LRU = 0;
                                        counter++;
                                        address += 4;
                                        goto NEW_WORD_FA;
                                    }
                                }
                            }
                            (current1->LRU)++;
                            if(current1->block_number == (unsigned long long int)(l1_cache_size/L1BLOCKSIZE - 1)){
                                if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                while(setassocsearch1 != NULL){
                                    if(setassocsearch1->LRU > (unsigned int)highest_LRU){
                                        highest_LRU = setassocsearch1->LRU;
                                    }
                                    setassocsearch1 = setassocsearch1->child;
                                }
                                if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                while(setassocsearch1 != NULL){
                                    if(setassocsearch1->LRU == (unsigned int)highest_LRU){
                                        break;
                                    }
                                    setassocsearch1 = setassocsearch1->child;
                                }
                                printf("Cache Miss\n");
                                if(op == 'I'){data_ptr->l1i_miss_count++;}else{data_ptr->l1d_miss_count++;}
                                setassocsearch1->LRU = 0;
                                printf("%llu\n", setassocsearch1->block_number);
                                if(setassocsearch1->dirty){
                                    printf("need to write to level 2\n");
                                    l2cachewrite(l2_assoc, l2_cache_size, l2_root, address, data_ptr);
                                    if(current1->tag != 0){
                                        if(op == 'I'){data_ptr->l1i_dirty_kickouts++;}else{data_ptr->l1d_dirty_kickouts++;}
                                    }
                                }
                                if(current1->tag != 0){
                                    if(op == 'I'){data_ptr->l1i_kickouts++;}else{data_ptr->l1d_kickouts++;}
                                }
                                printf("Need to return data from lower level memory\n");
                                l2cache(l2_assoc, l2_cache_size, l2_root, address, op, data_ptr);
                                setassocsearch1->address = address;
                                setassocsearch1->tag = address & tagmask1;
                                setassocsearch1->valid = true;
                                if(op == 'W'){
                                    setassocsearch1->dirty = true;
                                }else{setassocsearch1->dirty = false;}
                                if(!((counter + 1) < references)){
                                    highest_LRU = 0;
                                    goto NEW_ADDRESS_FA;
                                }else{
                                    highest_LRU = 0;
                                    counter++;
                                    address += 4;
                                    goto NEW_WORD_FA;
                                }
                            }
                            current1 = current1->child;
                        }
                    counter++;
                    address += 4;
                }
            }
		break;
		case 1:
            NEW_ADDRESS:
            while (scanf("%c %llx %d\n", &op, &address, &bytesize) == 3) {
                addresscounter++;
                printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter - 1);
                printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
                       l1_index_bits, tagmask1, address, address & tagmask1);
                references = (int)(ceil((address%4 + bytesize)/4.0));
                printf("refs %u\n",references);
                address =  address - (address%4);
                counter = 0;
		
                NEW_WORD:
                while(counter < references){
                    if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}
                    while(current1 != NULL){
                        if(current1->block_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
                            printf("blocks match: %llu %llu\n", current1->block_number,
                                   (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2)));
                            printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
                            printf("current address:%#llX\n", address);
                            if(current1->tag == (address & tagmask1)){
                                if(current1->valid){
                                    printf("Cache Hit\n");
                                    if(op == 'I'){data_ptr->l1i_hit_count++;}else{data_ptr->l1d_hit_count++;}
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
                            if(op == 'I'){data_ptr->l1i_miss_count++;}else{data_ptr->l1d_miss_count++;}
                            if(current1->dirty){
                                printf("need to write to level 2\n");
                                l2cachewrite(l2_assoc, l2_cache_size, l2_root, address, data_ptr);
                                if(current1->tag != 0){
                                    if(op == 'I'){data_ptr->l1i_dirty_kickouts++;}else{data_ptr->l1d_dirty_kickouts++;}
                                }
                            }
                            if(current1->tag != 0){
                                if(op == 'I'){data_ptr->l1i_kickouts++;}else{data_ptr->l1d_kickouts++;}
                            }
                            printf("Need to return data from lower level memory\n");
                            l2cache(l2_assoc, l2_cache_size, l2_root, address, op, data_ptr);
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
        break;
        default:
            NEW_ADDRESS_SA:
            while (scanf("%c %llx %d\n", &op, &address, &bytesize) == 3) {
                addresscounter++;
                printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter - 1);
                printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
                       l1_index_bits, tagmask1, address, address & tagmask1);
                references = (int)(ceil((address%4 + bytesize)/4.0));
                printf("refs %u\n",references);
                address =  address - (address%4);
                counter = 0;
                NEW_WORD_SA:
                while(counter < references){
                    if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}
                    while(current1 != NULL){
                        if(current1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
                            printf("sets match: %i %llu\n", current1->set_number,
                                   (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2)));
                            printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
                            printf("current address:%#llX\n", address);
                            if(current1->tag == (address & tagmask1)){
                                if(current1->valid){
                                    printf("Cache Hit\n");
                                    if(op == 'I'){data_ptr->l1i_hit_count++;}else{data_ptr->l1d_hit_count++;}
                                    if(op == 'W'){
                                        current1->dirty = true; printf("entry marked dirty\n");
                                        current1->address = address;
                                        current1->tag = address & tagmask1;
                                        current1->valid = true;
                                    }else{
                                        current1->valid = true;
                                    }
                                    if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                    while(setassocsearch1 != NULL){
                                        if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
                                            (setassocsearch1->LRU)++;
                                        }
                                        setassocsearch1 = setassocsearch1->child;
                                    }
							
                                    current1->LRU = 0;
							
                                    if(!((counter + 1) < references)){
                                        highest_LRU = 0;
                                        goto NEW_ADDRESS_SA;
                                    }else{
                                        counter++;
                                        address += 4;
                                        highest_LRU = 0;
                                        goto NEW_WORD_SA;
                                    }
                                }
                            }
					
                            if((current1->block_number%l1_assoc) == (l1_assoc - 1)){
                                printf("Cache Miss\n");
                                if(op == 'I'){data_ptr->l1i_miss_count++;}else{data_ptr->l1d_miss_count++;}
                                if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                while(setassocsearch1 != NULL){
                                    if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
                                        (setassocsearch1->LRU)++;
                                    }
                                    setassocsearch1 = setassocsearch1->child;
                                }
                                if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                while(setassocsearch1 != NULL){
                                    if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
                                        if(setassocsearch1->LRU > (unsigned int)highest_LRU){
                                            highest_LRU = setassocsearch1->LRU;
                                            printf("highest: %i, current: %i\n", highest_LRU, setassocsearch1->LRU);
                                        }
                                    }
                                    setassocsearch1 = setassocsearch1->child;
                                }
                                if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
                                while(setassocsearch1 != NULL){
                                    if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
                                        if(setassocsearch1->LRU == (unsigned int)highest_LRU){
                                            break;
                                        }
                                    }
                                    setassocsearch1 = setassocsearch1->child;
                                }
                                setassocsearch1->LRU = 0;
                                printf("%llu\n", setassocsearch1->block_number);
                                if(setassocsearch1->dirty){
                                    printf("need to write to level 2\n");
                                    l2cachewrite(l2_assoc, l2_cache_size, l2_root, address, data_ptr);
                                    if(current1->tag != 0){
                                        if(op == 'I'){data_ptr->l1i_dirty_kickouts++;}else{data_ptr->l1d_dirty_kickouts++;}
                                    }
                                }
                                if(current1->tag != 0){
                                    if(op == 'I'){data_ptr->l1i_kickouts++;}else{data_ptr->l1d_kickouts++;}
                                }
                                printf("Need to return data from lower level memory\n");
                                l2cache(l2_assoc, l2_cache_size, l2_root, address, op, data_ptr);
                                setassocsearch1->address = address;
                                setassocsearch1->tag = address & tagmask1;
                                setassocsearch1->valid = true;
                                if(op == 'W'){
                                    setassocsearch1->dirty = true;
                                }else{setassocsearch1->dirty = false;}
                                if(!((counter + 1) < references)){
                                    highest_LRU = 0;
                                    goto NEW_ADDRESS_SA;
                                }else{
                                    counter++;
                                    address += 4;
                                    highest_LRU = 0;
                                    goto NEW_WORD_SA;
                                }
				
                            }
                        }
                        current1 = current1->child;
                    }
                    counter++;
                    address += 4;
                }
            }
        }
        printf("l1i hits: %llu l1i misses: %llu l1d hits: %llu l1d misses: %llu l2 hits: %llu l2 misses: %llu\n\
                l1i kickouts: %llu, dirty %llu l1d kickouts: %llu, dirty %llu l2 kickouts: %llu, dirty %llu\n",
               data_ptr->l1i_hit_count, data_ptr->l1i_miss_count, data_ptr->l1d_hit_count, data_ptr->l1d_miss_count,
               data_ptr->l2_hit_count, data_ptr->l2_miss_count, data_ptr->l1i_kickouts, data_ptr->l1i_dirty_kickouts,
               data_ptr->l1d_kickouts, data_ptr->l1d_dirty_kickouts, data_ptr->l2_kickouts, data_ptr->l2_dirty_kickouts);
        return 0;
	
}

void l2cache(int l2_assoc, int l2_cache_size, Node *l2_root, unsigned long long int address,
			 char op, Data * data_ptr){
	printf("	l2 called\n");
	Node *current2, *setassocsearch2;
	int l2_index_bits, num_of_sets;
	int highest_LRU = 0;
    
	if(l2_assoc == 1){
		l2_index_bits = (log(L2BLOCKSIZE)/log(2)) + (log(l2_cache_size/L2BLOCKSIZE)/log(2));
	}else{if(l2_assoc == 0){
		l2_index_bits = (log(L2BLOCKSIZE)/log(2));
		}else{
			num_of_sets = (l2_cache_size/L1BLOCKSIZE)/l2_assoc;
			l2_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(num_of_sets)/log(2));
		}
	}
	
	unsigned long long int maskliteral = 0xFFFFFFFFFFFF;
	unsigned long long int tagmask2 = (maskliteral << l2_index_bits) & maskliteral;
	unsigned long long int indexmask2 = (maskliteral >> (48 - l2_index_bits)) & 
										(maskliteral << (int)(log(L2BLOCKSIZE)/log(2)));
	
	switch(l2_assoc){
		case 0:
			current2 = l2_root;	
			while(current2 != NULL){
					if(current2->tag == (address & tagmask2)){
						printf("	tags match: %#llX %#llX\n", current2->tag, (address & tagmask2));
						printf("	current address:%#llX\n", address);
						if(current2->valid){
							printf("	Cache Hit\n");
                            data_ptr->l2_hit_count++;
							if(op == 'W'){
								current2->dirty = true; printf("	entry marked dirty\n");
								current2->address = address;
								current2->tag = address & tagmask2;
								current2->valid = true;
							}else{
								current2->valid = true;
							}
							setassocsearch2 = l2_root;
							while(setassocsearch2 != NULL){
								(setassocsearch2->LRU)++;
								setassocsearch2 = setassocsearch2->child;
							}
							current2->LRU = 0;
							break;
						}
					}
					(current2->LRU)++;
					if(current2->block_number == (unsigned long long int)(l2_cache_size/L2BLOCKSIZE - 1)){
                        setassocsearch2 = l2_root;
                        while(setassocsearch2 != NULL){
                            if(setassocsearch2->LRU > (unsigned int)highest_LRU){
                                highest_LRU = setassocsearch2->LRU;
                            }
                            setassocsearch2 = setassocsearch2->child;
                        }
                        setassocsearch2 = l2_root;
                        while(setassocsearch2 != NULL){
                            if(setassocsearch2->LRU == (unsigned int)highest_LRU){
                                break;
                            }
                            setassocsearch2 = setassocsearch2->child;
                        }
                        printf("	Cache Miss\n");
                        data_ptr->l2_miss_count++;
                        setassocsearch2->LRU = 0;
                        printf("	%llu\n", setassocsearch2->block_number);
                        if(setassocsearch2->dirty){
                            printf("	need to write to main\n");
                            if(current2->tag != 0){
                                data_ptr->l2_dirty_kickouts++;
                            }
                        }
                        printf("	Need to return data from main\n");
                        if(current2->tag != 0){data_ptr->l2_kickouts++;}
                        setassocsearch2->address = address;
                        setassocsearch2->tag = address & tagmask2;
                        setassocsearch2->valid = true;
                        if(op == 'W'){
                            setassocsearch2->dirty = true;
                        }else{setassocsearch2->dirty = false;}
                    }
                    current2 = current2->child;
                }
            break;
            case 1:
                current2 = l2_root;
                while(current2 != NULL){
                    if(current2->block_number == (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2))){
                        printf("	blocks match: %llu %llu\n", current2->block_number,
                               (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2)));
                        printf("	current tag:%#llX address tag:%#llX\n", current2->tag, address & tagmask2);
                        printf("	current address:%#llX\n", address);
                        if(current2->tag == (address & tagmask2)){
                            if(current2->valid){
                                printf("	Cache Hit\n");
                                data_ptr->l2_hit_count++;
                                if(op == 'W'){
                                    current2->dirty = true; printf("	entry marked dirty\n");
                                    current2->address = address;
                                    current2->tag = address & tagmask2;
                                    current2->valid = true;
                                }else{
                                    current2->valid = true;
                                }
                                break;
                            }
                        }
                        printf("	Cache Miss\n");
                        data_ptr->l2_miss_count++;
                        if(current2->dirty){
                            printf("	need to write to main\n");
                            if(current2->tag != 0){
                                data_ptr->l2_dirty_kickouts++;
                            }
                        }
                        if(current2->tag != 0){data_ptr->l2_kickouts++;}
                        printf("	Need to return data from main memory\n");
                        current2->address = address;
                        current2->tag = address & tagmask2;
                        current2->valid = true;
                        if(op == 'W'){
                            current2->dirty = true;
                        }else{current2->dirty = false;}
					
                    }
                    current2 = current2->child;
                }
            break;
            default:
                current2 = l2_root;
                while(current2 != NULL){
                    if(current2->set_number == (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2))){
                        printf("	sets match: %i %llu\n", current2->set_number,
                               (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2)));
                        printf("	current tag:%#llX address tag:%#llX\n", current2->tag, address & tagmask2);
                        printf("	current address:%#llX\n", address);
                        if(current2->tag == (address & tagmask2)){
                            if(current2->valid){
                                printf("	Cache Hit\n");
                                data_ptr->l2_hit_count++;
                                if(op == 'W'){
                                    current2->dirty = true; printf("	entry marked dirty\n");
                                    current2->address = address;
                                    current2->tag = address & tagmask2;
                                    current2->valid = true;
                                }else{
                                    current2->valid = true;
                                }
                                setassocsearch2 = l2_root;
                                while(setassocsearch2 != NULL){
                                    if(setassocsearch2->set_number == (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2))){
										(setassocsearch2->LRU)++;
                                    }
                                    setassocsearch2 = setassocsearch2->child;
                                }
                                current2->LRU = 0;
                                break;
                            }
                        }
                        if((current2->block_number%l2_assoc) == (l2_assoc - 1)){
                            printf("	Cache Miss\n");
                            data_ptr->l2_miss_count++;
                            setassocsearch2 = l2_root;
                            while(setassocsearch2 != NULL){
                                if(setassocsearch2->set_number == (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2))){
                                    (setassocsearch2->LRU)++;
								}
                                setassocsearch2 = setassocsearch2->child;
                            }
                            setassocsearch2 = l2_root;
                            while(setassocsearch2 != NULL){
                                if(setassocsearch2->set_number == (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2))){
									if(setassocsearch2->LRU > (unsigned int)highest_LRU){
										highest_LRU = setassocsearch2->LRU;
										printf("	highest: %i, current: %i\n", highest_LRU, setassocsearch2->LRU);
									}
								}
                                setassocsearch2 = setassocsearch2->child;
                            }
                            setassocsearch2 = l2_root;
                            while(setassocsearch2 != NULL){
                                if(setassocsearch2->set_number == (address & indexmask2) >> (int)(log(L2BLOCKSIZE)/log(2))){
									if(setassocsearch2->LRU == (unsigned int)highest_LRU){
										break;
									}
								}
                                setassocsearch2 = setassocsearch2->child;
                            }
                            setassocsearch2->LRU = 0;
                            printf("	%llu\n", setassocsearch2->block_number);
                            if(setassocsearch2->dirty){
                                printf("	need to write to main\n");
                                if(current2->tag != 0){
                                    data_ptr->l2_dirty_kickouts++;
                                }
                            }
                            if(current2->tag != 0){data_ptr->l2_kickouts++;}
                            printf("	Need to return data from lower level memory\n");
                            setassocsearch2->address = address;
                            setassocsearch2->tag = address & tagmask2;
                            setassocsearch2->valid = true;
                            if(op == 'W'){
                                setassocsearch2->dirty = true;
                            }else{setassocsearch2->dirty = false;}
				
                        }
                    }
                    current2 = current2->child;
                }

        }
	
}

void l2cachewrite(int l2_assoc, int l2_cache_size, Node *l2_root, unsigned long long int address, Data * data_ptr){
    Node *writesearch = l2_root;
    int l2_index_bits, num_of_sets;
    
	if(l2_assoc == 1){
		l2_index_bits = (log(L2BLOCKSIZE)/log(2)) + (log(l2_cache_size/L2BLOCKSIZE)/log(2));
	}else{if(l2_assoc == 0){
		l2_index_bits = (log(L2BLOCKSIZE)/log(2));
    }else{
        num_of_sets = (l2_cache_size/L1BLOCKSIZE)/l2_assoc;
        l2_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(num_of_sets)/log(2));
    }
	}
	
	unsigned long long int maskliteral = 0xFFFFFFFFFFFF;
	unsigned long long int tagmask2 = (maskliteral << l2_index_bits) & maskliteral;
    
    while (writesearch != NULL) {
        if (writesearch->tag == (address & tagmask2) ) {
            printf("block marked dirty in 2\n");
            writesearch->dirty = true;
        }
        writesearch = writesearch->child;
    }

}




