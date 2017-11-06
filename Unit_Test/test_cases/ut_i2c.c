#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include "../includes/light_sensor.h"

#include "Unity/src/unity.h"

#define MAX_THREE_BYTE 16777216
#define MAX_BYTE       256 

void test_i2c_init()
{
    int fd;
    /* call func with valid dev addr */ 
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, i2c_init(LIGHT_SENSOR_DEV_ADDR, &fd),\
            "returns incorrect code when called with valid args");
    /*call func with ptr to NULL*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, i2c_init(LIGHT_SENSOR_DEV_ADDR, NULL),\
            "returns incorrect code when called with NULL ptr");

}
void test_i2c_write()
{
    int fd;

    if(i2c_init(LIGHT_SENSOR_DEV_ADDR, &fd)!=SUCCESS)
    {
        perror("init failed \n");
        exit(1);
    }
    /* call func with valid args */
    uint16_t byte;
    for(byte=0; byte<MAX_BYTE; byte++)
    {    
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, i2c_write((uint8_t*)&byte, fd), "returns incorrect val when called with valid args");
    }

    /*call with ptr to NULL*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, i2c_write(NULL, fd), "returns incorrect val when called with NULL");

}
void test_i2c_read()
{
    int fd;

    if(i2c_init(LIGHT_SENSOR_DEV_ADDR, &fd)!=SUCCESS)
    {
        perror("init failed \n");
        exit(1);
    }
    /* call func with valid args */
    uint8_t byte;
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, i2c_read(&byte, fd), "returns incorrect val when called with valid args");

    /*call with ptr to NULL*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, i2c_read(NULL, fd), "returns incorrect val when called with NULL");
    
}
void test_i2c_write_word()
{
    int fd;

    if(i2c_init(LIGHT_SENSOR_DEV_ADDR, &fd)!=SUCCESS)
    {
        perror("init failed \n");
        exit(1);
    }
    /* call func with valid args */
    uint32_t word;
    for(word=0; word<1000; word++)
    {    
        TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, i2c_write_word((uint8_t*)(&word), fd), "returns incorrect val when called with valid args");
    }

    /*call with ptr to NULL*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, i2c_write_word(NULL, fd), "returns incorrect val when called with NULL");

}
void test_i2c_read_word()
{
    int fd;

    if(i2c_init(LIGHT_SENSOR_DEV_ADDR, &fd)!=SUCCESS)
    {
        perror("init failed \n");
        exit(1);
    }
    /* call func with valid args */
    uint16_t word;

    TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, i2c_read_word((uint8_t*)&word, fd), "returns incorrect val when called with valid args");

    /*call with ptr to NULL*/
    TEST_ASSERT_EQUAL_INT_MESSAGE(FAILURE, i2c_read_word(NULL, fd), "returns incorrect val when called with NULL");
    
}
int main()
{
    
    UNITY_BEGIN();
    
    RUN_TEST(test_i2c_init);

    RUN_TEST(test_i2c_write);

    RUN_TEST(test_i2c_read);    
    
    RUN_TEST(test_i2c_write_word); 
    
    RUN_TEST(test_i2c_read_word); 

    return UNITY_END();
}
