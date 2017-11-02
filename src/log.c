#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mqueue.h>
#include <semaphore.h>

#include "../includes/light_sensor.h"
#include "../includes/i2c.h"
#include "../includes/log.h"


/* Global pthread variable */
pthread_t temperature_sensor_td;
pthread_t logger_td;
pthread_t light_sensor_td;
pthread_t decision_thread;

sem_t temp_sem;

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

    sem_wait(&temp_sem);
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
        
        printf("the message sent:%s\n", id_message.message); 
    }
     
        
    pthread_exit(0);
}

void *logger(void *logger_param)
{
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
    sem_post(&temp_sem);
    
    message_t received_log;
   

    while(1)
    {
        
        if(mq_receive(logger_queue, (char*)&received_log, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
        {
            perror("mq_receive failed");
            exit(0);
        }
        
        printf("the message received:%s\n", received_log.message); 
    }
    pthread_exit(0);
}

int main(int argc,char **argv)
{
    sem_init(&temp_sem, 0, 0);
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

    pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
    
    return 0;
}
