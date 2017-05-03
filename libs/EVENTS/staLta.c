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
#include <math.h>
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
	params.preEvent = 1*freq;
	params.postEvent = 1*freq;
	printf("freq %d, lengthSTA %d, lengthLTA %d, thOn %f, thOff %f, minimunDurationSeconds %d, preEvent %d, postEvent %d\n", params.freq, params.lengthSTA, params.lengthLTA, params.thOn, params.thOff, params.minimunDurationSeconds, params.preEvent, params.postEvent);
	//params.ltaMode = "";

}

void setParamsSTA_LTA(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, int minimunDurationSeconds, int preEvent, int postEvent){

	params.freq = freq;
	params.lengthSTA = staSeconds*freq;
	params.lengthLTA = ltaSeconds*freq;
	params.thOn = thOn;
	params.thOff = thOff;
	params.minimunDurationSeconds = minimunDurationSeconds;
	params.preEvent = preEvent*freq;
	params.postEvent = postEvent*freq;
	//params.ltaMode = "";
	printf("freq %d, lengthSTA %d, lengthLTA %d, thOn %f, thOff %f, minimunDurationSeconds %d, preEvent %d, postEvent %d\n", params.freq, params.lengthSTA, params.lengthLTA, params.thOn, params.thOff, params.minimunDurationSeconds, params.preEvent, params.postEvent);
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
				printf("Axis : %s - ultimo valor de tempData %f - counttempData es : %d\n", axis, event->tempData[event->countTempData], event->countTempData );
				//printf("se llamo a inizializeFirstSamples\n");
				inizializeFirstSamples(event);
			}

			if(count == (params.freq - 1)){
				addSamplesPreEvent(event,newSamples,params.freq, 0);
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
				event->sta_to_lta[0] = fabs(event->sta[0] / event->lta[0]);
				detectEvent(event, newSamples,count,axis,date,time,isGPS);
			}
			else{
				if( (event->countTempData - params.lengthSTA + count) >= event->countTempData ){
					event->sta[event->countLTA_STA] = event->sta[event->countLTA_STA - 1] + ((newSamples[count] - newSamples[count - params.lengthSTA]) / params.lengthSTA );
				}
				else{
					event->sta[event->countLTA_STA] = event->sta[event->countLTA_STA - 1] + ((newSamples[count] - event->tempData[event->countTempData - params.lengthSTA + count]) / params.lengthSTA );
				}

				event->lta[event->countLTA_STA] = event->lta[event->countLTA_STA - 1] + ((newSamples[count] - event->tempData[count]) / params.lengthLTA ); // tempData[count] = tempData[countTempData - params.lengthLTA +  count]
				event->sta_to_lta[event->countLTA_STA] = fabs(event->sta[event->countLTA_STA] / event->lta[event->countLTA_STA]);
				detectEvent(event, newSamples,count,axis,date,time,isGPS);
			}

			//printf("moviendo registro temp[0]: %lf - temp[40]: %lf\n",event->tempData[0], event->tempData[40]);

			if(count == (params.freq - 1)){
				printf("axis %s - count: %d - newSamples: %f - countLTA_STA: %d - sta: %f - lta: %f - sta_to_lta : %f\n",axis, count, newSamples[count] ,event->countLTA_STA, event->sta[event->countLTA_STA], event->lta[event->countLTA_STA], event->sta_to_lta[event->countLTA_STA]);
				moveRegister(event->tempData, newSamples,params.freq,params.lengthLTA,0); // desplazar el registro para guardar los nuevos datos temporales.
			}

			event->countLTA_STA++;
		}

		count++;
	}
	//printf("count staLta es %d\n", count);
}

