#include "../includes/USRled.h"

int detection_led(){
	FILE* LEDHandle = NULL;
	const char *LEDBrightness = "/sys/class/leds/beaglebone:green:usr0/brightness";
int i =0;
for(i=0;i<2;i++){
	if((LEDHandle = fopen(LEDBrightness,"r+")) !=NULL){
		fwrite("1",sizeof(char),1,LEDHandle);
		fclose(LEDHandle);
	}
	usleep(10);
	if((LEDHandle = fopen(LEDBrightness,"r+")) != NULL){
		fwrite("0",sizeof(char),1,LEDHandle);
		fclose(LEDHandle);
		}
	//usleep(1000000);
	}
	return 0;
}
