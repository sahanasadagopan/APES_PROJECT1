#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mqueue.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "../includes/light_sensor.h"
#include "../includes/i2c.h"
#include "../includes/log.h"


/* Global pthread variable */
pthread_t temperature_sensor_td;
pthread_t logger_td;
pthread_t light_sensor_td;
pthread_t decision_td;

sem_t temp_sem;


pthread_mutex_t temp_lock=PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t decision_mutex=PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t log_mutex=PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t light_mutex=PTHREAD_MUTEX_INITIALIZER;

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
static volatile sig_atomic_t count_log=1;
static volatile sig_atomic_t count_light=1;
pthread_cond_t decisioncond;
pthread_cond_t tempcond;
pthread_cond_t logcond;
pthread_cond_t lightcond;

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
            break;

		case SCHED_RR:
			printf("Pthread policy RR\n");
            break;
	}
}

void heartbeat_api(char* task,message_t* hb_message){
	gettimeofday(&hb_message->timestamp,NULL);
	strcpy(hb_message->thread_name,task);
	hb_message->log_level=LOG_INFO;
	sprintf(hb_message->message,"%s:is alive",hb_message->thread_name);
	hb_message->length = strlen(hb_message->message)+sizeof(hb_message->log_level)+ sizeof(hb_message->timestamp) +1; 
}

void log_ls_id(uint8_t part_no, uint8_t rev, message_t* log_id)
{
   // message_t log_id;
    //struct timeval timestamp;
    gettimeofday(&log_id->timestamp, NULL);
    
    strcpy(log_id->thread_name, "light sensor");
    
    log_id->log_level=LOG_INFO;

    sprintf(log_id->message, "%s: The part number is: %d, and the revision is:%d",\
            log_id->thread_name, part_no, rev);
    
    log_id->length=strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp) + 1; 
}

void log_ts_id(uint8_t data,message_t* log_id){
	gettimeofday(&log_id->timestamp,NULL);
	strcpy(log_id->thread_name,"temperature sensor");
	log_id->log_level=LOG_INFO;

	sprintf(log_id->message,"%s: Temperature is %d",log_id->thread_name,data);
	log_id->length = strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp) +1;
}

void *temperature(void *threadp){
	printf("got in temp thread\n");
	uint8_t temp_value = 25;
	message_t temp_message;

	log_ts_id(temp_value, &temp_message); 
    
    mqd_t logger_queue;

    struct mq_attr logger_queue_attr;

    logger_queue_attr.mq_flags=0;

    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;

    //sem_wait(&temp_sem);
    logger_queue=mq_open(LOG_QUEUE_NAME, O_RDWR, 666, &logger_queue_attr);
    
    
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in light thread open failed");
        exit(0);
    }
   

	while(1){	

		if(mq_send(logger_queue, (char*)&temp_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
        }
        //printf("the message sent:%s\n", temp_message.message);
		pthread_mutex_lock(&temp_lock);
		//printf("temperature thread got mutex\n");
		if(count_temp != 1){

			pthread_cond_signal(&tempcond);
			count_temp = 1;

			printf("temperature Thread sent signal\n");
		}
		pthread_mutex_unlock(&temp_lock);
	}
}

void *light_sensor(void *light_sensor_param)
{
    /* file descriptor */
    int light_sensor_fd;
    
    //i2c_init(LIGHT_SENSOR_DEV_ADDR, &light_sensor_fd);
    
    uint8_t part_no=10, rev=5;
    
    //read_id_reg(light_sensor_fd, &part_no, &rev);
    
    message_t id_message;
    
    log_ls_id(part_no, rev, &id_message); 
    
    mqd_t logger_queue;

    struct mq_attr logger_queue_attr;

    logger_queue_attr.mq_flags=0;

    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;

    //sem_wait(&temp_sem);
    logger_queue=mq_open(LOG_QUEUE_NAME, O_RDWR, 666, &logger_queue_attr);
    
    
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in light thread open failed");
        exit(0);
    }
   
    
   
    while(1)
    {
        if(mq_send(logger_queue, (char*)&id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
        }
        
        //printf("the message sent:%s\n", id_message.message); 
        pthread_mutex_lock(&light_mutex);
		//printf("decision Thread got mutex\n");
		
		if(count_light != 1){
			

			pthread_cond_signal(&lightcond);
			count_light = 1;
			printf("light Thread sent signal\n");	

		}
		pthread_mutex_unlock(&light_mutex);
    }
     
        
    pthread_exit(0);
}

void *decision(void *threadp){
	//pthread_mutex_t htimelock;
	
	
	while(1){
		//printf("decision thread while loop\n");
		//usleep(100000);
		pthread_mutex_lock(&decision_mutex);
		//printf("decision Thread got mutex\n");
		
		if(count_decision != 1){
			

			pthread_cond_signal(&decisioncond);
			count_decision = 1;
			printf("decision Thread sent signal\n");	

		}
		pthread_mutex_unlock(&decision_mutex);
		

	}
	
	

}


