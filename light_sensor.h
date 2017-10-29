
/*  
 *  Name        : light_sensor.c
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


i2c_rc read_id_reg(int light_sensor_fd, uint8_t* part_no, uint8_t* rev_no);

i2c_rc read_control_reg(int light_sensor_fd, uint8_t* control_reg_byte);

i2c_rc write_control_reg(int light_sensor_fd, uint8_t* control_reg_byte);

i2c_rc turn_on_light_sensor(int light_sensor_fd);
