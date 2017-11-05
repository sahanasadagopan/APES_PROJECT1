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


/* Global pthread variable */
pthread_t temperature_sensor_td;
pthread_t logger_td;
pthread_t light_sensor_td;
pthread_t decision_thread;

message_rc heartbeat_api(char* task,log_message_t* hb_message){
    gettimeofday(&hb_message->timestamp,NULL);
    strcpy(hb_message->thread_name,task);
    hb_message->log_level=LOG_INFO;
    sprintf(hb_message->message,"%s:is alive",hb_message->thread_name);
    hb_message->length = strlen(hb_message->message)+sizeof(hb_message->log_level)+ sizeof(hb_message->timestamp) +1; 
    return MSGRC_SUCCESS;
}

/* compares past present and past lux val 
 * and returns code which indicates if change is
 * required */
if_log_reqd_t if_lux_change_log(float luminosity_old, float luminosity,float* change_lux)
{
     *change_lux=luminosity_old-luminosity;

     if(*change_lux>MAX_CHANGE_PERMISSIBLE_POS)
     {
	/*return apt code suggesting log requirement*/	
	return CHANGE_LOG_REQUIRED;
     }

     if(*change_lux<MAX_CHANGE_PERMISSIBLE_NEG)
     {
     	/*return apt code suggesting log requirement*/	
        return CHANGE_LOG_REQUIRED;
     }

     /* value is within bounds */	
     return NO_CHANGE_LOG_REQUIRED;
}

message_rc log_ls_id(float luminosity, log_message_t* log_id)
{
   // message_t log_id;
    //struct timeval timestamp;
    gettimeofday(&log_id->timestamp, NULL);
    
    strcpy(log_id->thread_name, "light sensor");
    
    log_id->log_level=LOG_INFO;

    sprintf(log_id->message, "%s: Luminosity is %f",\
            log_id->thread_name, luminosity);
    
    log_id->length=strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp) + 1; 
    return MSGRC_SUCCESS;
}

message_rc log_ts_id(float celsius,float fahrenhiet,float kelvin,log_message_t* log_id){
    printf("in the log function\n");
    gettimeofday(&log_id->timestamp,NULL);
    strcpy(log_id->thread_name,"temperature sensor");
    log_id->log_level=LOG_INFO;
    sprintf(log_id->message,"%s: Temperature is in F,K and C %f %f %f ",log_id->thread_name,fahrenhiet,kelvin,celsius);
    log_id->length = strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp) +1;
    return MSGRC_SUCCESS;
}

message_rc log_ls_change(float change, log_message_t* log_id)
{
   // message_t log_id;
    //struct timeval timestamp;
    gettimeofday(&log_id->timestamp, NULL);
    
    strcpy(log_id->thread_name, "light sensor");
    
    log_id->log_level=LOG_ALERT;

    sprintf(log_id->message, "%s: Sudden Change in luminosity %f",\
            log_id->thread_name, change);
    
    log_id->length=strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp) + 1; 
	return MSGRC_SUCCESS;
}

message_rc task_initialise_msgpkt(log_message_t *log_id,const char* thread_name){
	gettimeofday(&log_id->timestamp, NULL);
    
    strcpy(log_id->thread_name, thread_name);
    
    log_id->log_level=LOG_INFO;

    sprintf(log_id->message, "%s: is Initialised",log_id->thread_name);
    
    log_id->length=strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp);

    return MSGRC_SUCCESS;

}

message_rc task_req_res(log_message_t *log_id,const char* thread_name,char* method){
	gettimeofday(&log_id->timestamp, NULL);
    
    strcpy(log_id->thread_name, thread_name);
    
    log_id->log_level=LOG_CRITICAL;

    sprintf(log_id->message, "%s: is %s to the query",log_id->thread_name,method);
    
    log_id->length=strlen(log_id->message)+sizeof(log_id->log_level)+sizeof(log_id->timestamp);

    return MSGRC_SUCCESS;

}