void *logger(void *logger_param)
{
    mqd_t logger_queue;

    struct mq_attr logger_queue_attr;
    logger_queue_attr.mq_flags=0;
    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    
    logger_queue=mq_open(LOG_QUEUE_NAME, O_RDONLY, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in logger failed");
        exit(0);
    }
    //sem_post(&temp_sem);
    
    message_t received_log;
   

    while(1)
    {
        
        if(mq_receive(logger_queue, (char*)&received_log, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
        {
            perror("mq_receive failed");
            exit(0);
        }
        
        printf("the message received:%s\n", received_log.message); 


        if(count_log != 1){
			

			pthread_cond_signal(&logcond);
			count_log = 1;
			printf("log Thread sent signal\n");	

		}

    }
    pthread_exit(0);
}

void main(int argc,char **argv){
	int ret,rt_max_prio,rt_min_prio,rc;
	struct sched_param main_param;
	pthread_attr_t main_attr;
	pid_t mainpid;
    
    struct timespec heartbeat_time;
    struct timeval tp;

    /*message queue initialisations*/

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
*/
    /*pthread_attr_init(&decision_attr);
	pthread_attr_setinheritsched(&decision_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&decision_attr,SCHED_FIFO);
	temp_params.sched_priority =  rt_max_prio-2;*/
	message_t hb_message;
    mqd_t logger_queue;

    struct mq_attr logger_queue_attr;
	

    logger_queue_attr.mq_flags=0;

    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    
    logger_queue=mq_open(LOG_QUEUE_NAME, O_CREAT|O_RDWR, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in logger failed");
        exit(0);
    }
    //sem_init(&temp_sem, 0, 0);
	if(pthread_create(&light_sensor_td, NULL, light_sensor, NULL)<0)
    {
        perror("Thread not created\n");
        exit(1);
    }

	if(pthread_create(&logger_td, NULL, logger, NULL)<0)
    {
        perror("Thread not created\n");
	    exit(1);
    }

    
	if(ret = pthread_create(&temperature_sensor_td,(void *)0,temperature,(void *)0)<0)
		perror("Thraed not created\n");
	if(ret = pthread_create(&decision_td,(void *)0,decision,(void *)0)<0)
		perror("Thread not created\n");
	while(1){
		printf("here in main\n");

		pthread_mutex_lock(&temp_lock);
		rc = gettimeofday(&tp,NULL);
		heartbeat_time.tv_sec = tp.tv_sec;
		heartbeat_time.tv_nsec = tp.tv_usec * 1000;
		heartbeat_time.tv_sec +=WAIT_TIME_SECONDS;
		if(count_temp == 1){
			count_temp =0;
			rc=pthread_cond_timedwait(&tempcond,&temp_lock,&heartbeat_time);
			if(rc==ETIMEDOUT){
				printf("wait timed out!\n");
				pthread_mutex_unlock(&temp_lock);
			}
			heartbeat_api("temp",&hb_message);
			if(mq_send(logger_queue, (char*)&hb_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        	{
            	perror("mq_send failed");
            	exit(0);
        	}
			pthread_mutex_unlock(&temp_lock);
			/*conditional wait for temp thread*/
		}
		

		pthread_mutex_lock(&decision_mutex);
		rc = gettimeofday(&tp,NULL);
		heartbeat_time.tv_sec = tp.tv_sec;
		heartbeat_time.tv_nsec = tp.tv_usec * 1000;
		heartbeat_time.tv_sec +=WAIT_TIME_SECONDS;
		if(count_decision == 1){
			count_decision = 0;
			rc=pthread_cond_timedwait(&decisioncond,&decision_mutex,&heartbeat_time);
			if(rc==ETIMEDOUT){
				printf("wait timed out!\n");
				pthread_mutex_unlock(&decision_mutex);
			}
			heartbeat_api("decision",&hb_message);
			if(mq_send(logger_queue, (char*)&hb_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        	{
            	perror("mq_send failed");
            	exit(0);
        	}
			pthread_mutex_unlock(&decision_mutex);
			/*conditional wait for decision thread*/
		}

		pthread_mutex_lock(&log_mutex);
		rc = gettimeofday(&tp,NULL);
		heartbeat_time.tv_sec = tp.tv_sec;
		heartbeat_time.tv_nsec = tp.tv_usec * 1000;
		heartbeat_time.tv_sec +=WAIT_TIME_SECONDS;
		if(count_log == 1){
			count_log = 0;
			rc=pthread_cond_timedwait(&logcond,&log_mutex,&heartbeat_time);
			if(rc==ETIMEDOUT){
				printf("wait timed out!\n");
				pthread_mutex_unlock(&log_mutex);
			}
			heartbeat_api("log",&hb_message);
			if(mq_send(logger_queue, (char*)&hb_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        	{
            	perror("mq_send failed");
            	exit(0);
        	}
			pthread_mutex_unlock(&log_mutex);
			/*conditional wait for decision thread*/
		}

		pthread_mutex_lock(&light_mutex);
		rc = gettimeofday(&tp,NULL);
		heartbeat_time.tv_sec = tp.tv_sec;
		heartbeat_time.tv_nsec = tp.tv_usec * 1000;
		heartbeat_time.tv_sec +=WAIT_TIME_SECONDS;
		if(count_light == 1){
			count_light = 0;
			rc=pthread_cond_timedwait(&lightcond,&light_mutex,&heartbeat_time);
			if(rc==ETIMEDOUT){
				printf("wait timed out!\n");
				pthread_mutex_unlock(&light_mutex);
			}
			heartbeat_api("light",&hb_message);
			if(mq_send(logger_queue, (char*)&hb_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        	{
            	perror("mq_send failed");
            	exit(0);
        	}
			pthread_mutex_unlock(&light_mutex);
			/*conditional wait for decision thread*/
		}
		
		//printf("%d\n",count );
	}
	pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
	pthread_join(temperature_sensor_td,NULL);
	pthread_join(decision_td,NULL);

}
