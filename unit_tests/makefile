#File			: makefile
#Author			: Ashwath Gundepally, CU, ECEE
#				  Sahana Sadagopan, CU, ECEE
#Description	: makefile for the unit tests				  		
				

#INCLUDE_DIRS = Unity/src/unity.h
LIB_DIRS = 
#gcc test_dll.c doubly_ll.c Unity/src/unity.c -o first

CDEFS=
LIBS=-pthread -lrt -lm

HFILES=

CFILES1= light_sensor.c 
CFILES2= i2c.c
CFILES3= Unity/src/unity.c
CFILES4=light_sensor_ut.c

CC=gcc

CFLAGS=-c 

all: light_sensor_ut

light_sensor_ut: light_sensor_ut.o light_sensor.o i2c.o unity.o
	$(CC) light_sensor.o light_sensor_ut.o i2c.o unity.o $(LIBS) -o out

ut_i2c: ut_i2c.o i2c.o unity.o
	$(CC) ut_i2c.o i2c.o unity.o $(LIBS) -o i2c_out
	

ut_i2c.o: ut_i2c.c
	$(CC) $(CFLAGS) $(LIBS) ut_i2c.c

light_sensor.o: light_sensor.c
	$(CC) $(CFLAGS) $(LIBS) light_sensor.c

i2c.o: i2c.c
	$(CC) $(CFLAGS) i2c.c

unity.o: Unity/src/unity.c
	$(CC) $(CFLAGS) Unity/src/unity.c

clean:
	rm -rf *.o *.d out
