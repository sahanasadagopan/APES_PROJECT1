#include "../includes/log.h"

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

/*void sensor_initilization(int sensor_fd,uint8_t *value){
	float celsius,fahreheit;
	i2c_init(DEV_ADDR,&sensor_fd);
	float resolution=0.00,celsius,fahreheit;
	resolution=set_resolution("max",sensor_fd);
	temp_register(resolution,sensor_fd,celsius,fahreheit);
	value=(uint8_t *)(&celsius);
}*/
