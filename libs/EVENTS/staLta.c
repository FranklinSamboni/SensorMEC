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

/*
staSeconds	->   Ventana de tiempo STA
ltaSeconds 	-> 	 Ventana de tiempo LTA
thOn 		->	 Disparador de evento "ON" cuando la relacion STA/LTA excede su valor
thOff		-> 	 Parador de evento "OFF" cuando la relacion STA/LTA es menor que su valor
minimunDurationSeconds -> La duracion del disparo debe exceder su valor para ser almacenada
*/

//Utilizar la funcion de math "fabs" para el valor absoluto

paramsSTA_LTA params;

void defaultParams(int freq){

	params.freq = freq;
	params.lengthSTA = 0.7*freq;
	params.lengthLTA = 7.0*freq;
	params.thOn = 3.0;
	params.thOff = 1.5;
	params.minimunDurationSeconds = 2;
	printf("defaultParams - freq %d, lengthSTA %d, lengthLTA %d, thOn %f, thOff %f, minimunDurationSeconds %d\n", params.freq, params.lengthSTA, params.lengthLTA, params.thOn, params.thOff, params.minimunDurationSeconds);
	//params.ltaMode = "";

}

void setParamsSTA_LTA(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, int minimunDurationSeconds){

	params.freq = freq;
	params.lengthSTA = staSeconds*freq;
	params.lengthLTA = ltaSeconds*freq;
	params.thOn = thOn;
	params.thOff = thOff;
	params.minimunDurationSeconds = minimunDurationSeconds;
	//params.ltaMode = "";
	printf("freq %d, lengthSTA %d, lengthLTA %d, thOn %f, thOff %f, minimunDurationSeconds %d\n", params.freq, params.lengthSTA, params.lengthLTA, params.thOn, params.thOff, params.minimunDurationSeconds);
}

void sta_lta(eventData *  event, float * newSamples, char * axis, char * date, char * time, int isGPS){

	int count = 0;
	while(count != params.freq){
		if(event->countTempData != (params.lengthLTA)){
			/* Al comenzar el script se debe inicializar el buffer temporal de los datos y las ventanas
			 * LTA y STA que se inicializan con inizializeFirstSamples cuando se ha completado el buffer de
			 * datos temporales.
			 */
			event->tempData[event->countTempData] = newSamples[count];

			if(event->countTempData == (params.lengthLTA - 1)){
				printf("ultimo valor de tempData %f - counttempData es : %d\n", event->tempData[event->countTempData], event->countTempData );
				printf("se llamo a inizializeFirstSamples\n");
				inizializeFirstSamples(event);
			}
			event->countTempData++; // countTempData al final sera igual a params.lengthLTA
		}
		else{

			if(event->countLTA_STA == params.lengthLTA){
				event->countLTA_STA = 0;
			}

			if(event->countLTA_STA == 0 ){
				event->sta[0] = event->sta[params.lengthLTA - 1] + ((newSamples[count] - event->tempData[event->countTempData - params.lengthSTA]) / params.lengthSTA);
				event->lta[0] = event->lta[params.lengthLTA - 1] + ((newSamples[count] - event->tempData[0]) / params.lengthLTA); // tempData[0] = tempData[countTempData - params.lengthLTA +  count]
				event->sta_to_lta[0] = event->sta[0] / event->lta[0];
				detectEvent(event, newSamples[count],axis,date,time,isGPS);
			}
			else{
				if( (event->countTempData - params.lengthSTA + count) >= event->countTempData ){
					event->sta[event->countLTA_STA] = event->sta[event->countLTA_STA - 1] + ((newSamples[count] - newSamples[count - params.lengthSTA]) / params.lengthSTA );
				}
				else{
					event->sta[event->countLTA_STA] = event->sta[event->countLTA_STA - 1] + ((newSamples[count] - event->tempData[event->countTempData - params.lengthSTA + count]) / params.lengthSTA );
				}

				event->lta[event->countLTA_STA] = event->lta[event->countLTA_STA - 1] + ((newSamples[count] - event->tempData[count]) / params.lengthLTA ); // tempData[count] = tempData[countTempData - params.lengthLTA +  count]
				event->sta_to_lta[event->countLTA_STA] = event->sta[event->countLTA_STA] / event->lta[event->countLTA_STA];
				detectEvent(event, newSamples[count],axis,date,time,isGPS);
			}

			//printf("count: %d - countLTA_STA: %d - temp[0]: %f - temp[40]: %f - sta: %f - lta: %f - sta_to_lta : %f\n", count, event->countLTA_STA , event->tempData[0] , event->tempData[40], event->sta[event->countLTA_STA], event->lta[event->countLTA_STA], event->sta_to_lta[event->countLTA_STA]);
			printf("count: %d - newSamples: %f - countLTA_STA: %d - sta: %f - lta: %f - sta_to_lta : %f\n", count, newSamples[count] ,event->countLTA_STA, event->sta[event->countLTA_STA], event->lta[event->countLTA_STA], event->sta_to_lta[event->countLTA_STA]);
			if(count == (params.freq - 1)){
				moveRegister(event->tempData, newSamples); // desplazar el registro para guardar los nuevos datos temporales.
			}

			event->countLTA_STA++;
		}

		count++;
	}
	printf("count staLta es %d\n", count);
}

