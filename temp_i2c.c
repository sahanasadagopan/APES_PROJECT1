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

#define DEV_ADDR 0X48
int fd;
int file;
void
signal_handler(int signum)
{

  assert(0 == close(fd));

  exit(signum);

}


int main(){


  char *bus = "/dev/i2c-2"; /* Pins P9_19 and P9_20 */
  int addr = 0x48;          /* The I2C address of TMP102 */
  uint8_t *buf = malloc(sizeof(uint8_t)*2);
  //char buf[2] = {0};
  int temp;
  unsigned char MSB, LSB;

  float f,c;

  i2c_init(DEV_ADDR);
 /* Register the signal handler */
 signal(SIGINT, signal_handler);
 
 while(1)
   {
	if((i2c_read(buf))!=SUCCESS){
		perror("Reading error\n");
         }
     // Using I2C Read
//     if (read(file,buf,2) != 2) {
       /* ERROR HANDLING: i2c transaction failed */
//       perror("Failed to read from the i2c bus.\n");

//   } 
	else {

       MSB = buf[0];
       LSB = buf[1];

       /* Convert 12bit int using two's compliment */
       /* Credit: http://bildr.org/2011/01/tmp102-arduino/ */
       temp = ((MSB << 8) | LSB) >> 4;

       c = temp*0.0625;
       f = (1.8 * c) + 32;

       printf("Temp Fahrenheit: %f Celsius: %f\n", f, c);
     }

     sleep(1);
   }



}
