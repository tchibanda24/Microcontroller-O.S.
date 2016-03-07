/*
 * myMem.c
 *
 * Routines for memory allocation(malloc, free, mmap)
 *
 *  Created on: Sept 24, 2015
 *      Author: Thabani Chibanda
 */

_EWL_IMP_EXP_C int _EWL_CDECL fprintf(FILE *_EWL_RESTRICT stream, const char *_EWL_RESTRICT format, ...) _EWL_CANT_THROW;

#define myMem_DISABLE 1
#include "myMem.h"
#include "derivative.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pushbutton.h>
#include <led.h>
#include <delay.h>
#include <mcg.h>
#include <sdram.h>
#include <uart.h>

#define h_top 0
#define h_bottom 1
#define HEADER_SZ sizeof(block);

typedef uint32_t uintptr_t;
extern struct console console;

//struct for block, bit field used to keep it double word
typedef struct block {
	unsigned int size;
	void *next;
	uint8_t p_id:7;
} block;

static block *myheap[2] = {NULL, NULL};
static block *p_top = NULL, *p_bottom = NULL;                   //top and bottom blocks of the processes LL
static block *s_top = NULL;                         			//top block of the free space LL

static unsigned int total_free = SDRAM_SIZE;                    //total amount of free memory

// helper function declarations, implementations below
static void *remove_slist(unsigned int size);
static void add_slist(block *freed);
static int remove_plist(block *freed);
static int coalesce(block **freed);
static void reorder (block **freed, block **prev);
static int get_pid();

//process id data type
extern pid_t pid();

/*  **********************************              Malloc Functions            ***************************************
 ***************************************************************************************************************  */

//8 byte aligned formula
//myheap = (void*)(((uintptr_t)mem+8) & ~ (uintptr_t)0x07); //forces 8-byte align

/*myMalloc(): malloc implementation that accepts an integer, and returns NULL or a pointer to the top of the
            memory space thaat has been requested.

    It will check for free memory to allocate, or allocate new memory.
 */
void *myMalloc(unsigned int sz){
	//if: if the heap is NULL, initialize the heap
	if(myheap[h_top] == NULL) {
		myheap[h_top] = (void*)((char*)SDRAM_START + LCDC_FRAME_BUFFER_SIZE);
		myheap[h_bottom] = (void*)SDRAM_END;
		(myheap[h_top])->size = total_free;
		(myheap[h_top])->next = NULL;
		s_top = myheap[h_top];
	}

	if((sz < 1) || sz > total_free) return NULL;

	unsigned int f_sz = ((sz + (unsigned int)sizeof(block)) + 7) & ~7;    	//full size of a block(sz + bookkeeping)
	void *request = NULL;                              	 		            //ptr to the requested block space
	block *new_block = NULL;                            		            //new block's bookkeeping
	request = remove_slist(f_sz);                       		            //check for a free block of memory

	//if: if a space was found for the requested memory, initialize @new_block and set @request for return
	if(request) {

		new_block = (block*)request;      //data for memory allocated
		request = (block*)request + 1;    //ptr to be returned

		new_block->size = (f_sz);
		new_block->p_id = 0;
		new_block->next = NULL;

		//add to the LL
		if(!p_top) {
			p_top = new_block;
			p_bottom = new_block;
		}
		else {
			p_bottom->next = new_block;
			p_bottom = new_block;
		}
	}

	return request;
}

/*myFree(): Accepts a pointer and frees it from the heap.
            Moves the memory from the process LL to the free spaces LL
 */
void myFree(void *ptr) {

	if(!ptr) return;

	block* f_request = (block*)ptr - 1;            //retreive the metadata of the memory block being freed

	int free_check = remove_plist(f_request);

    //if the block was succesfully removed, add it to the LL of free spaces
	if(free_check == 1) {
		add_slist(f_request);
		total_free += f_request->size;
	}

	return;
}

/*myMemoryMap(): Prints out the addresses of all malloc'd memory(includes the size and process id)
 */
