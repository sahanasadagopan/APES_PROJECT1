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

#include "../includes/light_sensor.h"
#include "../includes/i2c.h"
#include "../includes/log.h"
#include "../includes/temp.h"
#include "../includes/USRled.h"

#define WAIT_TIME_SECONDS 1

FILE *logfile;
char* filepath;

/* Global pthread variable */
pthread_t temperature_sensor_td;
pthread_t logger_td;
pthread_t light_sensor_td;
pthread_t decision_td;

/*Mutex Variables */
pthread_mutex_t temp_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t decision_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t light_mutex=PTHREAD_MUTEX_INITIALIZER;

/*struct params*/
struct sched_param temp_params;

/*sigatomic variable for Heartbeat*/
static volatile sig_atomic_t count_temp=1;
static volatile sig_atomic_t count_decision=1;
static volatile sig_atomic_t count_log=1;
static volatile sig_atomic_t count_light=1;

/*Conditional Variable*/
pthread_cond_t decisioncond=PTHREAD_COND_INITIALIZER;
pthread_cond_t tempcond=PTHREAD_COND_INITIALIZER;
pthread_cond_t logcond=PTHREAD_COND_INITIALIZER;
pthread_cond_t lightcond=PTHREAD_COND_INITIALIZER;

/* NIGHT DAY SIGNALLING */

pthread_mutex_t temp_night_day=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t main_night_day=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t temp_cond_nd=PTHREAD_COND_INITIALIZER, main_cond_nd=PTHREAD_COND_INITIALIZER; 

night_day_t night_day_for_temp;
night_day_t night_day_for_main;


