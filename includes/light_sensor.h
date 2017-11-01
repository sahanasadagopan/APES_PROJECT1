/*  
 *  Name        : light_sensor.h
 *  
 *  
 *  Description : Light Sensor I2C driver header file 
 *                that contains all the prototypes
 *                for functions defined in
 *                light_sensor.c
 *
 *
 *  Authors     : Ashwath Gundepally, CU, ECEE
 *                Sahana Sadagopan, CU, ECEE
 */             
#include <stdint.h>
#include "i2c.h"

#define DEV_ADDRESS         0x39

#define CMD_REG             0x80
#define CMD_REG_WORD        0xA0

#define CONTROL_REG_ADDR    0x00
#define CONTROL_REG_ON      0x03
#define CONTROL_REG_OFF     0x00

#define ID_REG_ADDRESS      0x0A 

/* timing reg stuff */
#define TIMING_REG_ADDR     0x01
#define ADC_GAIN_MASK       0x08
#define MANUAL_TIMING_MASK  0x04
#define INTEG_TIME_MASK     0x03


/* ADC reg stuff */
#define LOWER_BYTE_CH0_ADDR 0x0C
#define UPPER_BYTE_CH0_ADDR 0x0D
#define LOWER_BYTE_CH1_ADDR 0x0E
#define UPPER_BYTE_CH1_ADDR 0x0F

typedef enum {MIN_INTEG_TIME=0, MODERATE_INTEG_TIME=1, MAX_INTEG_TIME=2, INTEG_NA=3} integ_time_t;

typedef enum {MIN_GAIN=0, MAX_GAIN=1} gain_t;

/* THRESHOLD_LOW register stuff */
#define THRESH_LOW_LOW_ADDR   0x02
#define THRESH_LOW_HIGH_ADDR  0x03

/* Interrupt Control register stuff */
#define IC_REG_ADDR           0x06
#define INTERRUPT_ENABLE_MASK 0x10
#define NUMBER_OF_CYCLES_MASK 0x0F

typedef enum {INTERRUPT_ENABLE=0x10, INTERRUPT_DISABLE=0} ic_t; 


i2c_rc read_id_reg(int light_sensor_fd, uint8_t* part_no, uint8_t* rev_no);

i2c_rc read_control_reg(int light_sensor_fd, uint8_t* control_reg_byte);

i2c_rc write_control_reg(int light_sensor_fd, uint8_t* control_reg_byte);

i2c_rc light_sensor_write_reg(int light_sensor_fd, uint8_t reg_addr, uint8_t* val);

i2c_rc write_timing_reg(int light_sensor_fd,  gain_t adc_gain, integ_time_t integ_time, uint8_t if_manual);

i2c_rc read_timing_reg(int light_sensor_fd,  gain_t* adc_gain, integ_time_t* integ_time, uint8_t* if_manual);

i2c_rc light_sensor_read_reg(int light_sensor_fd, uint8_t reg_addr, uint8_t* val);

i2c_rc turn_on_light_sensor(int light_sensor_fd);

i2c_rc read_adc(int light_sensor_fd, uint16_t* ch0_adc_val, uint16_t* ch1_adc_val);

i2c_rc get_luminosity(int light_sensor_fd, float* luminosity);

i2c_rc light_sensor_read_word_reg(int light_sensor_fd, uint8_t reg_addr, uint16_t* word);

i2c_rc light_sensor_write_word_reg(int light_sensor_fd, uint8_t reg_addr, uint16_t* word);

i2c_rc light_sensor_read_thresh_low_reg(int light_sensor_fd, uint16_t* thresh_low);

i2c_rc light_sensor_write_thresh_low_reg(int light_sensor_fd, uint16_t* thresh_low);

i2c_rc light_sensor_read_ic_register(int light_sensor_fd, ic_t* if_enabled, uint8_t* number_of_cycles);

i2c_rc light_sensor_write_ic_register(int light_sensor_fd, ic_t if_enabled, uint8_t number_of_cycles);
