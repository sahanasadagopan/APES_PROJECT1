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
#include "../includes/temp.h"

message_rc light_sensor_read_queue(thread_message_t* received_message)
{  
    
    if(mq_receive(light_sensor_queue, (char*)received_message, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
    {   
        perror("mq_receive failed in read ls queue\n");
        exit(0);
    }
    return MSGRC_SUCCESS;
}

message_rc temperature_sensor_read_queue(thread_message_t* received_message){
	if(mq_receive(light_sensor_queue,(char*)received_message,MAX_MSG_SIZE_LOG_QUEUE,NULL)==-1){
		perrot("mq_recieve failed in read temperature_sensor_queue\n");
		exit(0);
	}
}

/* identify if the message is a response or a request
 * and call apt routines*/
message_rc light_sensor_parse_message(thread_message_t* received_message, int light_sensor_fd)
{
    if(received_message->message_type==REQUEST_MESSAGE)
    {
        thread_message_t response_message;
        /* a function that sends the apt data that is requested and logs it to the logger queue */
        light_sensor_create_response(received_message, &response_message, light_sensor_fd);
        
        /* send it to the requester's queue */
        send_response(&response_message, received_message->tid);

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

message_rc temperature_sensor_parse_message(thread_message_t* received_message,int temp_sensor_fd){
	if(received_message->message_type==REQUEST_MESSAGE){
		thread_message_t response_message;
		/*THe function sends the apt data taht is requested and logs it to the light queue*/
		temperature_sensor_create_response(received_message,&response_message,temp_sensor_fd);
		/*send it to the requester response*/
		send_response(response_message,received_message->tid);

		return MSGRC_SUCCESS;
	}
	else if(received_message->message_type==RESPONSE_MESSAGE){
		//temperature_sensor_store_response(received_message,)
		/*a function that logs the data and logs the logger queue*/
		return MSGRC_SUCCESS;
	}
	else
		return MSGRC_FAILURE;
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
            /* create pointer to heap to hold the luminosity value */
            float* luminosity=(float*)malloc(sizeof(float));
            
            if(luminosity==NULL)
            {
                perror("memory allocation failure while allocating for luminosity- cannot continue");
                exit(0);
            }
                        
            /* pass that pointer with the light_sensor's file descriptor and get luminosity */
            get_luminosity(light_sensor_fd, luminosity);    
            
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
        default:
        {
            /* TODO: write to the logger */
            perror("incorrect data type recvd");
            exit(0);
        }
    }
}

/*creates a response to the light sensor request
* sent to the requester
 * message with the apt data to the
 * queue of the requesting thread.
 * */

message_rc temperature_sensor_create_response(thread_message_t* received_message, thread_message_t* response_message, int temp_sensor_fd)
{
	/* start creating the packet to be sent */
    
    /* the message type will be a response */
    response_message->message_type=RESPONSE_MESSAGE;
    
    /* its tid will be LIGHT_TASK */
    response_message->tid=TEMPERATURE_TASK;
    
    /* its data type will be the same as that of the received message */
    response_message->what_data=received_message->what_data;

    switch(response_message->what_data)
    {
      case TEMPERATURE_DATA_C:
      {
      	float *celsius = (float*)malloc(sizeof(float));
      	if(celsius== NULL){
      		perror("memory allocation for temp in C failed\n");
      	}
      	temperature_C(resolution,temp_sensor_fd,celsius);
      	response_message->data=(void *)celsius;

      	return MSGRC_SUCCESS;
      }

      case TEMPERATURE_DATA_F:
      float *fahreheit = (float*)malloc(sizeof(float));
      if(fahreheit==NULL){
      		perror("memory allocation for temp in F failed\n");
      }
      temperature_F(resolution,temp_sensor_fd,fahreheit);
      response_message->data=(void *)fahreheit;

      return MSGRC_SUCCESS;

      case TEMPERATURE_DATA_K:
      float *fahrenheit = (float*)malloc(sizeof(float));
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
    *queue_descriptor=mq_open(queue_name, O_WRONLY, 666, &queue_descriptor_attr);
    
    /* check if the queue got created properly */
    if(*queue_descriptor==(mqd_t)-1)
    {
        perror("mq in logger failed");
        exit(0);
    }
    
    /* return successfully */
    return MSGRC_SUCCESS;
}

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
           
            
            /* write to the queue using the queue descriptor */
            if(mq_send(light_task_qd, (char*)response_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
            {
                perror("write to the light task queue failed\n");
                exit(0);
            }
            return MSGRC_SUCCESS;
        }
        case TEMPERATURE_TASK:
        {
            mqd_t temperature_task_qd;
            
            /* open queue belonging to the temperature task */
            open_queue(&temperature_task_qd, TEMPERATURE_TASK_QUEUE); 
            
            /* write to the queue using the queue descriptor */
            if(mq_send(temperature_task_qd, (char*)response_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
            {
                perror("write to the temperature task queue failed\n");
                exit(0);
            }
            
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
            
            return MSGRC_SUCCESS;
        }
        
        default:
            return MSGRC_FAILURE;
    }

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



void heartbeat_api(char* task,log_message_t* hb_message){
    gettimeofday(&hb_message->timestamp,NULL);
    strcpy(hb_message->thread_name,task);
    hb_message->log_level=LOG_INFO;
    sprintf(hb_message->message,"%s:is alive",hb_message->thread_name);
    hb_message->length = strlen(hb_message->message)+sizeof(hb_message->log_level)+ sizeof(hb_message->timestamp) +1; 
}

void log_ls_id(uint8_t part_no, uint8_t rev, log_message_t* log_id)
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

void log_ts_id(uint8_t celsius,log_message_t* log_id){
    char *bus = "/dev/i2c-2";
    int temp_fd;
    float resolution=0.00;
    resolution=set_resolution("max",temp_fd);
    //temp_register(resolution,temp_fd,celsius);
    gettimeofday(&log_id->timestamp,NULL);
    strcpy(log_id->thread_name,"temperature sensor");
    log_id->log_level=LOG_INFO;
    sprintf(log_id->message,"%s: Temperature is %d",log_id->thread_name,celsius);
    log_id->length = strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp) +1;
}