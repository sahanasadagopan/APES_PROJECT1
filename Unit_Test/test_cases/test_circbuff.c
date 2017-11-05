#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "circbuff.h"

uint32_t size_buff=12;
/**************************************************************************
*   Function - allocate
*  This fumction checks if the allocation is done properly
**************************************************************************/
void test_circbuf_allocate(void **state)
{
	circbuff *test_buff = (circbuff *) malloc(sizeof(circbuff));
	(test_buff->sizebuff) = size_buff;
	enum quadratic_status status = allocate(&test_buff);
	assert_int_equal(status,success);
	free(test_buff);
	test_buff->head=NULL;
	test_buff->tail=NULL;	
}

/**************************************************************************
*   Function - adding and deleting
*   This function checks the add and delete element function
*   of the circularbuff. The item is add to the Head and the 
*   item is deleted from the tail
**************************************************************************/

void test_circbuf_add_remove(void **state)
{
	circbuff *temp = (circbuff *) malloc(sizeof(circbuff));
	allocate(&temp);
	uint32_t data=5;
	enum quadratic_status status;
	status = add(&temp,data);
	assert_int_equal(status,success);
	data=8;
	status = add (&temp,data);
	assert_int_equal(status,success);
	status = delete(&temp);
	assert_int_equal(status,success);
	free(temp);
	temp->head=NULL;
	temp->tail=NULL;	

}

/**************************************************************************
*   Function - remove element
*   This function is when the user tries to removeans
*   elemeent in an empty buffer . This test case would fail
**************************************************************************/

void test_circbuff_empty_remove(void **state){

	circbuff *temp = (circbuff *) malloc(sizeof(circbuff));
	allocate(&temp);
	enum quadratic_status status;
	status = delete(&temp);
	assert_int_equal(status,success);
	free(temp);
	temp->head=NULL;
	temp->tail=NULL;	

}

/**************************************************************************
*   Function - add
*   This function test the buffer full and add function and it would show
*   show a statement if the buffer is full and item cannot be added more.
**************************************************************************/

void test_circbuff_add_full_wrap(void **state){
	circbuff *temp = (circbuff *) malloc(sizeof(circbuff));
	allocate(&temp);
	enum quadratic_status status;
	uint32_t data=3;
	status = add(&temp,data);
	assert_int_equal(status,success);
	if(ls_buff_full(temp)){
		status = failure;
	}
	assert_int_equal(status,success);
	data=7;
	status = add(&temp,data);
	if(ls_buff_full(temp)){
		status = failure;
	}
	assert_int_equal(status,success);
	free(temp);
	temp->head=NULL;
	temp->tail=NULL;
}
/**************************************************************************
*   Function - allocate
*   This is the negative condition of allocate function it fails when
*   size of the pointer is NULL
**************************************************************************/

void test_circbuff_NULLptr(void **state){
	circbuff *temp = NULL;
	enum quadratic_status status;
	printf("Enter the buffer size as 0\n");
	status = allocate(&temp);
	assert_int_not_equal(status,success);
	free(temp);
	temp->head=NULL;
	temp->tail=NULL;
}

/**************************************************************************
*   Function -ls_buffer_full
*   This is the case where a NULL pointer is passed to the check confiton
*   this is a fucntion that is expected to fail
**************************************************************************/

void test_circbuff_invalidptr(void **state){
	enum quadratic_status status = ls_buff_full(NULL);
	assert_int_equal(status,success);

}

/**************************************************************************
*   Function - tail wrap around
*   This test case checks if the tail wraps around
*   when all the items are deleted from a buffer
**************************************************************************/

void test_circbuff_remove_wrap(void **state){
	circbuff *temp = (circbuff *) malloc(sizeof(circbuff));
	allocate(&temp);
	temp->sizebuff=3;
	enum quadratic_status status;
	uint32_t data=3;
	status = add(&temp,data);
	assert_int_equal(status,success);
	data=5;
	status = add(&temp,data);
	assert_int_equal(status,success);
	data=7;
	status = add(&temp,data);
	assert_int_equal(status,success);
	status = delete(&temp);
	assert_int_equal(status,success);
	if(ls_buff_empty(temp)){
		assert_int_equal(status,success);
	}
	status = delete(&temp);
	assert_int_equal(status,success);
	if(ls_buff_empty(temp)){
		assert_int_equal(status,success);
	}
	status = delete(&temp);
	assert_int_equal(status,success);
	if(ls_buff_empty(temp)){
		assert_int_equal(status,success);
	}
	status = delete(&temp);
	assert_int_equal(status,success);
	free(temp);
	temp->head=NULL;
	temp->tail=NULL;
}

/**************************************************************************
*   Function - printfunction
*  Checks the print function of the circular buffer
**************************************************************************/

void test_circbuff_print(void **state){
	circbuff *temp = (circbuff *) malloc(sizeof(circbuff));
	allocate(&temp);
	temp->sizebuff=3;
	enum quadratic_status status;
	uint32_t data=3;
	status = add(&temp,data);
	assert_int_equal(status,success);
	data=5;
	status = add(&temp,data);
	assert_int_equal(status,success);
	data=7;
	status = add(&temp,data);
	assert_int_equal(status,success);
	status = dump(&temp);
	free(temp);
	temp->head=NULL;
	temp->tail=NULL;

}

int main(int argc, char **argv)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_circbuf_allocate),
    cmocka_unit_test(test_circbuf_add_remove),
    cmocka_unit_test(test_circbuff_add_full_wrap),
    cmocka_unit_test(test_circbuff_NULLptr),
    cmocka_unit_test(test_circbuff_empty_remove),
    cmocka_unit_test(test_circbuff_invalidptr),
    cmocka_unit_test(test_circbuff_remove_wrap),
    cmocka_unit_test(test_circbuff_print)
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}

