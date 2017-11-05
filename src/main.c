#include "../includes/light_sensor.h"
#include "../includes/i2c.h"
#include "../includes/log.h"
#include "../includes/temp.h"
#include "../includes/USRled.h"

/* Global pthread variable */
pthread_t temperature_sensor_td;
pthread_t logger_td;
pthread_t light_sensor_td;
pthread_t decision_td;

sem_t temp_sem;


pthread_mutex_t temp_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t decision_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t light_mutex=PTHREAD_MUTEX_INITIALIZER;

/*Pthread Attributes*/
pthread_attr_t temperature_attr;
pthread_attr_t logger_attr;
pthread_attr_t light_attr;
pthread_attr_t decision_attr;

/*struct params*/
struct sched_param temp_params;

/*sigatomic*/
static volatile sig_atomic_t count_temp=1;
static volatile sig_atomic_t count_decision=1;
static volatile sig_atomic_t count_log=1;
static volatile sig_atomic_t count_light=1;
pthread_cond_t decisioncond;
pthread_cond_t tempcond;
pthread_cond_t logcond;
pthread_cond_t lightcond;

#define WAIT_TIME_SECONDS 1

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

void *temperature(void *threadp){
    //int temp_fd;
    uint8_t temp_value= 25;
    message_t temp_message;
    //sensor_initialization(temp_fd,temp_value);
    log_ts_id(temp_value, &temp_message); 
    
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


	while(1){	

		if(mq_send(logger_queue, (char*)&temp_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed");
            exit(0);
        }

        if(mq_send(decision_queue,(char*)&temp_message,MAX_MSG_SIZE_LOG_QUEUE,0)==-1){
        	perror("mq_send for decision failed");
        	exit(0);
        }

        //printf("the message sent:%s\n", temp_message.message);
		pthread_mutex_lock(&temp_lock);
		//printf("temperature thread got mutex\n");
		if(count_temp != 1){

			pthread_cond_signal(&tempcond);
			count_temp = 1;

			printf("temperature Thread sent signal\n");
		}
		pthread_mutex_unlock(&temp_lock);
	}
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
        perror("mq in light thread open failed");
        exit(0);
    }
    decision_queue=mq_open(DECISION_QUEUE_NAME, O_RDWR, 666, &decision_queue_attr);
    if(decision_queue==(mqd_t)-1)
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
        
        if(mq_send(decision_queue, (char*)&id_message, MAX_MSG_SIZE_LOG_QUEUE, 0)==-1)
        {
            perror("mq_send failed for decision queue");
            exit(0);
        }
        //printf("the message sent:%s\n", id_message.message); 
        pthread_mutex_lock(&light_mutex);
		//printf("decision Thread got mutex\n");
		
		if(count_light != 1){
			

			pthread_cond_signal(&lightcond);
			count_light = 1;
			printf("light Thread sent signal\n");	

		}
		pthread_mutex_unlock(&light_mutex);
    }
     
        
    pthread_exit(0);
}

void *decision(void *threadp){
	//pthread_mutex_t htimelock;
	mqd_t decision_queue;
	char *parsing_buffer;
	char *decision_value;
	message_t received_decision;
    struct mq_attr decision_queue_attr;
    decision_queue_attr.mq_flags=0;
    decision_queue_attr.mq_maxmsg=MAX_MSGS_IN_LOG_QUEUE;
    decision_queue_attr.mq_msgsize=MAX_MSG_SIZE_LOG_QUEUE;
    
    decision_queue=mq_open(DECISION_QUEUE_NAME, O_RDONLY, 666, &decision_queue_attr);
    if(decision_queue==(mqd_t)-1)
    {
        perror("mq in decision failed");
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
		parsing_buffer= received_decision.message;
		parsing_buffer=strtok(parsing_buffer," ");
		while(parsing_buffer!=NULL){
			printf("%s\n",parsing_buffer);
			decision_value=parsing_buffer;
			parsing_buffer=strtok(NULL," ");	
		}
		int val =atoi(decision_value);
		if(val<30){
			printf("normal temperature\n");
		}
		else{
			printf("alert: LED will glow\n");
			detection_led();
		}
		

	}
}


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
    //sem_post(&temp_sem);
    
    message_t received_log;
   

    while(1)
    {
        
        if(mq_receive(logger_queue, (char*)&received_log, MAX_MSG_SIZE_LOG_QUEUE, NULL)==-1)
        {
            perror("mq_receive failed");
            exit(0);
        }
        
        //printf("the message received:%s\n", received_log.message); 


        if(count_log != 1){
			

			pthread_cond_signal(&logcond);
			count_log = 1;
			printf("log Thread sent signal\n");	

		}

    }
    pthread_exit(0);
}

void main(int argc,char **argv){
	int ret,rt_max_prio,rt_min_prio,rc;
	struct sched_param main_param;
	pthread_attr_t main_attr;
	pid_t mainpid;
    
    struct timespec heartbeat_time;
    struct timeval tp;

    /*message queue initialisations*/

	/*mainpid=getpid();
	rc=sched_getparam(mainpid,&main_param);
	if(rc){
		printf("EROOR:sched_setscheduler %d\n",rc );
		exit(-1);
	}

	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
	rt_min_prio = sched_get_priority_min(SCHED_FIFO);
	main_param.sched_priority=rt_max_prio;
	rc=sched_setscheduler(getpid(),SCHED_FIFO,&main_param);
	if(rc<0) perror("Main program scheduling:\n");
*/
    /*pthread_attr_init(&decision_attr);
	pthread_attr_setinheritsched(&decision_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&decision_attr,SCHED_FIFO);
	temp_params.sched_priority =  rt_max_prio-2;*/
	message_t hb_message;
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

    logger_queue=mq_open(LOG_QUEUE_NAME, O_CREAT|O_RDWR, 666, &logger_queue_attr);
    if(logger_queue==(mqd_t)-1)
    {
        perror("mq in logger failed");
        exit(0);
    }

    decision_queue=mq_open(DECISION_QUEUE_NAME,O_CREAT|O_RDWR,0666,&decision_queue_attr);
    if(decision_queue==(mqd_t)-1){
    	perror("mq in decision failed in creation");
    	exit(0);
    }
    //sem_init(&temp_sem, 0, 0);
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

    
	if(ret = pthread_create(&temperature_sensor_td,(void *)0,temperature,(void *)0)<0)
		perror("Thraed not created\n");
	if(ret = pthread_create(&decision_td,(void *)0,decision,(void *)0)<0)
		perror("Thread not created\n");
	while(1){
		printf("here in main\n");

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
	pthread_join(logger_td, NULL);
	pthread_join(light_sensor_td, NULL);
	pthread_join(temperature_sensor_td,NULL);
	pthread_join(decision_td,NULL);
}