void addSamplesPreEvent(eventData *  event, float * samples, int samplesNumber , int firstPosition){

	int count = 0;
	if(event->countPreEvent != params.preEvent){
		while(count != samplesNumber){ // inizializar el registro de pre eventos.
			event->preEvent[event->countPreEvent] = samples[count];
			//printf("count : %d - countPreEvent : %d -  llenando addSamples %lf\n", count,event->countPreEvent, event->preEvent[event->countPreEvent]);
			event->countPreEvent++;
			count++;
		}
		//printf("\n\n\n");
	}
	else{

		//printf("MOVIENDO addSamples preEvent[0]: %lf - preEvent[40]: %lf\n",event->preEvent[0], event->preEvent[40]);
		moveRegister(event->preEvent,samples, samplesNumber, event->countPreEvent, firstPosition);

	}
}

void moveRegister(float * tempData ,float * newSamples, int samplesNumber, int bufferSize, int firstPosition){

	/*  Se mueve el buffer tempData una posicion a la izquierda.
	 *  ( ej. tempData[0] = tempData [1]))
	 *  	  tempData[1] = tempData [2])) .... y se pasan las nuevas muestras al final del buffer.
	 */

	int count = 0;
	//int positionOfNewSamples =  params.lengthLTA - params.freq;
	int positionOfNewSamples =  bufferSize - samplesNumber;

	//while (count < params.lengthLTA){
	while (count < bufferSize){

		if(count >= (positionOfNewSamples)){
			tempData[count] = newSamples[count - positionOfNewSamples + firstPosition];
		}else{
			tempData[count] = tempData[samplesNumber + count];
		}
		count ++;
	}
}

void inizializeFirstSamples(eventData *  event){
	int count = 0;
	event->sta[count] = event->tempData[count] / params.lengthSTA;
	event->lta[count] = event->tempData[count] / params.lengthLTA;
	event->sta_to_lta[count] = fabs(event->sta[count] / event->lta[count]);

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
		event->sta_to_lta[count] = fabs(event->sta[count] / event->lta[count]);
		//printf("count : %d - sta: %f - lta: %f - sta_to_lta : %f \n", count, sta[count], lta[count], sta_to_lta[count]);
		count++;
	}

	printf("count inizializefirstSamples es %d\n", count);
}

