#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include "../includes/light_sensor.h"

#include "Unity/src/unity.h"

#define FILE_NAME "light_sensor_ut_results.txt"

FILE* fp;
int light_sensor_fd;

void test_turn_on_light_sensor()
{
    /* call the function */
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, turn_on_light_sensor(light_sensor_fd), "returns incorrect code when called with apt arguments");
    
    /* call the function with an incorrect descriptor */
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, turn_on_light_sensor(light_sensor_fd-1), "returns incorrect code when called with an incorrect fd");
    
 

}
void test_write_timing_reg()
{
    gain_t gain;
    uint8_t if_manual=0;
    integ_time_t integ_time=MIN_INTEG_TIME;

    /* call the function with legal arguments */
    for(gain=MIN_GAIN; gain<=MAX_GAIN; gain++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, write_timing_reg(light_sensor_fd, gain, integ_time, if_manual),\
            "returns incorrect code when called with apt arguments");
        
    
    }
    
    /* reset the gain */
    gain=MIN_GAIN;

    /* call the function varying the integ time arg within bounds */
    for(integ_time=MIN_INTEG_TIME; integ_time<=INTEG_NA; integ_time++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, write_timing_reg(light_sensor_fd, gain, integ_time, if_manual),\
            "returns incorrect code when called with apt arguments");
    
    }
    /* call with illegal arguments */
    for(integ_time=INTEG_NA+1; integ_time<=7; integ_time++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, write_timing_reg(light_sensor_fd, gain, integ_time, if_manual),\
            "returns incorrect code when called with in apt arguments");
    }

    /* call with illegal arguments */
    for(gain=MAX_GAIN+1; gain<10; gain++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, write_timing_reg(light_sensor_fd, gain, integ_time, if_manual),\
            "returns incorrect code when called with in apt arguments");
    }
    int ls_fd;
     
    /* call with an in correct file descriptor */ 
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, write_timing_reg(ls_fd, MIN_GAIN, MIN_INTEG_TIME, if_manual),\
            "returns incorrect code when called with in apt arguments");

    /* call with an in correct if_manual val */ 
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, write_timing_reg(light_sensor_fd, MIN_GAIN, MIN_INTEG_TIME, 5),\
            "returns incorrect code when called with in correct if_manual vals");

}

void test_read_timing_reg()
{
    gain_t gain;
    uint8_t if_manual;
    integ_time_t integ_time;

    /* call the function with legal arguments */
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, read_timing_reg(light_sensor_fd, &gain, &integ_time, &if_manual),\
            "returns incorrect code when called with apt arguments");
    
    /* call the function with NULL ptrs */

    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_timing_reg(light_sensor_fd, NULL, &integ_time, &if_manual),\
            "returns incorrect code when called with in apt arguments");
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_timing_reg(light_sensor_fd, &gain, NULL, &if_manual),\
            "returns incorrect code when called with in apt arguments");
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_timing_reg(light_sensor_fd, &gain, &integ_time, NULL),\
            "returns incorrect code when called with in apt arguments");
    
    /* call with random fd*/ 
    int random;
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_timing_reg(random, &gain, &integ_time, &if_manual),\
            "returns incorrect code when called with random fd");

    
}

void test_read_id_reg()
{
    /* call function with valid args */
    uint8_t part_no, rev_no;

    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, read_id_reg(light_sensor_fd, &part_no, &rev_no),\
            "returns incorrect code when called with apt arguments");
    
    /* call function with args that are ptrs to NULL*/

    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_id_reg(light_sensor_fd, NULL, &rev_no),\
            "returns incorrect code when called with incorrect arguments");
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_id_reg(light_sensor_fd, &part_no, NULL),\
            "returns incorrect code when called with incorrect arguments");

    /* call function with incorrect file descriptor */
    int random;
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, read_id_reg(random, &part_no, &rev_no),\
            "returns incorrect code when called with incorrect fd");
}

void test_read_ic_reg()
{
    ic_t interrupts;
    uint8_t number_of_cycles;

    /* call function with valid args */
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, light_sensor_read_ic_register(light_sensor_fd, &interrupts, &number_of_cycles),\
            "returns incorrect code when called with apt arguments");
    
    /* call function with args that are ptrs to NULL*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, light_sensor_read_ic_register(light_sensor_fd, NULL, &number_of_cycles),\
            "returns incorrect code when called with NULL ptrs");

    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, light_sensor_read_ic_register(light_sensor_fd, &interrupts, NULL),\
            "returns incorrect code when called with NULL ptrs");
                
    /* call function with incorrect file descriptor */
    int random;
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, light_sensor_read_ic_register(random, &interrupts, &number_of_cycles),\
            "returns incorrect code when called with incorrect fd");
    
}

void test_write_ic_reg()
{
    /* call function with valid args */
    uint8_t number_of_cycles;
    
    for(number_of_cycles=0; number_of_cycles<16; number_of_cycles++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, light_sensor_write_ic_register(light_sensor_fd, INTERRUPT_ENABLE, number_of_cycles),\
            "returns incorrect code when called with apt arguments");
    }
    
    for(number_of_cycles=0; number_of_cycles<16; number_of_cycles++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, light_sensor_write_ic_register(light_sensor_fd, INTERRUPT_DISABLE, number_of_cycles),\
            "returns incorrect code when called with apt arguments");
    }
    
    /* call function with invalid args */
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, light_sensor_write_ic_register(light_sensor_fd, INTERRUPT_DISABLE, 16),\
            "returns incorrect code when called with in-correct n.o.c");
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, light_sensor_write_ic_register(light_sensor_fd, (ic_t)(INTERRUPT_DISABLE+1), 15),\
            "returns incorrect code when called with in-correct ic_t variable");
    

}

void test_get_luminosity()
{
    float luminosity;

    if(write_timing_reg(light_sensor_fd, MAX_GAIN, MAX_INTEG_TIME, 0)!=SUCCESS)
    {
        perror("write to tmg reg failed\n");
        exit(0);
    }
#ifdef WELL_LIT_ROOM
    printf("luminosity in a well lit room\n");
#else 
    printf("luminosity with the light blocked\n");
#endif
    for(int i=0; i<10; i++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, get_luminosity(light_sensor_fd, &luminosity),\
            "returns incorrect code when called with apt arguments");
        printf("luminosity val:%f\n", luminosity);
        sleep(1);
    }

    /* call func with a NULL ptr*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, get_luminosity(light_sensor_fd, NULL),\
            "returns incorrect code when called with NULL ptr");

}

int main()
{
    fp=fopen(FILE_NAME, "w");
    
    /* initialize the sensor */
    i2c_init(LIGHT_SENSOR_DEV_ADDR, &light_sensor_fd);
    
    if(fp==NULL)
    {
        perror("file opening failed\n");
        exit(0);
    }
    
    UNITY_BEGIN();
    
    RUN_TEST(test_turn_on_light_sensor);
    
    RUN_TEST(test_write_timing_reg);
    RUN_TEST(test_read_timing_reg);
    RUN_TEST(test_read_timing_reg);
    
    RUN_TEST(test_read_id_reg);
    
    RUN_TEST(test_read_ic_reg);
    
    RUN_TEST(test_write_ic_reg);
    
    RUN_TEST(test_get_luminosity);
    
    fclose(fp);
    
    return UNITY_END();
}
