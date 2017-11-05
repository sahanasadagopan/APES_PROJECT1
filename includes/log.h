/*  File        :   log.h
 *
 *  Description :   Header file which contains the log message structure,
 *                  and prototypes used by the logging source files.
 *
 *
 *  Name        :   Sahana Sadagopan,
 *                  Ashwath Gundepally
 */
#ifndef LOG_H
#define LOG_H
#include <stdint.h>
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

#define MAX_MSGS_IN_LOG_QUEUE   1000
#define MAX_MSG_SIZE_LOG_QUEUE  300
#define LOG_QUEUE_NAME "/dummy"
#define DECISION_QUEUE_NAME "/decision"

typedef enum {LOG_INFO, LOG_DRIVER, LOG_ALERT, LOG_CRITICAL} log_level_t;

typedef struct
{
    struct timeval timestamp;    
    uint16_t length;
    log_level_t log_level;
    char thread_name[30];
    char message[256];
}message_t;

void heartbeat_api(char* task,message_t* hb_message);

void log_ls_id(uint8_t part_no, uint8_t rev, message_t* log_id);

void log_ts_id(uint8_t data,message_t* log_id);

#endif
