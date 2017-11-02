#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
//#include "temp.h"
//#include "i2c.h"
/*Global pthread variable*/
pthread_t temperature_thread;
pthread_t logger_thread;
pthread_t light_thread;
pthread_t decision_thread;

pthread_mutex_t htimelock=PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t decision_mutex=PTHREAD_MUTEX_INITIALIZER;

/*Pthread Attributes*/
pthread_attr_t temperature_attr;
pthread_attr_t logger_attr;
pthread_attr_t light_attr;
pthread_attr_t decision_attr;

/*struct params*/
struct sched_param temp_params;

/*sigatomic*/
static volatile sig_atomic_t count_temp=1;
static volatile sig_atomic_t count_decision=1;
pthread_cond_t hcond;
pthread_cond_t hdcond;

#define WAIT_TIME_SECONDS 1

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

	
	
	while(1){
		//	usleep(100000);
		pthread_mutex_lock(&htimelock);
		printf("temperature thread got mutex\n");
		if(count_temp != 1){

			pthread_cond_signal(&hdcond);
			count_temp = 1;

			printf("temperature Thread sent signal\n");
		}
		pthread_mutex_unlock(&htimelock);
	}
}

void *decision(void *threadp){
	//pthread_mutex_t htimelock;
	
	
	while(1){
		//printf("decision thread while loop\n");
		//usleep(100000);
		pthread_mutex_lock(&decision_mutex);
		printf("decision Thread got mutex\n");
		
		if(count_decision != 1){
			

			pthread_cond_signal(&hcond);
			count_decision = 1;
			printf("decision Thread sent signal\n");	

		}
		pthread_mutex_unlock(&decision_mutex);
		

	}
	
	

}

void main(int argc,char **argv){
	int ret,rt_max_prio,rt_min_prio,rc;
	struct sched_param main_param;
	pthread_attr_t main_attr;
	pid_t mainpid;
    
    struct timespec heartbeat_time;
    struct timeval tp;

	/*mainpid=getpid();
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
	temp_params.sched_priority =  rt_max_prio-2;*/


	if(ret = pthread_create(&temperature_thread,(void *)0,temperature,(void *)0)<0)
		perror("Thraed not created\n");
	if(ret = pthread_create(&decision_thread,(void *)0,decision,(void *)0)<0)
		perror("Thread not created\n");
	while(1){
		printf("here in main\n");

		pthread_mutex_lock(&htimelock);
		rc = gettimeofday(&tp,NULL);
		heartbeat_time.tv_sec = tp.tv_sec;
		heartbeat_time.tv_nsec = tp.tv_usec * 1000;
		heartbeat_time.tv_sec +=WAIT_TIME_SECONDS;
		if(count_temp == 1){
			count_temp =0;
			rc=pthread_cond_timedwait(&hdcond,&htimelock,&heartbeat_time);
			if(rc==ETIMEDOUT){
				printf("wait timed out!\n");
				pthread_mutex_unlock(&htimelock);

				
			}
			pthread_mutex_unlock(&htimelock);
			/*conditional wait for temp thread*/
		}
		



		pthread_mutex_lock(&decision_mutex);
		rc = gettimeofday(&tp,NULL);
		heartbeat_time.tv_sec = tp.tv_sec;
		heartbeat_time.tv_nsec = tp.tv_usec * 1000;
		heartbeat_time.tv_sec +=WAIT_TIME_SECONDS;
		if(count_decision == 1){
			count_decision = 0;
			rc=pthread_cond_wait(&hcond,&decision_mutex);
			if(rc==ETIMEDOUT){
				printf("wait timed out!\n");
				pthread_mutex_unlock(&decision_mutex);
			}
			pthread_mutex_unlock(&decision_mutex);
			/*conditional wait for decision thread*/
		}
		
		//printf("%d\n",count );
	}
	pthread_join(temperature_thread,NULL);
	pthread_join(decision_thread,NULL);

}