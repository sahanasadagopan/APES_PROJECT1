# APES_PROJECT1: Inter-Threaded Communication and Logging of Sensor Data on the BBG 
## Description:
Threads are created to call driver functions from a Temperature sensor (TMP 102), and Luminosity Sensor (AV02-2315EN0), and
log this data to another thread-the Logger thread- which continually writes messages it receives to a log file whose name can 
be specified on the command line as an argument. The main task, which creates these threads also receives heartbeat
notification from all the threads. Every task also logs initialisation and termination notification.
The driver for each sensor is local to only one thread- sensor data that is requested by another thread, is sent 
as a message, and not by simply reading from the driver functions. In this regard, Night/Day info which is requested 
by the main thread and the temperature thread is sent by the Light thread. 
Another thread, a decision thread, continually checks sensor data received from the temperature and light thread to see if it
is within bounds. And lights an LED on the device when it exceeds bounds.
All the threads communication is continually logged to a file.
### File wise detail:
#### Source files:
src/i2c.c: a generic i2c driver that communicates that is used by both the sensors. It contains basic routines like 
i2c (read/write) (byte/word), and i2c init.
src/light_sensor: high-level driver for the light sensor. Calls routines in i2c.c to read/write to all the registers on the
sensor.
src/temp.c:high-level driver for the temperature sensor. Calls routines in i2c.c to read/write to all the registers on the
sensor.
src/log.c: Contains routines that enable inter thread communication among the threads.
src/main.c: Contains all the thread functions and the message queue creation that is used by log.c to facilitate communication.
#### Header files:
The includes folder contains all the corresponding header files that are referenced by the above files, example, log.h
contains the message packet struct that is sent to the logger thread which is then logged, it also contains the message
packet structure that is passed among threads.

#### Unit Tests:
This folder contains all the src files mentioned above and in addition, it also contains the following test runner:
light_sensor_ut.c: test runner for the light sensor driver.
unit_tests/ut_i2c.c: test runner for the low-level i2c driver.
unit_tests/makefile: makefile for the unit tests. Do make to generate the executable for the light sensor driver test runner.
Do a 'make ut_i2c' to generate the executable i2c_ut for the test runner low level i2c.c file.

## Prerequisites:
1. The Beagle Bone Green
2. Temperature Sensor: TMP 102
3. Luminosity Sensor: AV02-2315EN0

## Compiling:
The makefile for the project can be found in the src directory, do a make command to generate the executable by the name
'main'.

## Authors: Sahana Sadagopan, Ashwath Gundepally, CU, ECEE
