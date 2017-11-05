#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mqueue.h>
#include <semaphore.h>
#include <inttypes.h>

#include "../includes/log.h"


/**************************************************************************
*   Function - heartbeatapi
*  This function checks if the allocation is done properly
**************************************************************************/
void test_heatbeat(void **state)
{
	log_message_t hb_message;
	enum message_rc status =heartbeat_api("task_new",&hb_message);
	assert_int_equal(status,MSGRC_SUCCESS);
}

/**************************************************************************
*   Function - adding and deleting
*   This function checks the add and delete element function
*   of the circularbuff. The item is add to the Head and the 
*   item is deleted from the tail
**************************************************************************/

void test_lux_change_log(void **state)
{
	float luminosity =2;
	float luminosity_old = 100,change_lux;
	enum if_log_reqd_t status = if_lux_change_log(luminosity_old,luminosity,&change_lux)	
	assert_int_equal(status,CHANGE_LOG_REQUIRED);
}

/**************************************************************************
*   Function - remove element
*   This function is when the user tries to removeans
*   elemeent in an empty buffer . This test case would fail
**************************************************************************/

void test_log_ls_id(void **state){
	float value = 34.6
	log_message_t light_message;
	enum message_rc status =log_ls_id(value,&light_message);
	assert_int_equal(status,MSGRC_SUCCESS);
		

}

void test_req_res(void **state){
	log_message_t log_id;
	enum message_rc status = task_req_res(&log_id,"temperature","request");
	assert_int_equal(status,MSGRC_SUCCESS);
}

int main(int argc, char **argv)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_heatbeat),
    cmocka_unit_test(test_lux_change_log),
    cmocka_unit_test(test_req_res),
    cmocka_unit_test(test_log_ls_id)
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
