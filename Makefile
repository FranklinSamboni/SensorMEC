lib = libs/

sacdir = $(lib)SAC_FILES/
adcdir = $(lib)ADC/
gpsdir = $(lib)GPS/
rtcdir = $(lib)RTC/
gpiodir = $(lib)GPIO/
socketdir = $(lib)SOCKET/
jsonfilesdir = $(lib)JSON_FILES/
eventsdir = $(lib)EVENTS/

MAIN = SensorIoT.c

CFLAGS =  -O0 -g3 -Wall -c -fmessage-length=0  -lm


OBJECTS = $(eventsdir)staLta.o $(jsonfilesdir)filesJ.o $(adcdir)adc.o $(gpsdir)gps.o $(rtcdir)rtc.o $(gpiodir)gpio.o $(sacdir)sacsubc.o  $(socketdir)socketlib.o 
OBJECTSRTC =  $(rtcdir)rtc.o $(gpiodir)gpio.o  $(socketdir)socketlib.o
OBJECTSGPS =  $(gpsdir)gps.o $(gpiodir)gpio.o  $(socketdir)socketlib.o
OBJECTSADC =  $(adcdir)adc.o $(gpiodir)gpio.o  $(socketdir)socketlib.o

.PHONY : all clean
all: SensorIoT

SensorIoT : $(OBJECTS)
	gcc -O0 -g3 -Wall tests/testGps.c -o tests/testGps $(OBJECTSGPS) -lrt -l json -lm
	gcc -O0 -g3 -Wall tests/testAdc.c -o tests/testAdc $(OBJECTSADC) -lrt -l json -lm
	gcc -O0 -g3 -Wall tests/testRtc.c -o tests/testRtc $(OBJECTSRTC) -lrt -l json -lm
	gcc -O0 -g3 -Wall $(MAIN) -o SensorIoT $(OBJECTS) -lrt -l json -lm 

$(eventsdir)staLta.o : $(eventsdir)staLta.c $(eventsdir)
	gcc $(CFLAGS) -o $(eventsdir)staLta.o -c $(eventsdir)staLta.c

$(jsonfilesdir)filesJ.o : $(jsonfilesdir)filesJ.c $(jsonfilesdir)
	gcc $(CFLAGS) -o $(jsonfilesdir)filesJ.o -lrt -l json -c $(jsonfilesdir)filesJ.c  

$(sacdir)sacsubc.o : $(sacdir)sacsubc.c $(sacdir)
	gcc $(CFLAGS) -o $(sacdir)sacsubc.o -c $(sacdir)sacsubc.c 

$(adcdir)adc.o : $(adcdir)adc.c $(adcdir)
	gcc $(CFLAGS) -o $(adcdir)adc.o -c $(adcdir)adc.c  

$(gpsdir)gps.o : $(gpsdir)gps.c $(gpsdir)
	gcc $(CFLAGS) -o $(gpsdir)gps.o -c $(gpsdir)gps.c

$(rtcdir)rtc.o : $(rtcdir)rtc.c $(rtcdir)
	gcc $(CFLAGS) -o $(rtcdir)rtc.o -c $(rtcdir)rtc.c 

$(gpiodir)gpio.o : $(gpiodir)gpio.c $(gpiodir)
	gcc  $(CFLAGS) -o $(gpiodir)gpio.o -c $(gpiodir)gpio.c 
	
$(socketdir)socketlib.o : $(socketdir)socketlib.c $(socketdir)
	gcc $(CFLAGS) -o $(socketdir)socketlib.o -c $(socketdir)socketlib.c 



clean :
	-rm SensorIoT $(OBJECTS)