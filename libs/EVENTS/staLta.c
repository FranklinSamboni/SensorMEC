#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "staLta.h"

#define EVENTS_DIR_R "/home/debian/Sensor-IOT/SensorIoT/eventos"
#define MAX_LENGTH_EVENT 144000
/*
staSeconds	->   Ventana de tiempo STA
ltaSeconds 	-> 	 Ventana de tiempo LTA
thOn 		->	 Disparador de evento "ON" cuando la relacion STA/LTA excede su valor
thOff		-> 	 Parador de evento "OFF" cuando la relacion STA/LTA es menor que su valor
minimunDurationSeconds -> La duracion del disparo debe exceder su valor para ser almacenada
*/

paramsSTA_LTA params;
//fullDate startTime;

//depValues strDepValues;

int eventNum = 0;
int EVENT_ON = 0;
int eventStart=0;
int eventEnd=0;

float eventSamples[MAX_LENGTH_EVENT] = {0};
int countEventSamples = 0;

float sta[144000] = {0};
float lta[144000] = {0};
float sta_to_lta[144000] = {0};

int countLTA_STA = 0;

int countTempData = 0;
float tempData[14400] = {0};


void defaultParams(int freq){

	params.freq = freq;
	params.lengthSTA = 0.7*freq;
	params.lengthLTA = 7.0*freq;
	params.thOn = 3.0;
	params.thOff = 1.5;
	params.minimunDurationSeconds = 2.0;
	printf("defaultParams - freq %d, lengthSTA %d, lengthLTA %d, thOn %f, thOff %f, minimunDurationSeconds %f\n", params.freq, params.lengthSTA, params.lengthLTA, params.thOn, params.thOff, params.minimunDurationSeconds);
	//params.ltaMode = "";

}

void setParamsSTA_LTA(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, float minimunDurationSeconds){

	params.freq = freq;
	params.lengthSTA = staSeconds*freq;
	params.lengthLTA = ltaSeconds*freq;
	params.thOn = thOn;
	params.thOff = thOff;
	params.minimunDurationSeconds = minimunDurationSeconds;
	//params.ltaMode = "";
	printf("freq %d, lengthSTA %d, lengthLTA %d, thOn %f, thOff %f, minimunDurationSeconds %f\n", params.freq, params.lengthSTA, params.lengthLTA, params.thOn, params.thOff, params.minimunDurationSeconds);
}

void sta_lta(float * newSamples, char * axis, char * date, char * time, int isGPS){

	int count = 0;
	while(count != params.freq){
		if(countTempData != (params.lengthLTA)){
			/* Al comenzar el script se debe inicializar el buffer temporal de los datos y las ventanas
			 * LTA y STA que se inicializan con inizializeFirstSamples cuando se ha completado el buffer de
			 * datos temporales.
			 */
			tempData[countTempData] = newSamples[count];

			if(countTempData == (params.lengthLTA - 1)){
				printf("ultimo valor de tempData %f - counttempData es : %d\n", tempData[countTempData], countTempData );
				printf("se llamo a inizializeFirstSamples\n");
				inizializeFirstSamples(tempData);
			}
			countTempData++; // countTempData al final sera igual a params.lengthLTA
		}
		else{

			if(countLTA_STA == params.lengthLTA){
				countLTA_STA = 0;
			}

			if(countLTA_STA == 0 && count == 0){
				sta[0] = sta[params.lengthLTA - 1] + ((newSamples[0] - tempData[countTempData - params.lengthSTA]) / params.lengthSTA);
				lta[0] = lta[params.lengthLTA - 1] + ((newSamples[0] - tempData[0]) / params.lengthLTA); // tempData[0] = tempData[countTempData - params.lengthLTA +  count]
				sta_to_lta[0] = sta[0] / lta[0];
				//detectEvent(count, newSamples[count],axis,date,time,isGPS);
			}

			/*else if(countLTA_STA == 0 && count != 0){
				printf("ERROR - EL PARAMETRO DE LTA SECONDS NO PUEDE TENER DECIMALES. EJEM (ltaSeconds = 8.5s) formato incorrecto.")
			}*/


			else{
				sta[countLTA_STA] = sta[countLTA_STA - 1] + ((newSamples[count] - tempData[countTempData - params.lengthSTA + count]) / params.lengthSTA);
				lta[countLTA_STA] = lta[countLTA_STA - 1] + ((newSamples[count] - tempData[count]) / params.lengthLTA);
				sta_to_lta[countLTA_STA] = sta[countLTA_STA] / lta[countLTA_STA];
				//detectEvent(count, newSamples[count],axis,date,time,isGPS);
			}

			printf("count: %d - countLTA_STA: %d - temp[0]: %f - temp[40]: %f - sta: %f - lta: %f - sta_to_lta : %f\n", count,countLTA_STA , tempData[0] , tempData[1], sta[countLTA_STA], lta[countLTA_STA], sta_to_lta[count]);

			movePositionRegister(newSamples);

			countLTA_STA++;
		}

		count++;
	}
	printf("count staLta es %d\n", count);
}

