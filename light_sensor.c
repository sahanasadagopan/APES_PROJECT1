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
        
    write_timing_reg(light_sensor_fd, MIN_GAIN, MODERATE_INTEG_TIME, 0); 

    gain_t adc_gain;
    integ_time_t integ_time;
    uint8_t if_manual;

    read_timing_reg(light_sensor_fd, &adc_gain, &integ_time, &if_manual); 
    

    printf("\n\nadc_gain:%d, integ_time:%d, if_manual:%d\n\n", adc_gain, integ_time, if_manual);   
        
    return 0;
}

i2c_rc read_id_reg(int light_sensor_fd, uint8_t* part_no, uint8_t* rev_no)
{
    uint8_t id_reg_val;
    
    if(light_sensor_read_reg(light_sensor_fd, ID_REG_ADDRESS, &id_reg_val)!=SUCCESS)
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

    if(light_sensor_read_reg(light_sensor_fd, CONTROL_REG_ADDR, control_reg_byte)!=SUCCESS)
        return FAILURE;

    /* return successfully*/
    return SUCCESS; 
}


i2c_rc write_control_reg(int light_sensor_fd, uint8_t* control_reg_byte)
{
    
    if(light_sensor_write_reg(light_sensor_fd, CONTROL_REG_ADDR, control_reg_byte)!=SUCCESS)
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

/* write to the timing register 
 * this also supports manual timing */
i2c_rc write_timing_reg(int light_sensor_fd,  gain_t adc_gain, integ_time_t integ_time, uint8_t if_manual)
{
    /* check if manual timing is requested */ 
    if(integ_time==INTEG_NA)
    {
        /* create the byte to write using the ADC gain and manual timing val(1/0 to start/stop respectively )*/
        uint8_t timing_reg_val=(ADC_GAIN_MASK & adc_gain)|(MANUAL_TIMING_MASK & if_manual);
        /* write byte to the timing register*/        
        if(light_sensor_write_reg(light_sensor_fd, TIMING_REG_ADDR, &timing_reg_val)!=SUCCESS)
            return FAILURE;
        /* return successfully*/    
        return SUCCESS;
    }

    /* create the byte to write using the ADC gain and integration time val(MIN:MODERATE:MAX::0:1:2)*/
    uint8_t timing_reg_val=(ADC_GAIN_MASK & adc_gain) | (INTEG_TIME_MASK & integ_time);
    
    /* write the byte to the timing register */ 
    if(light_sensor_write_reg(light_sensor_fd, TIMING_REG_ADDR, &timing_reg_val)!=SUCCESS)
        return FAILURE;
    
    /* return successfully*/
    return SUCCESS;
}

/* read the timing register */
i2c_rc read_timing_reg(int light_sensor_fd,  gain_t* adc_gain, integ_time_t* integ_time, uint8_t* if_manual)
{
    /* first read the register in a uint8_t type */ 
    uint8_t timing_reg_val;
    if(light_sensor_read_reg(light_sensor_fd, TIMING_REG_ADDR, &timing_reg_val)!=SUCCESS)
    {
        return FAILURE;
    }
    
    /* now get the value of all the params */
    *adc_gain=(gain_t)(ADC_GAIN_MASK & timing_reg_val);
    
    *integ_time=(integ_time_t)(INTEG_TIME_MASK & timing_reg_val);
    
    *if_manual=(MANUAL_TIMING_MASK & timing_reg_val); 



    /* return successfully*/
    return SUCCESS;
}

i2c_rc light_sensor_write_reg(int light_sensor_fd, uint8_t reg_addr, uint8_t* val)
{
    uint8_t cmd_reg_val=CMD_REG|reg_addr;
    
    if(i2c_write(&cmd_reg_val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    if(i2c_write(val, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    return SUCCESS;
}


i2c_rc light_sensor_read_reg(int light_sensor_fd, uint8_t reg_addr, uint8_t* val)
{
    uint8_t cmd_reg_val=CMD_REG|reg_addr;
    
    if(i2c_write(&cmd_reg_val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    if(i2c_read(val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    return SUCCESS;
}
