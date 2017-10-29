
/*  
 *  Name        : light_sensor.c
 *  
 *  
 *  Description : Light Sensor I2C driver that calls
 *                routines from the basic i2c.c, 
 *                and get's luminosity values for
 *                the sensor.
 * 
 * 
 *  Authors     : Ashwath Gundepally, CU, ECEE
 *                Sahana Sadagopan, CU, ECEE
 */                

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include "i2c.h"
#include "light_sensor.h"

#define DEV_ADDRESS         0x39
#define CMD_REG             0x80
#define CONTROL_REG_ADDR    0x00
#define CONTROL_REG_ON      0x03
#define CONTROL_REG_OFF     0x00
#define ID_REG_ADDRESS      0x0A 

int main()
{
    int light_sensor_fd;
    i2c_rc function_rc;


    function_rc=i2c_init(DEV_ADDRESS, &light_sensor_fd);
    if(function_rc!=SUCCESS)
    {
        perror("i2c init failed\n");
        exit(0);
    }

    function_rc=turn_on_light_sensor(light_sensor_fd);

    if(function_rc!=SUCCESS)
    {
        perror("turn light sensor on routine failed\n");
        exit(0);
    }
        
    uint8_t part_no, rev_no;
    function_rc=read_id_reg(light_sensor_fd, &part_no, &rev_no);
    
    if(function_rc!=SUCCESS)
    {    
        perror("read id failure\n");
        exit(0);
    }
    
    printf("part no:%"PRIu8"\nrev no:%"PRIu8"\n", part_no, rev_no);
     
    return 0;
}

i2c_rc read_id_reg(int light_sensor_fd, uint8_t* part_no, uint8_t* rev_no)
{
    /* write to the cmd register with the ID register's address */
    uint8_t cmd_id_reg=(CMD_REG|ID_REG_ADDRESS);
    if(i2c_write(&cmd_id_reg, light_sensor_fd)!=SUCCESS)
        return FAILURE;
        
    /* this read will be directed at the ID register */
    uint8_t id_reg_val;
    if(i2c_read(&id_reg_val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
        
        
    /* the first 4 bits don't form the part number */
    *part_no=(id_reg_val>>4); 
    
    
    /* the last 4 bits don't form the rev number */
    uint8_t rev_no_mask=0x0F;
    *rev_no=(id_reg_val)&(rev_no_mask);

    /* this was successfull */
    return SUCCESS;
}


i2c_rc read_control_reg(int light_sensor_fd, uint8_t* control_reg_byte)
{

    /* write to the cmd register with the control register's address */
    uint8_t cmd_control_reg=(CMD_REG|CONTROL_REG_ADDR);
    
    if(i2c_write(&cmd_control_reg, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* this read will read from the control register */
    if(i2c_read(control_reg_byte, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* return successfully*/
    return SUCCESS; 
}


i2c_rc write_control_reg(int light_sensor_fd, uint8_t* control_reg_byte)
{
    /* write to the cmd register with the control register's address */
    uint8_t cmd_control_reg=(CMD_REG|CONTROL_REG_ADDR);
    if(i2c_write(&cmd_control_reg, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* this write will be directed to the control register */
    if(i2c_read(control_reg_byte, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* return successfully*/
    return SUCCESS; 
}


i2c_rc turn_on_light_sensor(int light_sensor_fd)
{
    uint8_t control_reg_val;
    
    /* read the control register to see if the sensor is on */
    if(read_control_reg(light_sensor_fd, &control_reg_val)!=SUCCESS)
    {
        perror("read to control reg failed in turn-on light sensor\n");
        return FAILURE;
    }

    /* if it's not ON, send a command to turn it ON */
    if(control_reg_val!=CONTROL_REG_ON)
    {
        uint8_t write_byte=CONTROL_REG_ON;
        if(write_control_reg(light_sensor_fd, &write_byte)!=SUCCESS)
        {
            perror("write to control reg failed in turn on light sensor\n");
            return FAILURE;
        }
    }    

    /* return successfully */
    return SUCCESS;
}