void movePositionRegister(float * newSamples){

	/*  Se mueve el buffer tempData una posicion a la izquierda.
	 *  ( ej. tempData[0] = tempData [1]))
	 *  	  tempData[1] = tempData [2])) .... y se pasan las nuevas muestras al final del buffer.
	 */

	int count = 0;
	int positionOfNewSamples =  params.lengthLTA - params.freq;
	while (count < params.lengthLTA){
		tempData[count] = tempData[params.freq + count];
		if(count >= (params.lengthLTA - params.freq)){
			tempData[count] = newSamples[count - positionOfNewSamples];
		}
		count ++;
	}
}

void inizializeFirstSamples(float * tempData){
	int count = 0;
	sta[count] = tempData[count] / params.lengthSTA;
	lta[count] = tempData[count] / params.lengthSTA;
	sta_to_lta[count] = sta[count] / lta[count];

	printf("inizializa count : %d - sta: %f - lta: %f - sta_to_lta : %f \n", count, sta[count], lta[count], sta_to_lta[count]);

	count++;
	while(count != params.lengthLTA){

		if(count < params.lengthSTA){
			sta[count] = (tempData[count] + tempData[count - 1]) / params.lengthSTA;
		}
		else{
			sta[count] = sta[count - 1] + ( (tempData[count] - tempData[count - params.lengthSTA]) / params.lengthSTA);
		}
		lta[count] = (tempData[count] + tempData[count - 1]) / params.lengthLTA;
		sta_to_lta[count] = sta[count] / lta[count];
		//printf("count : %d - sta: %f - lta: %f - sta_to_lta : %f \n", count, sta[count], lta[count], sta_to_lta[count]);
		count++;
	}

	printf("count inizializefirstSamples es %d\n", count);
}

void detectEvent(int count, float sample, char *axis,char * date, char * time , int isGPS){

	if(EVENT_ON == 0 && sta_to_lta[count] >= params.thOn ){
		EVENT_ON = 1;

		/*strDepValues.npts = 0;
		strDepValues.dt = 1 / params.freq;
		strDepValues.dataNumber = 0;*/
		//initDataofSamples(date,time,isGPS);
		//eventStart =  count;
	}

	if(EVENT_ON == 1 && (sta_to_lta[count] <= params.thOff)){
		EVENT_ON = 0;
		checkMinimunDuration(axis,date,time);
	}

	//if(EVENT_ON == 1 && count < (params.freq - 1)){
	if(EVENT_ON == 1){
		if(countEventSamples == MAX_LENGTH_EVENT){
			EVENT_ON = 0;
			checkMinimunDuration(axis,date,time);
			countEventSamples = 0;
		}else{
			eventSamples[countEventSamples] = sample;
			countEventSamples++;
		}

	}
	/*else if (EVENT_ON == 1 && count == ( params.freq - 1)){
		eventSamples[countEventSamples] = sample;
		strDepValues.dataNumber = countEventSamples;
		strDepValues.npts = strDepValues.npts + strDepValues.dataNumber;
		strDepValues.dt = 1 / params.freq;
		countEventSamples = 0;
	}*/
}

void checkMinimunDuration(char *axis,char * date, char *time){

	char hour[2] = {0};
	char min[2] = {0};
	char seg[2] = {0};

	int startTime = 0, endTime = 0;

	hour[0] = time[0];
	hour[1] = time[1];

	min[0] = time[2];
	min[1] = time[3];

	seg[0] = time[4];
	seg[1] = time[5];

	//startTime = startTime.hour * 3600 + startTime.min * 60 + startTime.seg;
	endTime = atoi(hour) * 3600 + atoi(min) * 60 + atoi(seg);

	if((startTime - endTime) >= params.minimunDurationSeconds){
		eventNum += 1;
		//printf("Evento %d: de %d/%d/%d %d:%d:%d to date :%s time: %s", eventNum, startTime.year,startTime.month,startTime.day, startTime.hour, startTime.min, startTime.seg,date,time);
		//createEventFile(axis,date,time);
	}
	countEventSamples = 0;

    /*startTime.year = 0;
	startTime.month = 0;
	startTime.day = 0;
	startTime.hour = 0;
	startTime.min = 0;
	startTime.seg = 0;
	startTime.mseg = 0;*/

}


