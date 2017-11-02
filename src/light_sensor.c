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
#include <math.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "../includes/i2c.h"
#include "../includes/light_sensor.h"

//#define TEST_APIS

#ifdef  TEST_APIS
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

//    read/write word testing:

    uint16_t thresh_low=400;
    printf("thresh written:%"PRIu16"\n", thresh_low);
     
    light_sensor_write_thresh_low_reg(light_sensor_fd, &thresh_low);
    
    uint16_t thresh_read=0;
    
    light_sensor_read_thresh_low_reg(light_sensor_fd, &thresh_read);
    printf("thresh read:%"PRIu16"\n", thresh_read);
      
    
    return 0;
}
#endif
/* read's the id register and returns the part no and rev no via 
 * a ptr*/
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

/* read's the control register */
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

/* read ADC values for both channels and return them via ptrs to uint16 type 
 * Follow the right protocol, i.e., to read the lower byte before reading the
 * upper byte of the channel */
i2c_rc read_adc(int light_sensor_fd, uint16_t* ch0_adc_val, uint16_t* ch1_adc_val)
{
    /* get data from channel 0 first */
    /* read lower byte */
    uint8_t lower_byte_ch0;
    if(light_sensor_read_reg(light_sensor_fd, LOWER_BYTE_CH0_ADDR, &lower_byte_ch0)!=SUCCESS)
        return FAILURE;

    /* read upper byte */
    uint8_t upper_byte_ch0;
    if(light_sensor_read_reg(light_sensor_fd, UPPER_BYTE_CH0_ADDR, &upper_byte_ch0)!=SUCCESS)
        return FAILURE;
    
    /* cast the upper byte to a uint16_t type, then do an 8-bit 
     * shift, an OR with the lower byte will result in the required
     * 16-bit value */
    *ch0_adc_val=(((uint16_t)upper_byte_ch0)<<8) | (lower_byte_ch0); 
    
    printf("upper_byte_ch0:%"PRIu8"\n lower_byte_ch0:%"PRIu8"\n resulting 16-bit\
    val:%"PRIu16"\n", upper_byte_ch0, lower_byte_ch0, *ch0_adc_val);
    
    /* get data from channel 1 first */
    /* read lower byte */
    uint8_t lower_byte_ch1;
    if(light_sensor_read_reg(light_sensor_fd, LOWER_BYTE_CH1_ADDR, &lower_byte_ch1)!=SUCCESS)
        return FAILURE;

    /* read upper byte */
    uint8_t upper_byte_ch1;
    if(light_sensor_read_reg(light_sensor_fd, UPPER_BYTE_CH1_ADDR, &upper_byte_ch1)!=SUCCESS)
        return FAILURE;
    

    /* cast the upper byte to a uint16_t type, then do an 8-bit
     * shift, an OR with the lower byte will result in the required
     * 16-bit value */
    *ch1_adc_val=(((uint16_t)upper_byte_ch1)<<8) | (lower_byte_ch1); 
        
    printf("upper_byte_ch1:%"PRIu8"\n lower_byte_ch1:%"PRIu8"\n resulting 16-bit\
            val:%"PRIu16"\n", upper_byte_ch1, lower_byte_ch1, *ch1_adc_val);
    
    return SUCCESS;
}

/* get luminosity 
 * - calls read_adc to get adc count of ch0, ch1
 * - computes luminosity by calculating the ratio
 *   and applying the formula as per the datasheet
 *   */
i2c_rc get_luminosity(int light_sensor_fd, float* luminosity)
{
    
    uint16_t ch0_adc_count, ch1_adc_count;
    /* read ADC values of both channels in a register */
    if(read_adc(light_sensor_fd, &ch0_adc_count, &ch1_adc_count)!=SUCCESS)
        return FAILURE;
    
    float ch0_adc_float=ch0_adc_count, ch1_adc_float=ch1_adc_count;
    
    printf("ch0 adc count in float:%f, ch1_adc_float:%f\n\n", ch0_adc_float, ch1_adc_float);

    /* cast the result to a float and then assign */    
    float adc_count_ratio=ch1_adc_float/ch0_adc_float;
    

    /* these cases have been designed as per the 
     * lux formula given in the datasheet.
     * A conservative approach to cast all the ADC values
     * into floats before using them is employed. 
     * */
    if((adc_count_ratio>0)&&(adc_count_ratio<=0.50))
    {
        *luminosity= (0.0304 *ch0_adc_float) - (0.062 * ch0_adc_float * powf(adc_count_ratio, 1.4));
    }
    else if((adc_count_ratio>0.50)&&(adc_count_ratio<=0.61))
    {
        *luminosity= (0.0224 * ch0_adc_float) - (0.031 * ch1_adc_float);
    }
    else if((adc_count_ratio>0.61)&&(adc_count_ratio<=0.80))
    {
        *luminosity= (0.0128 * ch0_adc_float) - (0.0153 * ch1_adc_float);
    }
    else if((adc_count_ratio>0.80) && (adc_count_ratio<=1.30))
    {
        *luminosity= (0.00146 * ch0_adc_float) - (0.00112 * ch1_adc_float);
    }
    else if(adc_count_ratio > 1.30)
        *luminosity=0;
    else
        return FAILURE;

    return SUCCESS;
}

/* read the timing register */
i2c_rc read_timing_reg(int light_sensor_fd, gain_t* adc_gain, integ_time_t* integ_time, uint8_t* if_manual)
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

/* write a byte to the reg with address
 * reg_addr, the byte's value is *val 
 * */
