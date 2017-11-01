
/*  
 *  Name        : i2c.h
 *  
 *  Description : Contains all function prototypes, typedefs
 *                and enums for i2c.c
 *  
 *  Authors     : Ashwath Gundepally, CU, ECEE
 *                Sahana Sadagopan, CU, ECEE
 */
/* add this include guard */
#ifndef I2C_H
#define I2C_H

/* use this std library for using arch independent data types */

#include <stdint.h>

/* return this type for all the functions in the i2c driver */
typedef enum {SUCCESS, FAILURE} i2c_rc;

/* open the bus to use as i2c */
static i2c_rc open_i2c_bus(int* file);

i2c_rc i2c_write(uint8_t* byte_to_write, int file);

i2c_rc i2c_read(uint8_t* byte_read, int file);

i2c_rc i2c_init(uint8_t dev_addr, int* file);

#endif