#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "gpio.h"

void initGPIO(int number, gpioParams* params){

	if(params->number != 0){
		eraseParams(params);
	}

	params->number = number;

	sprintf(params->name, "gpio%d",number);
	sprintf(params->path, "%s%s/",GPIO_PATH,params->name);
	/*strcpy(params->path,GPIO_PATH);
	strcat(params->path,params->name);
	strcat(params->path,"/"); *///modificar

	//printf("%s\n",params->path);
	//printf("%s\n",GPIO_PATH);
	exportGPIO(params);
	usleep(250000); // esperando a que el sistema configure el GPIO

}

int exportGPIO(gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - Export");
		return -1;
	}

	//printf("Exportando..\n");
	int res = writeGPIOInt(GPIO_PATH, "export", params->number);
	return res;
}

int unexportGPIO(gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - Unexport");
		return -1;
	}

	//printf("Deshabilitando..\n");
	int res = writeGPIOInt(GPIO_PATH, "unexport", params->number);
	eraseParams(params);
	return res;
}

int writeGPIOInt(char * path, char * filename, int value){
	char val[10];
	sprintf(val, "%d",value);
	int res = writeGPIO(path,filename,val);
	return res;
}

int writeGPIO(char * path, char * filename, char * value){

	FILE *fp;
	char name[32];

	strcpy(name,path);
	strcat(name,filename);

	fp = fopen(name, "w");
	if(fp == NULL){
		return -1;
	}
	fprintf(fp,"%s",value);
	fclose(fp);
	return 1;
}

int readGPIO(char * path, char * filename, char * buffer){

	FILE *fp;
	char name[32];

	strcpy(name,path);
	strcat(name,filename);

	fp = fopen(name, "r");
	fgets(buffer, 255, (FILE*)fp);
	//printf("readGPIO %s\n", buffer);
	fclose(fp);

	return 1;

}

int setDirection(gpioDirection direction, gpioParams* params){

	int res=0;

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - setDirection");
		return -1;
	}

	if(direction == INPUT){
		res = writeGPIO(params->path, "direction", "in");
	}
	else if(direction == OUTPUT){
		res = writeGPIO(params->path, "direction", "out");
	}else{
		res = -1;
	}

	return res;

}

int setValue(gpioValue value, gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - setValue");
		return -1;
	}

	int res=0;

	if(value == LOW){
		//printf("Value LOW\n");
		res = writeGPIO(params->path, "value", "0");
	}
	else if(value == HIGH){
		//printf("Value HIGH\n");
		res = writeGPIO(params->path, "value", "1");
	}else{
		printf("eRROR\n");
		res = -1;
	}

	return res;

}

int setEdgeType(gpioEdge edge, gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - setEdgeType");
		return -1;
	}

	int res = 0;
	if(edge == NONE){
		res = writeGPIO(params->path, "edge", "none");
	}
	else if(edge == RISING){
		res = writeGPIO(params->path, "edge", "rising");
	}
	else if(edge == FALLING){
		res = writeGPIO(params->path, "edge", "falling");
	}
	else if(edge == BOTH){
		res = writeGPIO(params->path, "edge", "both");
	}
	else{
		res = -1;
	}
	return res;
}

int setActiveLow(gpioParams* params){
	int res = 0;
	res = writeGPIO(params->path, "active_low", "1");
	return res;
}

int setActiveHigh(gpioParams* params){
	int res = 0;
	res = writeGPIO(params->path, "active_low", "0");
	return res;
}



gpioDirection getDirection(gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - getDirection");
		return -1;
	}

	char in[10];

	readGPIO(params->path,"direction",in);
	in[10] = 0;

	if(strcmp(in,"in") == 0){
		return INPUT;
	}
	else{
		return OUTPUT;
	}

}

gpioValue getValue(gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - getValue");
		return -1;
	}

	char in[10];

	readGPIO(params->path,"value",in);
	in[10] = 0;
	//printf("get Value: %s\n", in);
	if( in[0] == '0' ){
		//printf("return LOW\n");
		return LOW;
	}
	else{
		//printf("return HIGH\n");
		return HIGH;
	}

}

gpioEdge getEdgeType(gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - getEdgeType");
		return -1;
	}

	char in[10];

	readGPIO(params->path,"edge",in);
	in[10] = 0;

	if(strcmp(in,"rising") == 0){
		return RISING;
	}
	else if(strcmp(in,"falling") == 0){
		return FALLING;
	}
	else if(strcmp(in,"both") == 0){
		return BOTH;
	}
	else{
		return NONE;
	}

}

int switchOutputValue(gpioParams* params){

	if(params->number == 0){
		printErrorMsgGpio("Variable no inicializada - switchOutputValue");
		return -1;
	}

	//setDirection(OUTPUT,params);

	if (getValue(params) == HIGH) {
		//printf("Set LOW\n");
		setValue(LOW,params);
	}
	else {
		//printf("Set HIGH\n");
		setValue(HIGH,params);
	}
    return 0;

}

void destroyGPIO(gpioParams* params){
	unexportGPIO(params);
}

void eraseParams(gpioParams* params){
	params->number = 0;

	int i = 0;
	while(i < 24){
		if(i < 10){
			params->name[i] = 0;
		}
		params->path[i] = 0;
		i = i+1;
	}
	params->name[10] = 0;
	params->name[24] = 0;
}

void printErrorMsgGpio(char *msgError){
	printf("\nGPIO Error: %s\n",msgError);
}