void moveRegister(float * tempData ,float * newSamples){

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

void inizializeFirstSamples(eventData *  event){
	int count = 0;
	event->sta[count] = event->tempData[count] / params.lengthSTA;
	event->lta[count] = event->tempData[count] / params.lengthLTA;
	event->sta_to_lta[count] = event->sta[count] / event->lta[count];

	printf("inizializa count : %d - sta: %f - lta: %f - sta_to_lta : %f \n", count, event->sta[count], event->lta[count], event->sta_to_lta[count]);

	count++;
	while(count != params.lengthLTA){

		if(count < params.lengthSTA){
			event->sta[count] = (event->tempData[count] + event->tempData[count - 1]) / params.lengthSTA;
		}
		else{
			event->sta[count] = event->sta[count - 1] + ( (event->tempData[count] - event->tempData[count - params.lengthSTA]) / params.lengthSTA);
		}
		event->lta[count] = (event->tempData[count] + event->tempData[count - 1]) / params.lengthLTA;
		event->sta_to_lta[count] = event->sta[count] / event->lta[count];
		//printf("count : %d - sta: %f - lta: %f - sta_to_lta : %f \n", count, sta[count], lta[count], sta_to_lta[count]);
		count++;
	}

	printf("count inizializefirstSamples es %d\n", count);
}

void detectEvent(eventData *  event, float sample, char *axis,char * date, char * time , int isGPS){

	if(event->isPendingSaveEvent != 1){
		if(event->EVENT_ON == 0 && event->sta_to_lta[event->countLTA_STA] >= params.thOn ){
			event->EVENT_ON = 1;
			strcpy(event->date,date);
			strcpy(event->time,time);
			strcpy(event->axis,axis);
			event->isGPS = isGPS;
			printf("Init Event %d - Sample: %f - sta_to_lta: %f - thOn: %f\n", event->eventNum,  sample, event->sta_to_lta[event->countLTA_STA], params.thOn);
		}

		if(event->EVENT_ON == 1 && (event->sta_to_lta[event->countLTA_STA] <= params.thOff)){
			event->EVENT_ON = 0;
			printf("Finish Event %d - Sample: %f - sta_to_lta: %f - thOff: %f\n", event->eventNum, sample, event->sta_to_lta[event->countLTA_STA], params.thOff);
			//checkMinimunDuration(event, date, time);
		}

		if(event->EVENT_ON == 1){
			 //Se añaden las muestras tomadas cuando ocurre un evento a un buffer para luego ser guardadas
			/*if(event->countEventSamples == MAX_LENGTH_EVENT){
				event->EVENT_ON = 0;
				checkMinimunDuration(event, date, time);
			}else{
				event->eventSamples[event->countEventSamples] = sample;
				event->countEventSamples++;
			}*/
		}
	}
}

void checkMinimunDuration(eventData *  event, char * date, char *time){

	int startTime = 0, endTime = 0;

	startTime = getSeconds(event->date, event->time);
	endTime = getSeconds(date, time);

	if((endTime - startTime) >= params.minimunDurationSeconds){
		event->eventNum += 1;
		event->isPendingSaveEvent = 1;
		//printf("Evento %d: de %d/%d/%d %d:%d:%d to date :%s time: %s", eventNum, startTime.year,startTime.month,startTime.day, startTime.hour, startTime.min, startTime.seg,date,time);
		//createEventFile(axis,date,time);
	}
	//countEventSamples = 0;
}

int getSeconds(char * date, char *time){

	/*int numeroDiasPorMes[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
	int days = 0;
	char month[2] = {0};
	char day[2] = {0};

	month[0] = date[2];
	month[1] = date[3];

	day[0] = date[0];
	day[1] = date[1];*/

	char hour[2] = {0};
	char min[2] = {0};
	char seg[2] = {0};

	hour[0] = time[0];
	hour[1] = time[1];

	min[0] = time[2];
	min[1] = time[3];

	seg[0] = time[4];
	seg[1] = time[5];

	//days = numeroDiasPorMes[atoi(month) - 1] + atoi(day);
	return  atoi(hour) * 3600 + atoi(min) * 60 + atoi(seg);
}


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


