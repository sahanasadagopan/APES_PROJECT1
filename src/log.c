#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mqueue.h>
#include <semaphore.h>
#include <inttypes.h>

#include "../includes/light_sensor.h"
#include "../includes/i2c.h"
#include "../includes/log.h"


/* Global pthread variable */
pthread_t temperature_sensor_td;
pthread_t logger_td;
pthread_t light_sensor_td;
pthread_t decision_thread;

sem_t temp_sem;


mqd_t light_sensor_queue;


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

message_rc light_sensor_read_queue(thread_message_t* received_message)
{  
    
    if(mq_receive(light_sensor_queue, (char*)received_message, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
    {   
        perror("mq_receive failed in read ls queue\n");
        exit(0);
    }
}
/* identify if the message is a response or a request
 * and call apt routines*/
message_rc light_sensor_parse_message(thread_message_t* received_message)
{
    if(received_message->message_type==REQUEST_MESSAGE)
    {
        /* a function that sends the apt data that is requested and logs it to the logger queue */
        
        return MSGRC_SUCCESS; 
    }
    else if(received_message->message_type==RESPONSE_MESSAGE)
    {
       /* a function that receives the data and logs to the logger queue */
    
        return MSGRC_SUCCESS;
    }
    else
        return MSGRC_FAILURE;
}


/* send a response to the request
 * message with the apt data to the
 * queue of the requesting thread.
 * */
message_rc light_sensor_create_response(thread_message_t* received_message)
{
    /* start creating the packet to be sent */
    thread_message_t response_message;
    
    /* the message type will be a response */
    response_message.message_type=RESPONSE_MESSAGE;
    
    /* its tid will be LIGHT_TASK */
    response_message.tid=LIGHT_TASK;
    
    /* its data type will be the same as that of the received message */
    response_message.what_data=received_message->what_data;
    
    /* find out which data was requested and then send it by 
     * calling the apt driver level APIs that interact with
     * the sensor */
    switch(response_message.what_data)
    {
        case LUMINOSITY_DATA:
        {   
            /* create pointer to heap to hold the luminosity value */
            float* luminosity=(float*)malloc(sizeof(float));
            
            /* pass that pointer with the light_sensor's file descriptor and get luminosity */
            get_luminosity(light_sensor_fd, luminosity);    
            
            /* now assign that pointer as the data value of the response packet */
            response_message.data=(void*)luminosity;
            
            /* grab the timestamp now */
            if(gettimeofday(&(response_message.timestamp), NULL)==-1)
            {
                perror("get tod fails in ls send response");
                exit(0);
            }
            /* send it to the requester's queue */
            light_sensor_send_response(response_message, received_message->tid);
            
            break;
        }
        case LIGHT_SENSOR_ID:
        {
            
            break;
        }
        case LIGHT_SENSOR_IC_REG:
        {
            break;
        }     
    }
    
}
/* sends packet to the queue of the thread with
 * the thread id given */
message_rc light_sensor_send_response(thread_message_t* response_message, thread_id tid)
{
    switch(tid)
    {
        case LIGHT_TASK:
            return MSGRC_SUCCESS;
            break;
        
        case TEMPERATURE_TASK:
            return MSGRC_SUCCESS;
            break;
        
        case MAIN_TASK:
            return MSGRC_SUCCESS;
            break;
        
        default:
            return MSGRC_FAILURE;
            break;    
    }

    return MSGRC_FAILURE;
}


void *light_sensor(void *light_sensor_param)
{
    uint8_t part_no=0, rev=0;
    
    thread_message_t received_message;

    light_sensor_read_queue(&received_message); 
    
    light_sensor_parse_message(&received_message); 
     
    
    
    
    pthread_exit(0);
}

void* temperature(void* temperature_sensor_param)
{
    ls_id_val id_val;
    thread_message_t ls_id_message;
    ls_id_message.tid=TEMPERATURE_TASK;

    request_ls_id(&ls_id_message, &id_val);

    
}

message_rc request_ls_id(thread_message_t* ls_id_message, ls_id_val* ls_id)
{
    ls_id_message->what_data= LIGHT_SENSOR_ID;
    ls_id_message->message_type=REQUEST_MESSAGE;
    
    if(gettimeofday(&(ls_id_message->timestamp), NULL)==-1)
    {
        perror("gettimeofday failed in request_ls_id\n");
        exit(0);
    }
    
    ls_id_message->data=(void*)ls_id;
    
    if(mq_send(light_sensor_queue, (char*)ls_id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
    {
        perror("send to ls queue failed\n");
        exit(0);
    }
    
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
    
    log_message_t received_log;
    
    
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
    
    struct mq_attr light_sensor_queue_attr;
    
    light_sensor_queue_attr.mq_flags=0;
        
    light_sensor_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    light_sensor_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
                
    light_sensor_queue=mq_open(LOG_QUEUE_NAME, O_CREAT|O_RDWR, 666, &light_sensor_queue_attr);
    if(light_sensor_queue==(mqd_t)-1)
    {
        perror("mq in logger failed");
        exit(0);
    }
	if(pthread_create(&light_sensor_td, NULL, light_sensor, NULL)<0)
    {
        perror("Thread not created\n");
        exit(1);
    }
    
/*	if(pthread_create(&logger_td, NULL, logger, NULL)<0)
    {
        perror("Thread not created\n");
	    exit(1);
    }
*/	
    if(pthread_create(&temperature_sensor_td, NULL, temperature, NULL)<0)
    {
        perror("Thread not created\n");
	    exit(1);
    }
    
//    pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
    pthread_join(temperature_sensor_td, NULL);
    
    return 0;
}