/*
void createEventFile(char *axis,char * date, char *time){

	float dt = 1 / params.freq;

	char dir[100] = {0};
	char fecha[100] = {0};
	struct stat st = {0};
	sprintf(fecha,"%s/%s",EVENTS_DIR_R,date);

	if (stat(EVENTS_DIR_R, &st) == -1) {
	    mkdir(EVENTS_DIR_R, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if (stat(fecha, &st) == -1) {
	    mkdir(fecha, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	sprintf(dir,"%s/%s/%s_%c%c%c%c%c%c_%s.sac",EVENTS_DIR_R,date,date,time[0],time[1],time[2],time[3],time[4],time[5],axis);

	writeSac(countEventSamples, eventSamples,dt,axis,dir);
	countEventSamples = 0;
}

void initDataofSamples(char * date, char *time, int isGPS){

	char year[2] = {0};
	char month[2] = {0};
	char day[2] = {0};

	char hour[2] = {0};
	char min[2] = {0};
	char seg[2] = {0};
	char mseg[3] = {0};

	// Se guarda en un array cuando dias hay desde el primero de enero hasta un mes.
	// int numeroDiasPorMes[13] = {0,31,28,31,30,31,30  ,31,31,  30,31,30,31};
	int numeroDiasPorMes[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};

	year[0] = date[4];
	year[1] = date[5];

	month[0] = date[2];
	month[1] = date[3];

	day[0] = date[0];
	day[1] = date[1];

	hour[0] = time[0];
	hour[1] = time[1];

	min[0] = time[2];
	min[1] = time[3];

	seg[0] = time[4];
	seg[1] = time[5];

	if(isGPS == 1){
		mseg[0] = time[7];
		mseg[1] = time[8];
		mseg[2] = time[9];
	}

	startTime.year = 2000 + atoi(year);
	startTime.month = atoi(month);

	startTime.day = numeroDiasPorMes[startTime.month - 1] + atoi(day);

	startTime.hour = atoi(hour);
	startTime.min = atoi(min);
	startTime.seg = atoi(seg);
	startTime.mseg = atoi(mseg);

}



void writeSac(int npts, float *arr, float dt, char *axis, char *filename){

        int nerr;
        float b = 0, e = 0, depmax = 0.0, depmin = 0.0, depmen = 0.0;

        e = b + (npts -1 )*dt;
        // get the extrema of the trace

        //scmxmn(arr,dataNumber,&strDepValues.depmax,&strDepValues.depmin,&strDepValues.depmen);
        scmxmn(arr,npts,&depmax,&depmin,&depmen);
        // create a new header for the new SAC file
        newhdr();

        // set some header values
        setfhv("DEPMAX", depmax, &nerr);
        setfhv("DEPMIN", depmin, &nerr);
        setfhv("DEPMEN", depmen, &nerr);
        setnhv("NPTS    ",npts,&nerr);
        setfhv("DELTA   ",dt  ,&nerr);

        setfhv("B       ",b  ,&nerr);
        setihv("IFTYPE  ","ITIME   ",&nerr);

        setfhv("E       ",e     ,&nerr);
        setlhv("LEVEN   ",1,&nerr);
        setlhv("LOVROK  ",1,&nerr);
        setlhv("LCALDA  ",1,&nerr);

        // put is a default time for the plot

        setnhv("NZYEAR", startTime.year, &nerr);
        setnhv("NZJDAY", startTime.day, &nerr);
     	setnhv("NZHOUR", startTime.hour, &nerr);
     	setnhv("NZMIN" , startTime.min, &nerr);
     	setnhv("NZSEC" , startTime.seg, &nerr);
    	setnhv("NZMSEC", startTime.mseg, &nerr);

    	setkhv("KNETWK", "MEC",&nerr);
    	setkhv("KSTNM", "POP",&nerr);
    	setkhv("KCMPNM", axis,&nerr);
    	bwsac(npts,filename,arr);

}


*/

/*
void teoria(){
	int count = 0;
	while(count != params.lengthLTA){
		if(EVENT_ON == 1 && (strcmp("frozen", params.ltaMode) == 0){
			printf("frozen");
		}
		else{
			lta[count] = lta[count - 1] + ((tempData[count] - tempData[count - params.lengthLTA]) / params.lengthLTA);
		}

		sta[count] = sta[count -1] + ((tempData[count] - tempData[count - params.lengthSTA]) / params.lengthSTA);
		sta_to_lta[count] = sta[count] / lta[count];

		if(EVENT_ON == 0 && sta_to_lta[count] >= params.thOn ){
			EVENT_ON = 1;
			eventStart =  count;
			ltaFreezeLevel = lta[count];
		}

		if(EVENT_ON == 1 && (sta_to_lta[count] <= params.thOff)){
			EVENT_ON = 0;
			eventEnd = count;
			if(strcmp("frozen", params.ltaMode) == 0){
				printf("frozen en EVENT_ON == 1");
			}
			if( (eventEnd - eventStart) >= params.minimunDurationSeconds ){
				eventNum += 1;
				printf("Evento %d: de %d to %d", eventNum, eventStart,eventEnd);
				trigArr[eventNum] = (float)eventStart;
				trigArr[eventNum + 1] = (float)eventEnd;
				eventStart = 0;
				eventEnd = 0;
			}
		}

	}
}*/


