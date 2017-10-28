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

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define TMP102_I2C	0x48
#define I2C_BUS		"/dev/i2c-1"

int main(void) {

	int file;
	char filename[40];
	int addr = TMP102_I2C; // The I2C address

	sprintf(filename, I2C_BUS);
	if ((file = open(filename, O_RDWR)) < 0) {
		printf("Failed to open the bus.");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		printf("error: %s (%d)\n", strerror(errno), errno);
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		printf("error: %s (%d)\n", strerror(errno), errno);
		exit(1);
	}

	write(file, 0x00, 1);

	usleep(500);

	int errCount = 0;
	for (;;) {
		char buf[1] = { 0 };

		float data;

		// Read 2 uint8 using I2C Read
		int k = read(file, buf, 1);
		printf("K return %d",k);
		if ((k != 1)) {
			errCount++;
			printf("error: %s (%d) %d\n", strerror(errno), errno, errCount);
		} else {

			int temperature;

			temperature = ((buf[0]) << 8) | (buf[1]);
			temperature >>= 4;

			//The tmp102 does twos compliment but has the negative bit in the wrong spot, so test for it and correct if needed
			if (temperature & (1 << 11))
				temperature |= 0xF800; //Set bits 11 to 15 to 1s to get this reading into real twos compliment

			printf(" temp:  %04f \t error:  %d\n", temperature * 0.0625,
					errCount);

		}
		usleep(5);

	}
	return 0;
}
