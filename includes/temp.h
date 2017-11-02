/*  
 *  Name        : i2c.c
 *  
 *  
 *  Description : I2C driver that interacts with the on board 
 *                I2C bus (bus 1).
 *                Contains routines like i2c_init(),
 *                i2c_read(), i2c_write().
 *  
 *  Authors     : Ashwath Gundepally, CU, ECEE
 *                Sahana Sadagopan, CU, ECEE
 */     

#ifndef TEMP_H
#define TEMP_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <inttypes.h>

#define POINTER_REG 0X00

#define TEMP_REG_ADDR 0X00
#define CONF_REG_ADDR 0X01
#define TLOW_REG_ADDR 0X02
#define THIGH_REG_ADDR 0x03

typedef enum {SUCCESS_TEMP,FAILURE_TEMP}temp_ret;
/*Configuration registure*/
#define CONF_REG_OS 0X8000
#define CONF_REG_MAX_RESOLUTION 0X600A
#define CONF_REG_MAX_S_RESOLUTION 0X400A
#define CONF_REG_MIN_RESOLUTION 0x000A
#define CONF_REG_MIN_S_RESOLTUION 0X0200A
#define CONF_REG_EXTENDED_MODE 0X000B

temp_ret thigh_reg_read(int temp_fd);
temp_ret thigh_reg_write(int temp_fd,uint16_t value);
temp_ret tlow_reg_write(int temp_fd,uint16_t value);
temp_ret tlow_reg_read(int temp_fd);
temp_ret temp_register(float resolution,int temp_fd);
temp_ret write_register(int temp_fd,uint8_t pointer_reg_conf,uint16_t* value);
temp_ret read_register(int temp_fd, uint8_t pointer_reg_conf);
float set_resolution(char* value,int temp_fd);
temp_ret pointer_reg(int reg,int temp_fd);
temp_ret conf_reg_read(int temp_fd);
void signal_handler(int signum);

#endif