#File			: makefile
#
#Author			: Sahana Sadagopan, CU, ECEE
# 				  Ashwath Gundepally, CU, ECEE
#
#Description	: makefile for the project 		
				



LIBS=-pthread -lrt -lm

HFILES=

CFILES1= light_sensor.c 
CFILES2= i2c.c
CFILES3= USRled.c
CFILES4= log.c
CFILES5= temp.c
#CFILES6= temp_i2c.c
CFILES7= main.c

CC=gcc

CFLAGS=-c 

all: main

main: main.o i2c.o light_sensor.o USRled.o log.o temp.o 
	$(CC) light_sensor.o main.o i2c.o USRled.o temp.o log.o $(LIBS) -o main
	
light_sensor.o: $(CFILES1)
	$(CC) $(CFLAGS) $(LIBS) $(CFILES1)

i2c.o: $(CFILES2)
	$(CC) $(CFLAGS) $(CFILES2)

USRled.o: $(CFILES3)
	$(CC) $(CFLAGS) $(LIBS) $(CFILES3)

log.o: $(CFILES4)
	$(CC) $(CFLAGS) $(LIBS) $(CFILES4)

temp.o: $(CFILES5)
	$(CC) $(CFLAGS) $(LIBS) $(CFILES5)

#temp_i2c.o: $(CFILES6)
#	$(CC) $(CFLAGS) $(LIBS) $(CFILES6)

main.o: $(CFILES7)
	$(CC) $(CFLAGS) $(LIBS) $(CFILES7)  





clean:
	rm -rf *.o *.d main
