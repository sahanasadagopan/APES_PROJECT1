#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <inttypes.h>

#include "temp.c"
#include "i2c.h"

/*Test case Should Pass for POINTER REGISTER*/
void test_pointer_reg(void **state){
	int temp_fd; //File pointer for I2C
	enum temp_ret status =pointer_reg(1,temp_fd);
	assert(status,SUCCESS_TEMP);
}

/*Test Case should fail for POINTER REGISTER*/
void test_pointer_reg_negative(void **state){
	int temp_fd;
	enum temp_ret status = pointer_reg(5,temp_fd);
	assert(status,FAILURE_TEMP);
}

/*Test Case should pass for READ_REGISTER*/

void test_read_register(void **state){
	int temp_fd;
	uint8_t pointer_reg_conf = CONF_REG_ADDR;
	enum temp_ret status = read_register(temp_fd,pointer_reg_conf)
	assert(status,SUCCESS_TEMP);
}

/*Test Case shoudl pass for WRITE_REGISTER*/

void test_write_register(void **state){
	int temp_fd;
	uint8_t pointer_reg_conf = CONF_REG_ADDR;
	uint16_t conf_reg_value = CONF_REG_MAX_RESOLUTION;
	enum temp_ret status = write_register(temp_fd,pointer_reg_conf,&conf_reg_value);
	assert(status,SUCCESS_TEMP);

}

/*Test case Should pass for TEMPERATURE_C*/

void test_temperature_C(void **state){
	int temp_fd;
	float c;
	enum temp_ret status =  temperature_C(temp_fd,c);
	assert(status,SUCCESS_TEMP);
}

int main(int argc, char **argv)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_pointer_reg),
    cmocka_unit_test(test_pointer_reg_negative),
    cmocka_unit_test(test_read_register),
    cmocka_unit_test(test_write_register),
    cmocka_unit_test(test_temperature_C),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}