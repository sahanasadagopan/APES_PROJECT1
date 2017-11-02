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

#define MAX_MSGS_IN_LOG_QUEUE   100
#define MAX_MSG_SIZE_LOG_QUEUE  300
#define LOG_QUEUE_NAME "/dummy"

typedef enum {LOG_INFO, LOG_DRIVER, LOG_ALERT, LOG_CRITICAL} log_level_t;

typedef struct
{
    struct timeval timestamp;    
    uint16_t length;
    log_level_t log_level;
    char thread_name[30];
    char message[256];
}message_t;


#endif
