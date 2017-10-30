#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "temp.h"
#include "i2c.h"
/*Global pthread variable*/
pthread_t temperature_thread;
pthread_t logger_thread;
pthread_t light_thread;
pthread_t decision_thread;

/*Pthread Attributes*/
pthread_attr_t temperature_attr;
pthread_attr_t logger_attr;
pthread_attr_t light_attr;
pthread_attr_t decision_attr;

/*struct params*/
struct sched_param temp_params;

void print_scheduler(void){
	int schedtype;
	schedtype = sched_getscheduler(getpid());

	switch(schedtype){
		case SCHED_FIFO:
			printf("Pthread Policy is SCHED_FIFO\n");
			break;
		case SCHED_OTHER:
			printf("Pthread policy SCHED_OTHER\n");

		case SCHED_RR:
			printf("Pthread policy RR\n");
	}
}

void *temperature(void *threadp){
	printf("got in temp thread\n");
}

void *decision(void *threadp){
	printf("Got to make the decision\n");
}

void main(int argc,char **argv){
	int ret,rt_max_prio,rt_min_prio,rc;
	struct sched_param main_param;
	pthread_attr_t main_attr;
	pid_t mainpid;

	mainpid=getpid();
	rc=sched_getparam(mainpid,&main_param);
	if(rc){
		printf("EROOR:sched_setscheduler %d\n",rc );
		exit(-1);
	}

	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
	rt_min_prio = sched_get_priority_min(SCHED_FIFO);
	main_param.sched_priority=rt_max_prio;
	rc=sched_setscheduler(getpid(),SCHED_FIFO,&main_param);
	if(rc<0) perror("Main program scheduling:\n");

	pthread_attr_init(&temperature_attr);
	pthread_attr_setinheritsched(&temperature_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&temperature_attr,SCHED_FIFO);
	temp_params.sched_priority =  rt_max_prio-1;

	pthread_attr_init(&decision_attr);
	pthread_attr_setinheritsched(&decision_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&decision_attr,SCHED_FIFO);
	temp_params.sched_priority =  rt_max_prio-2;


	if(ret = pthread_create(&temperature_thread,&temperature_attr,temperature,(void *)0)<0)
		perror("Thraed not created\n");
	if(ret = pthread_create(&decision_thread,(void *)0,decision,(void *)0)<0)
		perror("Thread not created\n");
	pthread_join(temperature_thread,NULL);
	pthread_join(decision_thread,NULL);

}