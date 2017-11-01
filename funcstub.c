
#include <stdio.h>
#include <pthread.h>

void* Temperature(void *threaddp){
	/*Creates a queue to interact with the Logger*/
	/*Opens the DRIVER FILE and reads the data */
}

void* light(void *threaddp){
	/*creates a queue to interact with the Logger*/
	/*Opens the DRIVER FILE and reads the data */
}

void* logger(void *threaddp){
	/*Function waits for the data*/
	/*OPens a file */
	/*write the recieved data into the file*/
	/*closes the file pointer*/

}

void show_error(){
	/*Controlling the USR LEDS*/
	/* Opening the file that has the controls the USR led */
	/*Uses Fork to perform the operation*/
}


void main(char *args,int argv){
	/*pthread initilisations*/
	/*pthread sequencing*/
	/*FIFO SCHEDULING*/
	/*PRIORITY
	* Task 1 :Desicion Task
	*TASK 2 AND task 3: Temperature and light sensor
	*TASK 4 :Logger task
	*/
	/*Creating a message queue for interacting with the LOGGER TASK*/

	if((pthread_create(decision_thread,descision_attr,desicion,(void *)0))<0)
		/*If error detected calls for SHOW_ERROR function*/
	if((pthread_create(Temperture_thread,temp_attr,temp,(void *)0))<0)
		/*If error detected calls for SHOW_ERROR function*/
	if((pthread_create(Light_thread,light_attr,light,(void *)0))<0)
		/*If error detected calls for SHOW_ERROR function*/
	if((pthread_create(Logger_thread,logger_attr,logger,(void *)0))<0)
	/*If error detected calls for SHOW_ERROR function*/

	pthread_join(desicion_thread,NULL);
	pthread_join(Temperture_thread,NULL);
	pthread_join(Light_thread,NULL);
	pthread_join(Logger_thread,NULL);
	



}