/****************************************************************
* Signal handler logs the data at the end and destroys all the mutexs
* and conditional variable unlinks the queues and closes the threads before
* exiting gracefully
*********************************************************************/
void int_handler()
{
  char ch;
  mqd_t logger_queue;
  struct mq_attr logger_queue_attr;
  log_message_t terminate_message;
  logger_queue_attr.mq_flags=0;
  logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
  logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
  printf("Enter y to close the program:");
  scanf("%c", &ch);
  logger_queue=mq_open(LOG_QUEUE_NAME, O_RDWR, 666,&logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in light thread open failed");
        exit(0);
    }
  task_initialise_msgpkt(&terminate_message,"killed all");
  if(mq_send(logger_queue, (char*)&terminate_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
  {
      perror("mq_send failed");
      exit(0);
  }
  mq_close(logger_queue);
  mq_unlink(LIGHT_TASK_QUEUE);
  mq_unlink(DECISION_QUEUE_NAME);
  mq_unlink(TEMPERATURE_TASK_QUEUE);
  mq_unlink(LOG_QUEUE_NAME);
  if(ch == 'y')
  {
	
	fclose(logfile);  
	pthread_cond_destroy(&decisioncond);
	pthread_cond_destroy(&logcond);
	pthread_cond_destroy(&tempcond);
	pthread_cond_destroy(&lightcond);
	pthread_cond_destroy(&temp_cond_nd);
	pthread_cond_destroy(&main_cond_nd);
	pthread_mutex_destroy(&temp_lock);
	pthread_mutex_destroy(&decision_mutex);
	pthread_mutex_destroy(&log_mutex);
	pthread_mutex_destroy(&light_mutex);
	pthread_mutex_destroy(&temp_night_day);
	pthread_mutex_destroy(&main_night_day);
	pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
	pthread_join(temperature_sensor_td,NULL);
	pthread_join(decision_td,NULL);
	pthread_exit(NULL);

	  
  }
  exit(0);

}


/*********************************************************************** 
 *Description: block on a condition variable 
 * 		until signalled to read a shared
 * 		piece of memory which has the updated
 * 		information.
 *************************************************************************/
void get_night_day_info_temp()
{
    /* block on a condition variable */
    pthread_mutex_lock(&temp_night_day);

    /* write to the logger here */
    pthread_cond_wait(&temp_cond_nd, &temp_night_day);
    /* read updated variable */

    if(night_day_for_temp==NIGHT)
    {
    	/* log saying its night */
        printf("got night val from ls in temperature\n");
    }
    if(night_day_for_temp==DAY)
    {
	/* log saying its day */
        printf("got day val from ls in temperature\n");
    }

    pthread_mutex_unlock(&temp_night_day);
}
void give_night_day_info_temp(int light_sensor_fd)
{
    /* get the luminosity */
    float lux;
    get_luminosity(light_sensor_fd, &lux);
    
    /* block on a condition variable */
    pthread_mutex_lock(&temp_night_day);

    if(lux>DAYTIME_THRESHOLD)
    {
        night_day_for_temp=DAY;
    }
    else
        night_day_for_temp=NIGHT;

    pthread_cond_signal(&temp_cond_nd);
    pthread_mutex_unlock(&temp_night_day);
}
void get_night_day_info_main()
{
    /* block on a condition variable */
    pthread_mutex_lock(&main_night_day);
    /* write to the logger here */
    pthread_cond_wait(&main_cond_nd, &main_night_day);
    /* read updated variable */

    if(night_day_for_main==NIGHT)
    {
    	/* log saying its night */
        printf("got night val from ls in main\n");
    }
    if(night_day_for_main==DAY)
    {
	/* log saying its day */
        printf("got day val from ls in maintask\n");
    }
    pthread_mutex_unlock(&main_night_day);
}

void give_night_day_info_main(int light_sensor_fd)
{
    /* get the luminosity */
    float lux;
    get_luminosity(light_sensor_fd, &lux);
    
    /* block on a condition variable */
    pthread_mutex_lock(&main_night_day);

    if(lux>DAYTIME_THRESHOLD)
        night_day_for_main=DAY;
    else
        night_day_for_main=NIGHT;

    pthread_cond_signal(&main_cond_nd);
    pthread_mutex_unlock(&main_night_day);
}

/*******************************************************************
*TASK NAME: Temperature Task
*
*PARAMETERS: None:Thread Id passed
*FUNCTION:  The function of this TASK is to log the temperature values
*			and send the Kelvin, Celcius and Fahrenhiet values
*
*EVENTS LOGS:Heartbeat sent
*			:Temperature Message packet
			:Decision Task result
*QUEUE_OPEN: LOGGER QUEUE 
*			 DECISION TASK
*
********************************************************************/

void *temperature(void *threadp){
    int temp_fd;
    log_message_t temp_message;
    thread_message_t received_message;
    float temp_value=0,temp_fahrenhiet=0,temp_kelvin=0;
    i2c_init(DEV_ADDR,&temp_fd);
    float resolution;
    float fahrenheit,celsius,kelvin;
    resolution=set_resolution(1,temp_fd);
    printf("resolution %f\n", resolution);
    temp_register(resolution,temp_fd,&fahrenheit,&celsius,&kelvin);
    printf("celsius value in temp %f ",celsius);
    temp_value = celsius;

    mqd_t logger_queue;
    mqd_t decision_queue;

    struct mq_attr logger_queue_attr;
    struct mq_attr decision_queue_attr;

    logger_queue_attr.mq_flags=0;
    decision_queue_attr.mq_flags=0;

    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    decision_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;

    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    decision_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    //sem_wait(&temp_sem);
    logger_queue=mq_open(LOG_QUEUE_NAME, O_RDWR, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in temperature thread open failed");
        exit(0);
    }
    decision_queue=mq_open(DECISION_QUEUE_NAME,O_RDWR,666,&decision_queue_attr);
    if(decision_queue==(mqd_t)-1){
	perror("mq in temperature thread open failed");
	exit(0);
    }
    task_initialise_msgpkt(&temp_message,"Temperature Sensor");
    if(mq_send(logger_queue, (char*)&temp_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
       }
  
	while(1){
		printf("resolution %f\n", resolution);
		temp_register(resolution,temp_fd,&fahrenheit,&celsius,&kelvin);
    		printf("celsius value in temp %f ",celsius);
    		temp_value = celsius;
    		temp_fahrenhiet=fahrenheit;
		temp_kelvin=kelvin;

    		log_ts_id(temp_value,temp_fahrenhiet,temp_kelvin,&temp_message);

		
		if(mq_send(logger_queue, (char*)&temp_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
	        {
        	    perror("mq_send failed");
            	    exit(0);
        	}

        	if(mq_send(decision_queue,(char*)&temp_message,MAX_MSG_SIZE_LOG_QUEUE,0)==-1){
        		perror("mq_send for decision failed");
        		exit(0);
        	}
		pthread_mutex_lock(&temp_lock);
		if(count_temp != 1){

			pthread_cond_signal(&tempcond);
			count_temp = 1;
			printf("temperature Thread sent signal\n");
		}
		pthread_mutex_unlock(&temp_lock);

                get_night_day_info_temp();
		task_req_res(&temp_message,"temperature thread","requesting light task");
		if(mq_send(logger_queue, (char*)&temp_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
       		 {
           		 perror("mq_send failed for decision queue");
           		 exit(0);
      		  }
	}
}

/*******************************************************************
*TASK NAME: Light Task
*
*PARAMETERS: None:Thread Id passed
*FUNCTION:  The function of this TASK is to record and process all
* 			the ligh values and send it to LOGGER, predict it as LIGHT and Day
*
*EVENTS LOGS:Checks for NIGHT/DAY
			:Gets the Luminosity value
			:sends heartbeat to main
*QUEUE_OPEN: LOGGER QUEUE 
*			 DECISION QUEUE
*
********************************************************************/

void *light_sensor(void *light_sensor_param)
{
    /* file descriptor */
    int light_sensor_fd;
    
    i2c_init(LIGHT_SENSOR_DEV_ADDR, &light_sensor_fd);
    float luminosity=0, luminosity_old=0,change_lux=0;
    turn_on_light_sensor(light_sensor_fd);
    get_luminosity(light_sensor_fd,&luminosity);

    log_message_t id_message;
    thread_message_t received_message;
    
    log_ls_id(luminosity, &id_message); 
    
    mqd_t logger_queue;
    mqd_t decision_queue;

    struct mq_attr logger_queue_attr;
    struct mq_attr decision_queue_attr;

    logger_queue_attr.mq_flags=0;
    decision_queue_attr.mq_flags=0;

    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    decision_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;

    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    decision_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;

    logger_queue=mq_open(LOG_QUEUE_NAME, O_RDWR, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in light thread open failed");
        exit(0);
    }

    decision_queue=mq_open(DECISION_QUEUE_NAME, O_RDWR, 666, &decision_queue_attr);
    if(decision_queue==(mqd_t)-1)
    {
        perror("mq in light thread open failed");
        exit(0);
    }
    task_initialise_msgpkt(&id_message,"Light Sensor");
    if(mq_send(logger_queue, (char*)&id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
       }
  
    while(1)
    {
	luminosity_old=luminosity;
	get_luminosity(light_sensor_fd,&luminosity);
	if(if_lux_change_log(luminosity_old, luminosity,&change_lux)==CHANGE_LOG_REQUIRED)
        {
            log_ls_change(change_lux,&id_message);
            printf("\n\nchange log reqd\n\n");
	    	        
        }
	else{
		log_ls_id(luminosity, &id_message);
	} 
	printf("The actual luminosity data %f\n",luminosity);
        if(mq_send(logger_queue, (char*)&id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
        }
        
        if(mq_send(decision_queue, (char*)&id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed for decision queue");
            exit(0);
        } 
        pthread_mutex_lock(&light_mutex);
		if(count_light != 1){
			

			pthread_cond_signal(&lightcond);
			count_light = 1;
			printf("light Thread sent signal\n");	

		}
	pthread_mutex_unlock(&light_mutex);
	give_night_day_info_temp(light_sensor_fd);
	task_req_res(&id_message,"light thread","responding");
	if(mq_send(logger_queue, (char*)&id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed for decision queue");
            exit(0);
        }
    
    }
     
        
    pthread_exit(0);
}
/*******************************************************************
*TASK NAME: decision Task
*
*PARAMETERS: None:Thread Id passed
*FUNCTION  : The function receives data continously from Temperature
*			and Light task. Makes the USR led glow if the value of 
*           data crosses a certain threshold.
*
*EVENTS    : Checks the threshold value and logs to the LOGGER QUEUE
			 Glows the USR Led as it crosses above or below threshold value
			 Sends Heartbeat to main
*QUEUE_OPEN: LOGGER QUEUE 
*			 DECESION QUEUE
*
********************************************************************/
void *decision(void *threadp){
	//pthread_mutex_t htimelock;
    mqd_t decision_queue;
    mqd_t logger_queue;
    char *parsing_buffer;
    char *decision_value;
    char *task;
    log_message_t received_decision;
    log_message_t log_decision;
    struct mq_attr decision_queue_attr;
    struct mq_attr logger_queue_attr;
    decision_queue_attr.mq_flags=0;
    logger_queue_attr.mq_flags=0;
    decision_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    decision_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;

    decision_queue=mq_open(DECISION_QUEUE_NAME, O_RDONLY, 666, &decision_queue_attr);
    if(decision_queue==(mqd_t)-1)
    {
        perror("mq in decision failed");
        exit(0);
    }
    logger_queue=mq_open(LOG_QUEUE_NAME, O_RDWR, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in light thread open failed");
        exit(0);
    }
    task_initialise_msgpkt(&log_decision,"Decision Task initialised");
    if(mq_send(logger_queue, (char*)&log_decision, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
       }

	while(1){
		
		
        //printf("the message received:%s\n", received_decision.message);
		pthread_mutex_lock(&decision_mutex);		
		if(count_decision != 1){
			

			pthread_cond_signal(&decisioncond);
			count_decision = 1;
			printf("decision Thread sent signal\n");	

		}
		pthread_mutex_unlock(&decision_mutex);
		if(mq_receive(decision_queue, (char*)&received_decision, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
        	{
            		perror("mq_receive failed");
            		exit(0);
        	}
        	printf("the message received:%s\n", received_decision.message);
		parsing_buffer= received_decision.message;
		parsing_buffer=strtok(parsing_buffer," ");
                task = parsing_buffer;
		printf("\n\n%s\n\n",task);
		while(parsing_buffer!=NULL){
			//printf("%s\n",parsing_buffer);
			decision_value=parsing_buffer;
			parsing_buffer=strtok(NULL," ");	
		}
		int val =atoi(decision_value);
		
		if(strcmp(task,"light")==0){
			printf("Before comaprision\n");
			if(val<40 || val>3 ){
				printf("normal light values\n");
			}
			else{
				printf("alert: LED will glow\n");
				detection_led();
			}	
		}
		if(strcmp(task,"temperature")==0){
			if(val<28 ){
				printf("normal temperature\n");
			}
			else{
				printf("alert: LED will glow\n");
				detection_led();
			}
		}	

	}
}
/*******************************************************************
*TASK NAME: Logger Task
*
*PARAMETERS: None:Thread Id passed
*FUNCTION:  The function of this TASK is to log all the posible events 
*			during executiion
*
*EVENTS LOGS:Heartbeat
*			:Temperature Message packet
*			:Light Message packet
*			:Sudden Change in Light temperture
			:NIGHT OR DAY message when asked
			:Initialisation/Termination of a task
			:Decision Task result
*QUEUE_OPEN: LOGGER QUEUE 
*
********************************************************************/

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
        
    log_message_t received_log;
    logfile = fopen(filepath,"w+");

    while(1)
    {
        
        if(mq_receive(logger_queue, (char*)&received_log, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
        {
            perror("mq_receive failed");
            exit(0);
        }
        
        printf("the message received:%s\n", received_log.message); 
	
	
        fprintf(logfile,"[%ld.%ld],log level:%d,thread name:%s ,%s\n",received_log.timestamp.tv_sec,received_log.timestamp.tv_usec,\
received_log.log_level,received_log.thread_name,received_log.message);
	
	

        if(count_log != 1){
			

			pthread_cond_signal(&logcond);
			count_log = 1;
			printf("log Thread sent signal\n");	

		}

    }
    pthread_exit(0);
}

void main(int argc,char **argv){
    int ret=0,rc=0;
    filepath=argv[1];
/*Message Structure for heartbeat*/
    log_message_t hb_message;
    
    struct timespec heartbeat_time;
    struct timeval tp;
   /*message queue initialisations*/
	
    mqd_t logger_queue;
    mqd_t decision_queue;

    struct mq_attr logger_queue_attr;
    struct mq_attr decision_queue_attr;

    logger_queue_attr.mq_flags=0;
    decision_queue_attr.mq_flags=0;

    logger_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    decision_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;

    logger_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    decision_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;

    /*Logger queue Created and Opened*/
    logger_queue=mq_open(LOG_QUEUE_NAME, O_CREAT|O_RDWR, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in logger failed");
        exit(0);
    }
    /* Decision Queue Created and Opened*/
    /* creation of Threads*
    *Logger thread created*
    *Temperature thread created*
    *Light thread created*
    *Decision thread created**/
    decision_queue=mq_open(DECISION_QUEUE_NAME,O_CREAT|O_RDWR,0666,&decision_queue_attr);
    if(decision_queue==(mqd_t)-1){
    	perror("mq in decision failed in creation");
    	exit(0);
    }
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

    
	if(ret = pthread_create(&temperature_sensor_td,(void *)0,temperature,(void *)0)<0){
		perror("Thraed not created\n");
		exit(1);

	}
		
	if(ret = pthread_create(&decision_td,(void *)0,decision,(void *)0)<0){
		perror("Thread not created\n");
		exit(1);
	}
        
	signal(SIGINT,int_handler);
        
	while(1){
	/* HEARTBEAT OF TEMPERATURE TASK TO MAIN*/
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
		
	/*HEARTBEAT OF DECISION TASK TO MAIN*/
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
    /*HEARTBEAT OF LOGGER TASK TO MAIN*/
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
	/*HEARTBEAT OF LIGHT TASK TO MAIN*/
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
		
	}
	/*pthreads are joined after the while loop*/
	pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
	pthread_join(temperature_sensor_td,NULL);
	pthread_join(decision_td,NULL);
}
