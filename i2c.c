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

#include "i2c.h"

#define DEV_ADDRESS         0x39
#define CMD_REG             0x80
#define CONTROL_REG_ADDR    0x00
#define CONTROL_REG_ON      0x03
#define CONTROL_REG_OFF     0x00

/* let file be a global variable for now- this represents the I2C bus */

/* let main be the temp driver */

int main()
{
    int file;
    
    i2c_init(DEV_ADDRESS, &file);

    uint8_t address_control_reg=CMD_REG|CONTROL_REG_ADDR;

    i2c_write(&address_control_reg, file);
    
    uint8_t byte_to_write=CONTROL_REG_ON;
    
    i2c_write(&byte_to_write, file);
    
    i2c_write(&address_control_reg, file);

    uint8_t byte_read=0;

    printf("byte read before the call:%" PRIu8 "\n", byte_read);

    i2c_read(&byte_read, file);
    
    printf("byte read after the call:%" PRIu8 "\n", byte_read);

}

/* Name         :   static i2c_rc open_i2c_bus();
 * 
 *
 * Description  :   This function will be called by i2c_init, 
 *                  opens the bus which will be running i2c with
 *                  the sensors. 
 *                  Simply calls open on the bus with read/write
 *                  access.
 *                  It's scope is limited to this file because
 *                  only i2c_init will ever call it.
 * 
 * Args         :   None. Because there is only one usable bus 
 *                  running i2c on the BBG.
 * 
 * Returns      :   SUCCESS: When the file descriptor returned
 *                           by open is non negative        
 *
 *                  FAILURE: When the file descriptor returned
 *                           by open is negative                 
 *
 * */
static i2c_rc open_i2c_bus(int* file)
{
    int adapter_nr = 2; /* probably dynamically determined */
    char filename[20];
           
    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    
    *file = open(filename, O_RDWR);
    
    /* ERROR HANDLING; you can check errno to see what went wrong */
    if (*file < 0) 
    {
        perror("bus open falied\n");
        return FAILURE;    
    }
    
    return SUCCESS;
}

/* Name         :   i2c_rc i2c_init(uint8_t dev_addr)
 * 
 * 
 * Description  :   This function initializes I2C with the 
 *                  device whos address is given by dev_addr.
 *
 *                  Open the available bus first, then uses the
 *                  ioctl system call to initialize I2C with slave
 *                  devices. 
 * 
 * Args         :   uint8_t dev_addr: 1 byte device address of the
 *                  I2C slave in consideration- the sensor. 
 * 
 * Returns      :   SUCCESS: When the call to open_i2c_bus is 
 *                           successful, and the return from
 *                           ioctl is non negative.
 *                                 
 * 
 *                  FAILURE: When either of the above mentioned
 *                           conditions fail.              
 * 
 * */
i2c_rc i2c_init(uint8_t dev_addr, int* file)
{
    /* open the I2C bus first */
    if(open_i2c_bus(file)!=SUCCESS)
    {
        perror("failed to open the i2c bus");
        return FAILURE;
    }
    
    /* initialize i2c with the device with address dev_addr */
    if (ioctl(*file, I2C_SLAVE, dev_addr) < 0) 
    {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        perror("initialisation failed\n");
        return FAILURE;
    }
    /* return success */
    return SUCCESS;    
}


/* Name         :   i2c_rc i2c_write(uint8_t* byte_to_write)
 * 
 * 
 * Description  :   Writes a byte to the I2C device. Takes a ptr
 *                  to the byte to be write. 
 * 
 * Args         :   uint8_t* byte_to_write: ptr to the byte
 *                  to be written. 
 * 
 * Returns      :   SUCCESS: When the call to the write sytem
 *                  call returns 1 (the number of bytes written) 
 *                                 
 * 
 *                  FAILURE: When write returns something other
 *                           than 1.
 * 
 * */
i2c_rc i2c_write(uint8_t* byte_to_write, int file)
{
    /* write a byte on the bus using the write system call,
     * this is just like writing to a file, except multibyte
     * writes will probably not work. 
     * the I2C module will take care of following other requirements
     * of the I2C protocol */
    if (write(file, byte_to_write, 1) != 1) 
    {
        /*ERROR HANDLING: i2c transaction failed */
        perror("write failed\n");
        return FAILURE;
    }
    /* return  success*/
    return SUCCESS;    
}

/* Name         :   i2c_rc i2c_write(uint8_t* byte_to_write)
 * 
 * 
 * Description  :   Reads a byte from the I2C device. Takes a ptr
 *                  to the byte in which the value will be stored.
 * 
 * Args         :   uint8_t* read_byte: ptr to the byte
 *                  to be read into. 
 * 
 * Returns      :   SUCCESS: When the call to the read sytem
 *                  call returns 1 (the number of bytes written) 
 *                                 
 * 
 *                  FAILURE: When read returns something other
 *                           than 1.
 * 
 * */
i2c_rc i2c_read(uint8_t* byte_read, int file)
{
    
    /* read a byte on the bus using the read system call,
     * this is just like reading from a file, except multibyte
     * reads will probably not work. 
     * the I2C module will take care of following other requirements
     * of the I2C protocol */
    if (read(file, byte_read, 1) != 1) 
    {
        /*ERROR HANDLING: i2c transaction failed */
        perror("read failed\n");
        return FAILURE;
    }
    
    /* return  success*/
    return SUCCESS;    
}
