/*  File        :   log.h
 *
 *  Description :   Header file which contains the log message structure,
 *                  non-logger message structure and function prototypes 
 *                  used by the inter-threaded comm source file.
 *
 *  Name        :   Sahana Sadagopan,
 *                  Ashwath Gundepally
 */

#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <sys/time.h>


#include "light_sensor.h"

#define MAX_MSGS_IN_LOG_QUEUE   1000
#define MAX_MSG_SIZE_LOG_QUEUE  300

/* queue names */
#define LOG_QUEUE_NAME          "/logger_task"
#define LIGHT_TASK_QUEUE        "/light_task"
#define TEMPERATURE_TASK_QUEUE  "/temperature_task"
#define DECISION_QUEUE_NAME "/decision"

typedef enum {LOG_INFO, LOG_DRIVER, LOG_ALERT, LOG_CRITICAL} log_level_t;

/* packet that will be written to the logger queue */
/* this data will be simply dumped to the file */
typedef struct
{
    struct timeval timestamp;
    uint16_t length;
    log_level_t log_level;
    char thread_name[30];
    char message[256];
}log_message_t;

/* different task names */
typedef enum { TEMPERATURE_TASK, LIGHT_TASK, MAIN_TASK, LOGGER_TASK, DECISION_TASK } thread_id;

/* the data that can be sent/received by different threads */
typedef enum { 
               LUMINOSITY_DATA, LIGHT_SENSOR_ID, LIGHT_SENSOR_IC_WR, LIGHT_SENSOR_IC_RD, LIGHT_SENSOR_TIMING_RD, LIGHT_SENSOR_TIMING_WR,\
               
               TEMPERATURE_DATA_C, TEMPERATURE_DATA_F, TEMPERATURE_DATA_K, TEMPERATURE_SET_RES, TEMPERATRUE_SD_RD, TEMPERATRUE_SD_WR,\
               
               TEMPERATRUE_INTR_RD, TEMPERATURE_INTR_WR   
             
             } data_type_t;

/* enum to define whether a message is a request or a response */
typedef enum { REQUEST_MESSAGE, RESPONSE_MESSAGE} message_type_t;

/* return code for the inter-thread message API*/
typedef enum { MSGRC_SUCCESS, MSGRC_FAILURE}  message_rc;


/* message packet that will be exchanged among threads */
typedef struct
{    
    struct timeval timestamp;
    thread_id tid;
    message_type_t message_type;
    data_type_t what_data;
    void* data;
}thread_message_t;

/* CHANGE LOG STUFF */
#define MAX_CHANGE_PERMISSIBLE_POS  50.0
#define MAX_CHANGE_PERMISSIBLE_NEG -50.0

typedef enum {CHANGE_LOG_REQUIRED, NO_CHANGE_LOG_REQUIRED} if_log_reqd_t;

if_log_reqd_t if_lux_change_log(float luminosity_old, float luminosity,float* change_lux);

typedef enum {DAY, NIGHT} night_day_t;
#define DAYTIME_THRESHOLD	8.0
message_rc request_ls_id(thread_message_t* ls_id_message, ls_id_val* ls_id);

message_rc light_sensor_create_response(thread_message_t* received_message, thread_message_t* response_message, int light_sensor_fd);

//message_rc light_sensor_send_response(thread_message_t* response_message, thread_id tid);

message_rc temperature_sensor_create_response(thread_message_t* received_message, thread_message_t* response_message, int temp_sensor_fd);

message_rc temperature_sensor_read_queue(thread_message_t* received_message);

message_rc light_sensor_read_queue(thread_message_t* received_message);

message_rc temperature_sensor_parse_message(thread_message_t* received_message,int temp_sensor_fd);

message_rc light_sensor_parse_message(thread_message_t* received_message, int light_sensor_fd);

message_rc open_queue(mqd_t* queue_descriptor, const char* queue_name);

message_rc send_response(thread_message_t* response_message, thread_id tid_requester);

message_rc heartbeat_api(char* task,log_message_t* hb_message);

message_rc log_ls_id(float luminosity, log_message_t* log_id);

message_rc log_ts_id(float celsius,float fahrenhiet,float kelvin,log_message_t* log_id);

message_rc task_initialise_msgpkt(log_message_t *log_id,const char* thread_name);

message_rc log_ls_change(float change, log_message_t* log_id);

message_rc task_req_res(log_message_t *log_id,const char* thread_name,char* method);

#endif
