#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "circbuff.h"

circbuff *buffer=NULL;

/**************************************************************************
*   Function - allocate
*   Parameters - double pointer 
*   Returns - status
*   Purpose - Initialises the buffer and the structure in dynamic memory
**************************************************************************/

enum quadratic_status allocate(circbuff **init_buffer){
	buffer=(circbuff*)malloc(sizeof(circbuff));
	printf("Enter the size of your buffer\n");
	scanf("%d",&(buffer->sizebuff));
	uint32_t* size =NULL;
	size= (uint32_t*)malloc(sizeof(uint32_t)*(buffer->sizebuff));
	if(size==0){
		printf("Not allocated any space\n");
		return failure;
	}
	else{
		buffer->head=size;
		buffer->tail=size;
		buffer->buffer=size;
		*init_buffer=buffer;
		return success;
	}
	
}

/**************************************************************************
*   Function - add
*   Parameters - double pointer buffer and elemt to add
*   Returns - status 
*   Purpose - Adds the element to the head of the buffer and it wraps around if the buffer is full
**************************************************************************/

enum quadratic_status add(circbuff **init_buffer, uint32_t additem){
	buffer= *init_buffer;
	circbuff* allocbuffer=buffer;
	bool buffercheck=ls_buff_full(buffer);
	if(buffercheck){
		printf("buffer full wrapping around\n");
		return failure;
	}
	else{
		*buffer->head=additem;
		*init_buffer=allocbuffer;
		(*init_buffer)->head=allocbuffer->head;
		(*init_buffer)->num_elements=buffer->num_elements;
		buffer->head++;	
		buffer->num_elements++;
		
		printf("%d\n",buffer->num_elements );
		
		return success;
	}
}

bool ls_buff_empty(circbuff *buffer)
{
	if(buffer->num_elements<1)
		return true;
	else
		return false;
}


bool ls_buff_full(circbuff *buffer)
{
    if(buffer->num_elements==((buffer->sizebuff)+1))
    {
    	printf("%d\n", buffer->num_elements);
    	printf("%d\n",buffer->sizebuff );
        return true;
    }
    else 
    	return false;
}

enum quadratic_status delete(circbuff **init_buffer){
	bool buffercheck=ls_buff_empty(buffer);
	uint32_t removed=0;
	printf("%p\n",buffer->tail);
	if(buffer->tail!=buffer->buffer){
		if(buffer->num_elements==0){
			printf("The tail is wrapping up, all deleted\n");
			buffer->tail=buffer->buffer;
			return success;
		}
		
	}
	if(buffercheck) {
		printf("list empty, Please add an element before deleting.\n");
		return failure;
	}
	
	else{
		removed= *buffer->tail;
		printf("removed element:%d\n",removed );
		buffer->tail++;
		buffer->num_elements--;
		return success;
	}
}

enum quadratic_status dump(circbuff **init_buffer)
{
	uint32_t* allocbuffer=buffer->tail;
	bool buffercheck=ls_buff_empty(buffer);
	if(buffercheck) {
		printf("list empty, Please add an element before deleting.\n");
		return failure;
	}
	else{
		for(int i=0;i<buffer->num_elements;i++){
			if(buffer->num_elements != ((buffer->sizebuff)+1)){
				printf("The %d buffer element is:%d\n",i,*allocbuffer);
		        allocbuffer++;
		    }
		    else{
		    	printf("The buffer was full so no more element was added");
		    }
		}
		return success;
	}
}


