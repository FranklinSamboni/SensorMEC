#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/*
staSeconds	->   Ventana de tiempo STA
ltaSeconds 	-> 	 Ventana de tiempo LTA
thOn 		->	 Disparador de evento "ON" cuando la relacion STA/LTA excede su valor
thOff		-> 	 Parador de evento "OFF" cuando la relacion STA/LTA es menor que su valor
minimunDurationSeconds -> La duracion del disparo debe exceder su valor para ser almacenada
*/

paramsSTA_LTA params;

int eventNum = 0;
int EVENT_ON = 0;
int eventStart=0;
int eventEnd=0;

float ltaFreezeLevel = 0;

float trigArr[200] = {0};

float sta[144000] = {0};
float lta[144000] = {0};
float sta_to_lta[144000] = {0};

int countTempData = 0;
float tempData[14400] = {0};

float currentSamples [200] = {0};


void defaultParams(int freq){

	params.freq = freq;
	params.lengthSTA = 0.7*freq;
	params.lengthLTA = 7.0*freq;
	params.thOn = 3.0;
	params.thOff = 1.5;
	params.minimunDurationSeconds = 2.0;
	params.ltaMode = "";

}

void setParamsSTA_LTA(int freq,  float staSeconds, float ltaSeconds, float thOn, float thOff, float minimunDurationSeconds){

	params.freq = freq;
	params.lengthSTA = staSeconds*freq;
	params.lengthLTA = ltaSeconds*freq;
	params.thOn = thOn;
	params.thOff = thOff;
	params.minimunDurationSeconds = minimunDurationSeconds;
	params.ltaMode = "";

}

void sta_lta(float * data, char * date, char * time){

	int count = 0;
	while(count != params.freq){
		if(countTempData != (params.lengthLTA)){
			tempData[countTempData] = data[count];
			tempData++;
		}
		else{
			tempData[count] = tempData[params.freq + count];
			tempData[countTempData - params.freq + count] = data[count];
		}
		count++;
	}
}


void inizializeFirstSamples(float * tempData){
	int count = 0;
	sta[count] = tempData[count];
	lta[count] = tempData[count];
	sta_to_lta[count] = sta[count] / lta[count];
	count++;
	while(count != params.lengthLTA){

		if(count < params.lengthSTA){
			sta[count] = (tempData[count] + tempData[count - 1]) / params.lengthSTA;
		}else{
			sta[count] = tempData[count -1] + ( (tempData[count] - tempData[count - params.lengthSTA]) / params.lengthSTA);
		}
		lta[count] = (tempData[count] + tempData[count - 1]) / params.lengthLTA;
		sta_to_lta[count] = sta[count] / lta[count];
		count++;
	}
}

void sta_lta(int length, float * data){

	int count = 0;

	while(count != length){
		if(EVENT_ON == 1 && (strcmp("frozen", params.ltaMode) == 0){
			printf("frozen");
		}

		else{
			lta[count] = lta[count - 1] + ((data[count] - data[count - params.lengthLTA]) / params.lengthLTA);
		}

		sta[count] = sta[count -1] + ((data[count] - data[count - params.lengthSTA]) / params.lengthSTA);
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
}