void detectEvent(eventData *  event, float * newSamples, int countNewSamples, char *axis,char * date, char * time , int isGPS){

	if(event->isPendingSaveEvent != 1){

		if(event->isEnableAddPostEvent == 1){

			if(event->countPostEvent != params.postEvent){
				// borrar este registro postEvent cuando se haya creado el archivo de los eventos  y reiniciar el countPostEvent.
				printf("count postEvent : %d - llenando post event %lf \n",event->countPostEvent, newSamples[countNewSamples]);
				event->postEvent[event->countPostEvent] = newSamples[countNewSamples];
				event->countPostEvent++;
			}
			else{
				event->isPendingSaveEvent = 1;
				event->isEnableAddPostEvent = 0;

				printf("TERMINO POST EVENT count postEvent : %d - llenando post event %lf \n",event->countPostEvent, newSamples[countNewSamples]);
				printf("preEvent count es %d\n", event->countPreEvent);

				//printfBuff(event->preEvent,(event->countPreEvent + 10), "Buf pre - Event + 10 ");
				//printfBuff(event->postEvent,(event->countPostEvent + 10), "Buf POST - Event + 10 ");

				//printfBuff(event->eventSamples,(event->countEventSamples + 10), "Buf eventSamples + 10 ");
				//printfBuff(event->tempData,(event->countTempData + 10), "Buf tempData + 10 ");

			}
		}
		else{
			if(event->EVENT_ON == 0 && event->sta_to_lta[event->countLTA_STA] >= params.thOn ){
				event->EVENT_ON = 1;
				strcpy(event->date,date);
				strcpy(event->time,time);
				strcpy(event->axis,axis);
				event->isGPS = isGPS;
				addSamplesPreEvent(event,newSamples,countNewSamples,0);
				printf("Axis %s - count event: %d - Init Event %d - Sample: %f - countSamples : %d - sta_to_lta: %f - time %s\n",axis, event->countEventSamples, event->eventNum,  newSamples[countNewSamples], countNewSamples, event->sta_to_lta[event->countLTA_STA],time);

				printfBuff(event->tempData,(event->countTempData + 10), "Init Buf tempData + 10 ");
				printfBuff(event->preEvent,(event->countPreEvent + 10), "Init Buf pre - Event + 10 ");
			}
			else{
				if(countNewSamples == (params.freq - 1) && event->EVENT_ON == 0){
					addSamplesPreEvent(event,newSamples,params.freq,0);
				}
			}

			if(event->EVENT_ON == 1 && (event->sta_to_lta[event->countLTA_STA] <= params.thOff)){
				event->EVENT_ON = 0;
				printf("Axis %s - count event: %d - Finish Event %d - Sample: %f - countSamples : %d - sta_to_lta: %f - time %s\n",axis, event->countEventSamples, event->eventNum, newSamples[countNewSamples], countNewSamples ,event->sta_to_lta[event->countLTA_STA], time);

				if(checkMinimunDuration(event, date, time) == 1){
					event->isEnableAddPostEvent = 1;
					event->eventNum += 1;
				}
				else{
					if(event->countEventSamples > event->countPreEvent){
						//se añaden las muestras que se guardaron en el buffer de eventos a el buffer de pre evento ya que este no duro lo suficiente.
						addSamplesPreEvent(event,event->eventSamples,event->countPreEvent , (event->countEventSamples - event->countPreEvent));
					}
					else{
						 addSamplesPreEvent(event, event->eventSamples, event->countEventSamples,0);
					}
					event->countEventSamples = 0;
					event->isPendingSaveEvent = 0;
					event->isEnableAddPostEvent = 0;
				}
				printfBuff(event->tempData,(event->countTempData + 10), "Finish Buf tempData + 10 ");
				printfBuff(event->preEvent,(event->countPreEvent + 10), "Finish Buf pre - Event + 10 ");
			}


			if(event->EVENT_ON == 1){
				 //Se aï¿½aden las muestras tomadas cuando ocurre un evento a un buffer para luego ser guardadas
				if(event->countEventSamples == MAX_LENGTH_EVENT){
					event->EVENT_ON = 0;
					checkMinimunDuration(event, date, time);
				}else{
					event->eventSamples[event->countEventSamples] = newSamples[countNewSamples];
					//printf("axi: %s - count event: %d - sample %lf\n", axis, event->countEventSamples, sample);
					event->countEventSamples++;
				}
			}
		}
	}
}

int checkMinimunDuration(eventData *  event, char * date, char *time){

	int startTime = 0, endTime = 0;

	startTime = getSeconds(event->date, event->time);
	endTime = getSeconds(date, time);

	if((endTime - startTime) >= params.minimunDurationSeconds){

		printf("startTime es: %d, end time es :%d -- preEvent es: %d\n",startTime,endTime,(params.preEvent / params.freq));
		if(startTime >= (params.preEvent / params.freq)){
			startTime = startTime - (params.preEvent / params.freq);
			printf("startTime menos el valor de preEvent %d\n", startTime);
			setStartTime(startTime, event->date, event->time);

		}
		return 1;
		//printf("Evento %d: de %d/%d/%d %d:%d:%d to date :%s time: %s", eventNum, startTime.year,startTime.month,startTime.day, startTime.hour, startTime.min, startTime.seg,date,time);
	}
	else{
		return 0;
	}
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

void setStartTime(int seconds, char * date, char *time){

	int hour = 0,min = 0,seg = 0;

	hour = seconds / 3600;
	printf("horas %d\n",hour);
	min = (seconds % 3600) / 60;
	printf("min %d\n",min);
	seg = (seconds % 3600) % 60;
	printf("seg %d\n",seg);
	sprintf(time,"%02d%02d%02d",hour,min,seg);
	printf("la nueva hora es: %s\n", time);
}

void printfBuff(float * buffer, int ln, char * name){
	int count = 0;
	while (count < ln){
		printf("buffer : %s - count:  %d - valor : % lf\n", name, count, buffer[count]);
		count++;
	}
	printf("\n");
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