i2c_rc light_sensor_write_reg(int light_sensor_fd, uint8_t reg_addr, uint8_t* val)
{
    uint8_t cmd_reg_val=CMD_REG|reg_addr;

    /* request access to the given register */
    if(i2c_write(&cmd_reg_val,light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* write the byte to the given register */
    if(i2c_write(val, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    return SUCCESS;
}

/* returns the byte at the given register */
i2c_rc light_sensor_read_reg(int light_sensor_fd, uint8_t reg_addr, uint8_t* val)
{
    uint8_t cmd_reg_val=CMD_REG|reg_addr;

    /* write to the cmd requesting access for the given address */    
    if(i2c_write(&cmd_reg_val, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* do a single byte read now
     * -rest assured that it will come from the reg 
     *  whose address is reg_addr */
    if(i2c_read(val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    return SUCCESS;
}

/* Returns a word at the given address
 *
 * -just does two consecutive single byte reads and 
 *  doesn't read a word as the name suggests. 
 * -increments the address for the consecutive read. 
 *  */
i2c_rc light_sensor_read_word_reg(int light_sensor_fd, uint8_t reg_addr, uint16_t* word)
{
    /* read byte at reg_addr first */
    uint8_t cmd_reg_val=CMD_REG|reg_addr;
    

    if(i2c_write(&cmd_reg_val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    uint8_t lower_byte=0;
    
    if(i2c_read(&lower_byte, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    /* read upper byte at reg_addr+1 */
    cmd_reg_val=CMD_REG|(reg_addr+1);
    

    if(i2c_write(&cmd_reg_val, light_sensor_fd)!=SUCCESS)
        return FAILURE;
    
    uint8_t upper_byte=0;
    
    if(i2c_read(&upper_byte, light_sensor_fd)!=SUCCESS)
        return FAILURE;

    /* create the word using the lower and upper bytes */
    *word=(*word)|lower_byte;
    
    *word=(*word)|(((uint16_t)upper_byte)<<8);
    
     
    return SUCCESS;
}

/* writes calls i2c_write_word to do a write
 * 2-byte write.
 * pass the cmd, and the word to write in a
 * 3-byte buff along with the fd*/
i2c_rc light_sensor_write_word_reg(int light_sensor_fd, uint8_t reg_addr, uint16_t* word)
{
    /* create cmd to address the threshold low reg as a word (and not a byte) */
    uint8_t cmd_reg_val=CMD_REG_WORD|reg_addr;
    /* create the buffer to write to the bus, add cmd and word */
    uint8_t buff[3];
    buff[0]=cmd_reg_val;
    *((uint16_t*)&buff[1]) = *word;

    /* call i2c write */
    if(i2c_write_word(&buff[0], light_sensor_fd)!=SUCCESS)
    {
        perror("write word failed\n");
        exit(0);
    }
    
    /* return successfully */
    return SUCCESS;
}
/* reads a word from the threshold low reg */
i2c_rc light_sensor_read_thresh_low_reg(int light_sensor_fd, uint16_t* thresh_low)
{
    /* returns a word at the given address */
    if(light_sensor_read_word_reg(light_sensor_fd, THRESH_LOW_LOW_ADDR, thresh_low)!=SUCCESS)
        return FAILURE;
    
    return SUCCESS;
}
/* writes a word to the thresh low reg- doesn't work at the momment */
i2c_rc light_sensor_write_thresh_low_reg(int light_sensor_fd, uint16_t* thresh_low)
{
    /* writes a word to the given address- doesn't work at the momment */
    if(light_sensor_write_word_reg(light_sensor_fd, THRESH_LOW_LOW_ADDR, thresh_low)!=SUCCESS)
        return FAILURE;

    /* return successfully */
    return SUCCESS;
}

/* write to the interrupt control register 
 * takes the number of cycles, and an enum 
 * to enable/disable interrupts */
i2c_rc light_sensor_write_ic_register(int light_sensor_fd, ic_t if_enabled, uint8_t number_of_cycles)
{
    uint8_t ic_reg_val=0;
    
    if(number_of_cycles>15)
        return FAILURE;
    
    /* create the byte to write using bit masks */
    ic_reg_val=(ic_reg_val)|(if_enabled);
    
    ic_reg_val=(ic_reg_val)|(number_of_cycles);
    
    printf("byte written to the ic reg:%"PRIu8"\n", ic_reg_val); 
    /* write the byte required */
    if(light_sensor_write_reg(light_sensor_fd, IC_REG_ADDR, &ic_reg_val)!=SUCCESS)
        return FAILURE;

    /* return successfully */
    return SUCCESS;     
}

/* read the interrupt control register 
 * returns the number of cycles, and an enum 
 * to enable/disable interrupts via ptrs */
i2c_rc light_sensor_read_ic_register(int light_sensor_fd, ic_t* if_enabled, uint8_t* number_of_cycles)
{
    uint8_t ic_reg_val=0;
    
    /* read the byte required */
    if(light_sensor_read_reg(light_sensor_fd, IC_REG_ADDR, &ic_reg_val)!=SUCCESS)
        return FAILURE;
    
    /* get individual components using bit masks */
   
    *if_enabled=(ic_t)(INTERRUPT_ENABLE_MASK & ic_reg_val);
   
   
    *number_of_cycles=(NUMBER_OF_CYCLES_MASK & ic_reg_val);
    
    printf("byte read from the ic reg:%"PRIu8"\n", ic_reg_val); 

    /* return successfully */
    return SUCCESS;     
}
