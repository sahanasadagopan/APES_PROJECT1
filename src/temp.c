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

#include "i2c.h"
#include "temp.h"

#define DEV_ADDR 0X48

/*global variable*/

int file; 

/* Name         :   void signal_handler(int signum)
 * 
 * 
 * Description  :   Closing the file descriptor that opens
 *                  the I2C bus incase of any Signal.
 * 
 * Args         :  int signum: an int value that asserts the argument. 
 * 
 * Returns      :  void: No value is returned
 *                          
 * */

void signal_handler(int signum)
{

  assert(0 == close(file));

  exit(signum);

}

/* Name         :   temp_ret pointer_reg(int reg) 
 * 
 * Description  :   The Pointer register is configured which initialises
 *                  the other registers.
 * 
 * Args         :   int reg: The commmand number for the register. 
 * 
 * Returns      :   SUCCESS_TEMP: When the Configuration done is correct                     
 * 
 *                  FAILURE: When unable to set the register.
 * 
 * */



temp_ret pointer_reg(int reg){

	if(reg == 1){
		uint8_t pointer_reg_conf = POINTER_REG|TEMP_REG_ADDR;
	}
	if(reg == 2){
		uint8_t pointer_reg_conf = POINTER_REG|CONF_REG_ADDR;
	}
	if(reg == 3){
		uint8_t pointer_reg_conf = POINTER_REG|TLOW_REG_ADDR;
	}
	if(reg == 4){
	
	uint8_t pointer_reg_conf = POINTER_REG|THIGH_REG_ADDR;
	}

	return SUCCESS_TEMP;

}

/* Name         :   float set_resolution(char* value) 
 * 
 * Description  :   Sets the resolution of the conversion.
 * 
 * Args         :   char *value: Sets the value to MAX or MIN. 
 * 
 * Returns      :   The value to convert to the correct resolution
 * 
 * */



float set_resolution(char* value){
	float resolution=0;
	pointer_reg(2);
	printf("value of input %s\n",value);
	if(value == "max"){
		printf("got value\n");
		uint16_t CONF_REG = CONF_REG_MAX_RESOLUTION;
		resolution = 0.0625;
		printf("now res is %f\n", resolution);
	}
	if(value == "min"){
		uint16_t CONF_REG = CONF_REG_MAX_RESOLUTION;
		resolution = 0.5;
	}
	printf("resolution in func %f\n",resolution);
	return resolution;
	
}

void temp_register(float resolution){
	unsigned char MSB, LSB;
	float temp;
	float f,c;
	pointer_reg(1);
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	while(1)
   	{
		if((i2c_read(buf))!=SUCCESS){
			perror("Reading error\n");
         	}
	//printf("buf value %d",buf);
     // Using I2C Read 
		else {
			printf("buf value %d",buf);
       			MSB = buf[0];
       			LSB = buf[1];

       	/* Convert 12bit int using two's compliment */
        /* Credit: http://bildr.org/2011/01/tmp102-arduino/ */
       			temp = ((MSB << 8) | LSB) >> 4;

       			c = temp*resolution;
       			f = (1.8 * c) + 32;

       			printf("Temp Fahrenheit: %f Celsius: %f\n", f, c);
     			}
	}
	
}
int main(){

   
  char *bus = "/dev/i2c-2"; /* Pins P9_19 and P9_20 */
  int addr = 0x48;          /* The I2C address of TMP102 */
  //char buf[2] = {0};
  float resolution=0.00;
  i2c_init(DEV_ADDR);
 /* Register the signal handler */
 signal(SIGINT, signal_handler);
 resolution=set_resolution("max");
 printf("resolution %f\n", resolution);
 temp_register(resolution);
 sleep(1);
}
