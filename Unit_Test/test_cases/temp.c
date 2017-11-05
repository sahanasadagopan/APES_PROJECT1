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

#include "i2c.h"
#include "temp.h"


//temp_ret read_register(int temp_fd, uint8_t pointer_reg_conf);
//temp_ret write_register(int temp_fd,uint8_t pointer_reg_conf,uint16_t* value);
#define DEV_ADDR 0X48 

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



temp_ret pointer_reg(int reg,int temp_fd){
	if(reg == 1){
		uint8_t pointer_reg_conf = POINTER_REG|TEMP_REG_ADDR;
		i2c_write(&pointer_reg_conf,temp_fd);
		return SUCCESS_TEMP;
	}
	if(reg == 2){
		
		uint8_t pointer_reg_conf = POINTER_REG|CONF_REG_ADDR;
		read_register(temp_fd,pointer_reg_conf);
		return SUCCESS_TEMP;
		
	}
	if(reg == 3){
		uint8_t pointer_reg_conf = POINTER_REG|TLOW_REG_ADDR;
		read_register(temp_fd,pointer_reg_conf);
		return SUCCESS_TEMP;

	}
	if(reg == 4){
		uint8_t pointer_reg_conf = POINTER_REG|THIGH_REG_ADDR;
		read_register(temp_fd,pointer_reg_conf);
		return SUCCESS_TEMP;
	}
	if(reg>4){
		return FAILURE_TEMP;
	}

	

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



float set_resolution(char* value,int temp_fd){
	float resolution=0;
	pointer_reg(2,temp_fd);
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
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

temp_ret read_register(int temp_fd, uint8_t pointer_reg_conf){
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	if(buf== NULL){
		return FAILURE_TEMP;
	}
	i2c_write(&pointer_reg_conf,temp_fd);
	i2c_read_word(buf,temp_fd);
	printf("resolution set 1st bit%x\n",buf[0]);
	printf("resol 2nd bit %x\n ", buf[1]);	
	free(buf);
	return SUCCESS_TEMP;

}

temp_ret write_register(int temp_fd,uint8_t pointer_reg_conf,uint16_t* value){

	uint8_t *buf = malloc(sizeof(uint8_t)*3);
	if(buf== NULL){
		return FAILURE_TEMP;
	}
	//i2c_write(&pointer_reg_conf,temp_fd);
	//printf("The register written %x \n",pointer_reg_conf);
	buf[0]=pointer_reg_conf;
	*((uint16_t*)&buf[1])=*value;
	i2c_write_word(&buf[0],temp_fd);
	//printf("value to the reg %x \n",value);
	free(buf);
	return SUCCESS_TEMP

}

temp_ret conf_reg_read(int temp_fd){
	uint8_t conf_reg_addr = CONF_REG_ADDR;
	read_register(temp_fd,conf_reg_addr);
}

temp_ret conf_reg_write(int temp_fd){
	uint8_t conf_reg_addr = CONF_REG_ADDR;
	uint16_t conf_reg_value = CONF_REG_MAX_RESOLUTION;
	write_register(temp_fd,conf_reg_addr,&conf_reg_value);



}
temp_ret conf_reg_interruptmode(int temp_fd){
	uint8_t conf_reg_addr = CONF_REG_ADDR;
	uint16_t conf_reg_value = CONF_REG_MAX_RESOLUTION|CONF_REG_INTERRUPT;
	write_register(temp_fd,conf_reg_addr,&conf_reg_value);

}

temp_ret conf_reg_sd(int temp_fd){
	uint8_t conf_reg_addr = CONF_REG_ADDR;
	uint16_t conf_reg_value = CONF_REG_MAX_RESOLUTION|CONF_REG_SHUTDOWN;
	write_register(temp_fd,conf_reg_addr,&conf_reg_value);
}

temp_ret temperature_C(int temp_fd,float c){
	float f=0.0,k;
	float resolution=set_resolution("max",temp_fd);
	temp_register(resolution,temp_fd,&c,&f,&k);
	return SUCCESS_TEMP;
}

/*temp_ret temperature_F(int temp_fd,float f){
	float c=0.0,k;
	float resolution=set_resolution("max",temp_fd);
	temp_register(resolution,temp_fd,c,f,k);
}
temp_ret temperature_K(int temp_fd,float k){
	float c,f;
	float resolution=set_resolution("max",temp_fd);
	temp_register(resolution,temp_fd,c,f,k);
}*/

temp_ret temp_register(float resolution,int temp_fd,float *f,float *c,float *k){
	unsigned char MSB, LSB;
	float temp;
	
	pointer_reg(1,temp_fd);
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	int count =0;
//	while(count <10)
//   	{
		if((i2c_read(buf,temp_fd))!=SUCCESS){
			perror("Reading error\n");
         	}
	//printf("buf value %d",buf);
     // Using I2C Read 
		else {
			printf("buf value %x\n",buf[0]);
			printf("buf value %x\n",buf[1]);
       			MSB = buf[0];
       			LSB = buf[1];

       	/* Convert 12bit int using two's compliment */
        /* Credit: http://bildr.org/2011/01/tmp102-arduino/ */
       			temp = ((MSB << 8) | LSB) >> 4;

       			*c = temp*resolution;
       			*f = (1.8 * (*c)) + 32;
			*k = (*c)+273;
       			printf("Temp Fahrenheit: %f Celsius: %f\n kevin:%f\n", *f, *c,*k);
     			}
		count++;
//	}
	free(buf);
	
}
temp_ret tlow_reg_read(int temp_fd){
	uint8_t tlow_reg_addr = TLOW_REG_ADDR;
	read_register(temp_fd,tlow_reg_addr);
	

}

temp_ret tlow_reg_write(int temp_fd,uint16_t value){
	uint8_t tlow_reg_addr = TLOW_REG_ADDR;
	write_register(temp_fd,tlow_reg_addr,&value);

}

temp_ret thigh_reg_write(int temp_fd,uint16_t value){
	uint8_t thigh_reg_addr = THIGH_REG_ADDR;
	write_register(temp_fd,thigh_reg_addr,&value);

}


temp_ret thigh_reg_read(int temp_fd){
	uint8_t thigh_reg_addr = THIGH_REG_ADDR;
	read_register(temp_fd,thigh_reg_addr);
}



int main(){

   int temp_fd;
  char *bus = "/dev/i2c-2"; /* Pins P9_19 and P9_20 */
  uint16_t lvalue = 0x0a;
  uint16_t hvalue = 0x23;
  float resolution=0.00,celsius=0,fahreheit=0,kelvin=0;
  i2c_init(DEV_ADDR,&temp_fd);
  /*Register the signal handler */
 resolution=set_resolution("max",temp_fd);
 printf("resolution %f\n", resolution);
 temp_register(resolution,temp_fd,&celsius,&fahreheit,&kelvin);
 tlow_reg_write(temp_fd,lvalue);
 thigh_reg_write(temp_fd,hvalue);
 thigh_reg_read(temp_fd);
 tlow_reg_read(temp_fd);
 conf_reg_interruptmode(temp_fd);
 conf_reg_read(temp_fd);
 //signal(SIGINT, signal_handler);
 sleep(1);
}