void myMemoryMap(){

	block *curr = p_top;
	void *addr;

	char * out = malloc(60);
	sprintf(out, "\nTOTAL FREE SPACE: %d\r\n", total_free);
	uartPuts(UART2_BASE_PTR, out);
	lcdcConsolePuts(&console, out);

    //print the metadata of all the blocks in the allocated memory LL
	while(curr){
		addr = (void*)(curr + 1);

		sprintf(out, "Proc: %d - Size: %d - Address : %p\r\n", curr->p_id, curr->size, addr);
		uartPuts(UART2_BASE_PTR, out);
		lcdcConsolePuts(&console, out);

		curr = curr->next;
	}

	uartPuts(UART2_BASE_PTR, "FINISH MAP\r\n");
	lcdcConsolePuts(&console,  "FINISH MAP\r\n");

	free(out);
}


/*  ************************************            Helper Functions          *************************************
 ***************************************************************************************************************  */


/*remove_slist: Accepts an integer from malloc and serches for the frist free space large enough from the free LL.
                Also splits this memory if it is signicantly larger than the requested amount.

    Returns a pointer to the free space, and removes the block from the free LL.
    Return NULL if there's no space large enough
 */
static void *remove_slist(unsigned int size) {

	block *prev = NULL;     //block for tracking the previously checked block
	block *curr = s_top;    //block currently being checked(and to be returned)

	while(curr) {

		//if: if the current space is not large enough, increment @curr and @prev
		//else: set @curr to point to the available block
		if((unsigned int)(curr->size) < size){
			prev = curr;
			curr = curr->next;
		}
		else {

			//Check if the user is trying to allocate past the end of SRAM, return NULL if so
			if(((uintptr_t*)curr + size) > (uintptr_t*)SDRAM_END)
				return NULL;

			//if: if the block is has at least a double word length of extra space, split it before returning
			//else: remove @curr from the LL for return
			if((unsigned int)(curr->size) >= (size + 16)) {

				unsigned int new_size = curr->size - size;

				//create a new block, and pass it the data of @curr. Update sizes
				block *new_space = (block*)((char*)curr + size);
				*new_space = *curr;

				curr->size = size;
				new_space->size = new_size;

				//replace @curr with @new_space in the LL
				if(s_top == curr) {
					s_top = new_space;
					new_space->next = curr->next;
				}
				else {
					prev->next = new_space;
				}
			}
			else {
				if(s_top == curr)
					s_top = curr->next;
				else
					prev->next = curr->next;
			}

			total_free = total_free - curr->size;
			return curr;
		}
	}

	return NULL;
}

/*remove_plist: Accepts a pointer to malloc'd memory to be freed and removes it from the processer LL.
                Uses a previous and current block to track the list, and remove the block.

    Returns a 1 on success and a -1 if it fails.
 */
static int remove_plist(block *freed) {

	block *prev = NULL;
	block *curr = p_top;

	if(get_pid() != freed->p_id) return -1;

	//iterate through the list until the block is found, then remove it
	while(curr) {

		if(curr != freed){
			prev = curr;
			curr = curr->next;
		}
		else {
			//conditions for accurate list removal
			if ((curr == p_top) && (curr == p_bottom)) {
				p_top = curr->next;
				p_bottom = prev;
			}
			else if(curr == p_top) {
				p_top = curr->next;
			}
			else if(curr == p_bottom) {
				prev->next = NULL;
				p_bottom = prev;
			}
			else {
				prev->next = curr->next;
			}

			return 1;
		}
	}

	return -1;
}

/*add_slist: Adds a free space block to the free space LL that has been freed from memory.
            Also uses a current and previous variable for tracking LL iterations.
            Has a function for coalescing a space to be freed. Does it as memory is freed.
 */
static void add_slist(block *freed) {
	//if the free list is empty, make @freed the header and exit
	if(s_top == NULL) {
		s_top = freed;
		freed->next = NULL;
		return;
	}

	block *prev = NULL;
	block *curr = s_top;

	//attempt a coalesce first, if successful exit
	int coal_check = coalesce(&freed);
	if(coal_check == 1) return;

	//loop for iterating through rest of LL when coalesce fails, sorts LL from smallest to largest space using
    // if statements to slide blocks down the list into their correct position
	while(curr) {

		if(curr->size < freed->size) {
			if(!curr->next) {
				curr->next = freed;
				freed->next = NULL;
				return;
			}

			prev = curr;
			curr = curr->next;
		}
		else {
			if(curr == s_top) {
				freed->next = curr;
				s_top = freed;
			}
			else {
				freed->next = curr;
				prev->next = freed;
			}

			return;
		}
	}
	return;
}


