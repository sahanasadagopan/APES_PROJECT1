#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mqueue.h>
#include <semaphore.h>
#include <inttypes.h>
#include <syslog.h>
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

pthread_mutex_t queue_lock;

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

message_rc task_read_queue(thread_message_t* received_message, mqd_t task_qd)
{  
    
    if(mq_receive(task_qd, (char*)received_message, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
    {   
        perror("mq_receive failed in read ls queue\n");
        exit(0);
    }
    return MSGRC_SUCCESS;
}
/* identify if the message is a response or a request
 * and call apt routines*/
message_rc light_sensor_parse_message(thread_message_t* received_message, int light_sensor_fd)
{
    if(received_message->message_type==REQUEST_MESSAGE)
    {
        thread_message_t response_message;
        //printf("received request message\n"); 
	    syslog(LOG_KERN |LOG_CRIT, "received request mesg\n");        
	/* a function that sends the apt data that is requested and logs it to the logger queue */
        light_sensor_create_response(received_message, &response_message, light_sensor_fd);
	    syslog(LOG_KERN |LOG_CRIT, "created response packet\n");        
        /* send it to the requester's queue */
        send_response(&response_message, received_message->tid);
	    syslog(LOG_KERN |LOG_CRIT, "sent response ls\n");        
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

/* 
 * identify if the message is a response or a request
 * and call apt routines
 * */
message_rc temperature_task_parse_message(thread_message_t* received_message, int temperature_sensor_fd)
{
    if(received_message->message_type==REQUEST_MESSAGE)
    {
         
        return MSGRC_SUCCESS; 
    }
    else if(received_message->message_type==RESPONSE_MESSAGE)
    {
        /* a function that receives the data and logs to the logger queue */
        temperature_sensor_receive_response(received_message);
        
        return MSGRC_SUCCESS;
    }
    else
        return MSGRC_FAILURE;
}

/* write given message to the logger queue */
message_rc write_log_message(log_message_t* log_message)
{
    mqd_t logger_queue_qd;
    
    /* program exits on failure */
    open_queue(&logger_queue_qd, LOG_QUEUE_NAME);
    
    if(mq_send(logger_queue_qd, (char*)log_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
    {
        return MSGRC_FAILURE;
    }
    return MSGRC_SUCCESS;
}

/* receive the data in the message and log about it
 * - this data maybe a ptr to the heap, in that case,
 *   free the ptr.*/
message_rc temperature_sensor_receive_response(thread_message_t* received_message)
{
    switch(received_message->what_data)
    {
        /* log this info */
        /* create the log packet */
        log_message_t log_message;
           
        log_message.log_level=LOG_INFORMATION;
        strcpy(log_message.thread_name, TEMPERATURE_TASK_NAME);
        
        case LUMINOSITY_DATA:
        {   
            /* localise the data that's on the heap */
            float luminosity_received=*((float*)received_message->data);  
            
            /* free the ptr to the heap */
            free(received_message->data);
            
            
            sprintf(log_message.message, "Received the lumonisity value as:%f from the Light Task", luminosity_received);
            
            if(gettimeofday(&log_message.timestamp, NULL)!=0)
            {
                perror("gettod call failed in temp sensor receive response\n");
                exit(0);
            }
            
            if(write_log_message(&log_message)!=MSGRC_SUCCESS)
            {
                perror("write to log queue failed in temp sensor receive response");
                exit(0);
            }
            
            return MSGRC_SUCCESS;
        }
        case LIGHT_SENSOR_ID:
        {
            /* localise the data that's on the heap */
            ls_id_val ls_id=*((ls_id_val*)received_message->data);
            
            /* free the ptr to the heap */
            free(received_message->data);
                
            
            sprintf(log_message.message, "Received the id register's contents as part_no:%d, rev_no:%d from the Light Task", ls_id.part_no, ls_id.rev_no);
            
            if(gettimeofday(&log_message.timestamp, NULL)!=0)
            {
                perror("gettod call failed in temp sensor receive response\n");
                exit(0);
            }
            
            if(write_log_message(&log_message)!=MSGRC_SUCCESS)
            {
                perror("write to log queue failed in temp sensor receive response");
                exit(0);
            }
                
                    
            return MSGRC_SUCCESS;
        }
        case LIGHT_SENSOR_IC_RD:
        {
            /* localise the data that's on the heap */
            ic_reg_val ic_reg=*((ic_reg_val*)received_message->data);
            /* free the ptr to the heap */
            free(received_message->data);
            /* create log message */
            sprintf(log_message.message, "Received the interrupt control register's contents as\
                    IC-bit:%d, number_of_cycles:%d from the Light Task", ic_reg.interrupts, ic_reg.number_of_cycles);
            /* get time stamp */    
            if(gettimeofday(&log_message.timestamp, NULL)!=0)
            {
                perror("gettod call failed in temp sensor receive response\n");
                exit(0);
            }
            
            if(write_log_message(&log_message)!=MSGRC_SUCCESS)
            {
                perror("write to log queue failed in temp sensor receive response");
                exit(0);
            }
            

            /* return successfully */
            return MSGRC_SUCCESS;
        }
        case LIGHT_SENSOR_TIMING_RD:
        {
            /* localise the data that's on the heap */
            timing_reg_val timing_reg=*((timing_reg_val*)received_message->data);
            
            /* free the ptr to the heap */
            free(received_message->data);
            sprintf(log_message.message, "Received the timing register's contents as\
                    gain-bit{1x:4x::0:1}:%d, integration time{max:mod:min::2:1:0}:%d,\
                    and the manual timing bit{auto:manual::0:1}:%d from the Light Task", timing_reg.gain_val, timing_reg.integ_time, timing_reg.if_manual);
            
            if(gettimeofday(&log_message.timestamp, NULL)!=0)
            {
                perror("gettod call failed in temp sensor receive response\n");
                exit(0);
            }
            
            if(write_log_message(&log_message)!=MSGRC_SUCCESS)
            {
                perror("write to log queue failed in temp sensor receive response");
                exit(0);
            }
             
            /* return successfully */
            return MSGRC_SUCCESS; 
        }   
        case IF_NIGHT_DAY:
        {
            /* localise the data that's on the heap */
            time_of_day_t time_of_day=*((time_of_day_t*)received_message->data);
            
            /* free the ptr to the heap */
            free(received_message->data);
            
            sprintf(log_message.message, "Received bit indicating what time of data it is {DAY_TIME:NIGHT_TIME::0:1}-%d, from the light Task", time_of_day);
            
            if(gettimeofday(&log_message.timestamp, NULL)!=0)
            {
                perror("gettod call failed in temp sensor receive response\n");
                exit(0);
            }
            
            if(write_log_message(&log_message)!=MSGRC_SUCCESS)
            {
                perror("write to log queue failed in temp sensor receive response");
                exit(0);
            }

        }
        default:
        {
            /* send this as an error message to the log queue */
            strcpy(log_message.message, "Received illegal response from Light task");
            log_message.log_level=LOG_CRITICAL;
            
            if(gettimeofday(&log_message.timestamp, NULL)!=0)
            {
                perror("gettod call failed in temp sensor receive response\n");
                exit(0);
            }

            /* exit program and fix the error if this fails */
            if(write_log_message(&log_message)!=MSGRC_SUCCESS)
            {
                perror("write to log queue failed in temp sensor receive response");
                exit(0);
            }
        }
    }
}

/* create a response packet ready to be  
 * sent to the requester
 * message with the apt data to the
 * queue of the requesting thread.
 * */
message_rc light_sensor_create_response(thread_message_t* received_message, thread_message_t* response_message, int light_sensor_fd)
{
    /* start creating the packet to be sent */
    
    /* the message type will be a response */
    response_message->message_type=RESPONSE_MESSAGE;
    
    /* its tid will be LIGHT_TASK */
    response_message->tid=LIGHT_TASK;
    
    /* its data type will be the same as that of the received message */
    response_message->what_data=received_message->what_data;
    
    /* find out which data was requested and then send it by 
     * calling the apt driver level APIs that interact with
     * the sensor */
    switch(response_message->what_data)
    {
        case LUMINOSITY_DATA:
        {   
            syslog(LOG_KERN|LOG_CRIT, "processing luminosity\n");
            /* create pointer to heap to hold the luminosity value */
            float* luminosity=(float*)malloc(sizeof(float));
            
            if(luminosity==NULL)
            {
                perror("memory allocation failure while allocating for luminosity- cannot continue");
                exit(0);
            }
            *luminosity=10;            
            /* pass that pointer with the light_sensor's file descriptor and get luminosity */
            //get_luminosity(light_sensor_fd, luminosity);    
            
            syslog(LOG_KERN|LOG_CRIT, "got luminosity\n");
            
            /* now assign that pointer as the data value of the response packet */
            response_message->data=(void*)luminosity;
               
            return MSGRC_SUCCESS;
        }
        case LIGHT_SENSOR_ID:
        {
            /* create pointer to the heap to the hold the light sensor id structure*/
            ls_id_val* ls_id=(ls_id_val*)malloc(sizeof(ls_id_val));
            
            if(ls_id==NULL)
            {
                perror("memory allocation failure while allocating memory for the ls_id");
                exit(0);
            }
            
            /* call driver routine that grabs the id */
            if(read_id_reg(light_sensor_fd, &(ls_id->part_no), &(ls_id->rev_no))!=SUCCESS)
            {
                perror("call to read reg id failed in ls create response");
                exit(0);
            }
            
            /* assign the ptr to the heap after casting it to a (void*) */
            response_message->data=(void*)ls_id; 
            
            return MSGRC_SUCCESS;
        }
        case LIGHT_SENSOR_IC_RD:
        {
            ic_reg_val* ic_reg=(ic_reg_val*)malloc((sizeof(ic_reg_val)));
            
            /* check malloc's return */
            if(ic_reg==NULL)
            {
                perror("memory allocation failure while allocating memory for ic_reg_val-can't continue");
                exit(0);
            }
            
            /* read the ic register */ 
            if(light_sensor_read_ic_register(light_sensor_fd, &(ic_reg->interrupts), &(ic_reg->number_of_cycles))!=SUCCESS)
            {
                perror("read to the ic reg failed in create response");
            }
            
            /* assign the ptr to the heap after casting it to a (void*) */
            response_message->data=(void*)ic_reg;
            
            /* return successfully */
            return MSGRC_SUCCESS;
        }
#ifdef LIGHT_SENSOR_WRITES
        case LIGHT_SENSOR_IC_WR:
        {                
            /* localize the value on the stack from the heap */
            ic_reg_val ic_val_write=(*(ic_reg_val*)received_message->data);
            
            /* free the memory on the heap */
            free(received_message->data);
            
            /* write to the ic reg the data that is requested to be written */
            if(light_sensor_write_ic_register(light_sensor_fd, ic_val_write.interrupts, ic_val_write.number_of_cycles)!=SUCCESS)
            {
                perror("write to the ic register failed");
                exit(0);
            }
            
            /* return successfully */
            return MSGRC_SUCCESS;
        }
#endif
        case LIGHT_SENSOR_TIMING_RD:
        {
            /* create a ptr to the heap with space to hold the value of this register's attrs */
            timing_reg_val* timing_reg=(timing_reg_val*)malloc(sizeof(timing_reg_val));
            
            /* call the driver level function to read the value */
            if(read_timing_reg(light_sensor_fd,  &(timing_reg->gain_val), &(timing_reg->integ_time),  &(timing_reg->if_manual))!=SUCCESS)
            {
                perror("timing reg read failed");
                exit(0);
            }
            
            /* return successfully */
            return MSGRC_SUCCESS; 
        }   
#ifdef LIGHT_SENSOR_WRITES
        case LIGHT_SENSOR_TIMING_WR:
        {
            /* localize the value on the stack from the heap */
            timing_reg_val timing_reg=(*(timing_reg_val*)received_message->data);
            
            /* free the memory on the heap */
            free(received_message->data);
            
            /* call the driver level function to write the value */
            if(write_timing_reg(light_sensor_fd,  (timing_reg.gain_val), (timing_reg.integ_time),  (timing_reg.if_manual))!=SUCCESS)
            {
                perror("timing reg write failed");
                exit(0);
            }
            
            /* return successfully */
            return MSGRC_SUCCESS;
        }
#endif
        case IF_NIGHT_DAY:
        {
            float luminosity;
                        
            /* pass that pointer with the light_sensor's file descriptor and get luminosity */
            get_luminosity(light_sensor_fd, &luminosity);    
        
            /* assign memory on the heap to the ptr to time_of_day_t */
            time_of_day_t* time_of_day=(time_of_day_t*)malloc(sizeof(time_of_day_t));
            
            if(time_of_day==NULL)
            {
                perror("memory allocation failure\n");
                exit(0);
            }
            
            /* determine what time of day it is based on the luminosity values */
            if(luminosity>=DAYTIME_LUMINOSITY)
                *time_of_day=DAY_TIME;
            else
                *time_of_day=NIGHT_TIME;
            response_message->data=(void*)time_of_day;               
            
            return MSGRC_SUCCESS;            
        }
        default:
        {
            /* TODO: write to the logger */
            perror("incorrect data type recvd");
            exit(0);
        }
    }
}

/* open queue to write to it */
message_rc open_queue(mqd_t* queue_descriptor, const char* queue_name)
{
    /* assign queue attributes */
    struct mq_attr queue_descriptor_attr;
    
    queue_descriptor_attr.mq_flags=0;
        
    queue_descriptor_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    queue_descriptor_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    
    /* create queue and make the ptr argument pt to its descriptor */    
    *queue_descriptor=mq_open(queue_name, O_RDWR, 777, &queue_descriptor_attr);
    
    /* check if the queue got created properly */
    if(*queue_descriptor==(mqd_t)-1)
    {
        syslog(LOG_CRIT|LOG_KERN, "%s couldn't open\n", queue_name);
        perror("mq open failed");
        exit(0);
    }
    
    /* return successfully */
    return MSGRC_SUCCESS;
}

/* sends packet to the queue of the thread with
 * the thread id given and then calls routine that logs about it
 * */

message_rc send_response(thread_message_t* response_message, thread_id tid_requester)
{
    /* grab the timestamp now */
    if(gettimeofday(&(response_message->timestamp), NULL)==-1)
    {
        perror("get tod fails in ls send response");
        exit(0);
    }

    switch(tid_requester)
    {
        case LIGHT_TASK:
        {
            mqd_t light_task_qd;
            
            /* open queue belonging to the light task */
            open_queue(&light_task_qd, LIGHT_TASK_QUEUE); 
           
            syslog(LOG_CRIT|LOG_KERN, "opened light queue to write request\n"); 
            /* write to the queue using the queue descriptor */
            if(mq_send(light_task_qd, (char*)response_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
            {
                perror("write to the light task queue failed\n");
                exit(0);
            }
            syslog(LOG_CRIT|LOG_KERN, "wrote to light queue\n"); 
            
            mq_close(light_task_qd);
            
            return MSGRC_SUCCESS;
        }
        case TEMPERATURE_TASK:
        {
            mqd_t temperature_task_qd;
            syslog(LOG_CRIT|LOG_KERN, "opened temp queue to write \n"); 
            
            /* open queue belonging to the temperature task */
            open_queue(&temperature_task_qd, TEMPERATURE_TASK_QUEUE); 
            
            /* write to the queue using the queue descriptor */
            if(mq_send(temperature_task_qd, (char*)response_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
            {
                perror("write to the temperature task queue failed\n");
                exit(0);
            }
            syslog(LOG_CRIT|LOG_KERN, "wrote to temp queue\n"); 

            mq_close(temperature_task_qd);
            
            return MSGRC_SUCCESS;
        }
        case MAIN_TASK:
        {
            mqd_t main_task_qd;
            
            /* open queue belonging to the main task */
            open_queue(&main_task_qd, MAIN_TASK_QUEUE); 
            
            /* write to the queue using the queue descriptor */
            if(mq_send(main_task_qd, (char*)response_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
            {
                perror("write to the main task queue failed\n");
                exit(0);
            }
            
            mq_close(main_task_qd);
            
            return MSGRC_SUCCESS;
        }
        
        default:
            return MSGRC_FAILURE;
    }

}


void *light_sensor(void *light_sensor_param)
{
    int light_sensor_fd=0;
    
    //i2c_init(LIGHT_SENSOR_DEV_ADDR, &light_sensor_fd);    
    
    //turn_on_light_sensor(light_sensor_fd);
    float luminosity=10;

    //get_luminosity(light_sensor_fd, &luminosity); 
     
    mqd_t light_task_qd;

    
    syslog(LOG_CRIT|LOG_KERN, "light queue opened for reading, luminosity is:%f\n", luminosity); 
    int i=0;
    while(i<10)
    {
        thread_message_t received_message;
        syslog(LOG_CRIT|LOG_KERN, "Light task about to read its queue\n");
        open_queue(&light_task_qd, LIGHT_TASK_QUEUE);
        task_read_queue(&received_message, light_task_qd); 
        mq_close(light_task_qd);
        syslog(LOG_CRIT|LOG_KERN, "Light task read its queue\n");
        syslog(LOG_CRIT|LOG_KERN, "read %d mesg\n", i); 
        light_sensor_parse_message(&received_message, light_sensor_fd);
        syslog(LOG_CRIT|LOG_KERN, "iteration :%d\n", i); 
        i++;
        sleep(1);
    }
    
    
    pthread_exit(0);
}

void* temperature(void* temperature_sensor_param)
{
    mqd_t temperature_task_qd;
    
    open_queue(&temperature_task_qd, TEMPERATURE_TASK_QUEUE);
    
    thread_message_t request_mesg;
    request_mesg.message_type=REQUEST_MESSAGE;
    request_mesg.tid=TEMPERATURE_TASK;
    request_mesg.data=NULL;
    int i=0;
    while(i<10)
    {
        request_mesg.what_data=(data_type_t)0;
        
        /* using this func for now, needs to be changed soon */
        send_response(&request_mesg, LIGHT_TASK);
        syslog(LOG_CRIT|LOG_KERN, "sent %d response\n", i); 
        thread_message_t received_message;

        syslog(LOG_CRIT|LOG_KERN, "Temperature task about to read its queue\n");
        
        open_queue(&temperature_task_qd, TEMPERATURE_TASK_QUEUE);
        task_read_queue(&received_message, temperature_task_qd);
        mq_close(temperature_task_qd);
        syslog(LOG_CRIT|LOG_KERN, "Temperature task  read its queue\n");
        syslog(LOG_CRIT|LOG_KERN, "got mesg %d in queue\n", i);
        temperature_task_parse_message(&received_message, 0);    
        i++;
        sleep(1);
    }
    
}

/*message_rc request_ls_id(thread_message_t* ls_id_message, ls_id_val* ls_id)
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
*/

void *logger(void *logger_param)
{
    
    log_message_t received_log;
    
    mqd_t logger_task_qd;
    
    open_queue(&logger_task_qd, LOG_QUEUE_NAME);
    
    FILE* fp=fopen("logfile.log", "w");
    if(fp==NULL)
    {
        perror("file creation failed in log queue");
    }
    int i=0; 
    while(i<10)
    {    
        if(mq_receive(logger_task_qd, (char*)&received_log, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
        {
            perror("mq_receive failed in logger queue");
            exit(0);
        }
        i++;
        fprintf(fp, "the message received:%s\n", received_log.message);
        sleep(1); 
    }
    pthread_exit(0);
}

int main(int argc,char **argv)
{
    /* create the light sensor queue */ 
    struct mq_attr light_sensor_queue_attr;
    
    light_sensor_queue_attr.mq_flags=0;
        
    light_sensor_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    light_sensor_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
                
    mqd_t light_sensor_queue=mq_open(LIGHT_TASK_QUEUE, O_CREAT|O_RDWR, 777, &light_sensor_queue_attr);
     
    if(light_sensor_queue==(mqd_t)-1)
    {
        perror("mq creaet light failed");
        exit(0);
    }
    mq_close(light_sensor_queue);
        
    /* create the logger queue */
    mqd_t logger_queue;
    
    struct mq_attr logger_queue_attr;
    
    logger_queue_attr.mq_flags=0;
        
    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
                
    logger_queue=mq_open(LOG_QUEUE_NAME, O_CREAT|O_RDWR, 777, &logger_queue_attr);
    
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq create logger failed");
        exit(0);
    }
    mq_close(logger_queue); 
    /* create the temperature task queue */
    mqd_t temperature_queue;
    
    struct mq_attr temperature_queue_attr;
    
    temperature_queue_attr.mq_flags=0;
        
    temperature_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    temperature_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
                
    temperature_queue=mq_open(TEMPERATURE_TASK_QUEUE, O_CREAT|O_RDWR, 777, &logger_queue_attr);
    
    if(temperature_queue==(mqd_t)-1)
    {
        perror("mq create temperature failed");
        exit(0);
    }
    mq_close(temperature_queue); 
    
    
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
	
    if(pthread_create(&temperature_sensor_td, NULL, temperature, NULL)<0)
    {
        perror("Thread not created\n");
	    exit(1);
    }
    while(1);
   /* 
    pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
    pthread_join(temperature_sensor_td, NULL);
    */
    return 0;
}