/*coalesce: Coalesces any space adjacent to the one being freed.
            Iterates through the list once, saves adjacent spaces then puts them together.
 */
static int coalesce(block **freed) {

	int check = -1;
	block *prev = NULL;
	block *curr = s_top;
	block *hold_curr[2] = {NULL, NULL};     //keeps track of adjacent spaces
	block *hold_prev[2] = {NULL, NULL};     //keeps track of previous blocks to adjacent spaces

	while(curr) {
		//check the block above, save it if adjacent to the current block
		if((char*)(*freed) == ((char*)curr + (curr->size))) {
			hold_curr[0] = curr;
			hold_prev[0] = prev;
			check = 1;
		}
		//check the block below, save it if adjacent to the current block
		else if((char*)(*freed) == ((char*)curr - ((*freed)->size))) {
			hold_curr[1] = curr;
			hold_prev[1] = prev;
			check = 1;
		}

		prev = curr;
		curr = curr->next;
	}

	//if: the block below is adjacent, merge it with free and add free to the LL
	if(hold_curr[1]) {
		if(hold_curr[1] == s_top) s_top = *freed;

		(*freed)->size = ((*freed)->size) + (hold_curr[1])->size;
		(*freed)->next = (hold_curr[1])->next;

		if(hold_prev[1]) hold_prev[1]->next = (*freed);

		if((*freed)->next) reorder(freed, &(hold_prev[1]));
	}

	//if: the block above is adjacent, merge it with free and reorder the LL
	if(hold_curr[0] != NULL) {
		(hold_curr[0])->size = ((*freed)->size) + (hold_curr[0])->size;
		(*freed) = hold_curr[0];

		if(hold_prev[0]) (hold_prev[0])->next = (*freed)->next;

		if((*freed)->next) reorder(freed, &(hold_prev[0]));
	}

	return check;
}

//reorder: function for reorder the free LL after blocks are coalesced
static void reorder (block **freed, block **prev){
	//loop for reaordering the list, while @freed has a next we ccheck it
	while((*freed)->next) {
		block *temp = ((block*)((*freed)->next));   //block for holding the position after free

		//if the size of the freed block is greater than the next one, slide down
		if((*freed)->size > temp->size) {
			//if: if freed is the top, make the next block the top before moving down
            //    (protects from NULL previous node as well).
			//else: we know there's a previous node so just update pointers to slide down
			if((*freed) == s_top){
				s_top = temp;
				(*freed)->next = temp->next;
				temp->next = (*freed);
				(*prev) = temp;
			}
			else{
				(*prev)->next = temp;
				(*freed)->next = temp->next;
				temp->next = (*freed);
			}
		}
		else {
			break;
		}
	}
}

//get_pid: function for retrieving calling process id
static int get_pid(){
	return pid();
}


/*  ///////////////////////////////////////////////////////////////////////////////////////////////////////
 ******************************************************************************************************
                                                TESTING
 *******************************************************************************************************    */

/*
int main(int argc, char *argv[])
{
    fprintf(stdout, "\n 1) Before malloc\n");
    myMemoryMap();

    int *int_1 = myMalloc(sizeof(int*));
    char *char_1 = myMalloc(sizeof(char*));
    int *int_p = myMalloc(sizeof(int*));
    char *char_p = myMalloc(sizeof(char*));
    int *int_2 = myMalloc(sizeof(int*));
    char *char_2 = myMalloc(sizeof(char*));
    void *newguy = myMalloc(13);

    fprintf(stdout, " 2) After malloc\n");
    myMemoryMap();

    myFree(int_1);
    myFree(int_2);
    myFree(char_p);
    myFree(int_p);

    fprintf(stdout, " 3) After free\n");
    myMemoryMap();

    int *nobody = myMalloc(0);
    int *int_3 = myMalloc(sizeof(int*));
    char *char_3 = myMalloc(sizeof(char*));
    int *int_p2 = myMalloc(sizeof(int*));
    char *char_p2 = myMalloc(sizeof(char*));

    fprintf(stdout, " 4) After more malloc\n");
    myMemoryMap();

    myFree(int_p);
    myFree(char_p2);

    fprintf(stdout, " 5) After more free\n");
    myMemoryMap();

    fprintf(stdout, "%lu %lu %lu %lu ", sizeof(int*), sizeof(char*), sizeof(block), sizeof(uint8_t));

    return 0;
}*/
