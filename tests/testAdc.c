#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../libs/ADC/adc.h"
#include "../libs/GPIO/gpio.h"
#include "../libs/SOCKET/socketlib.h"

#define ERROR  -1
#define LOADING  1
#define SUCCESS  2

#define TYPE  "TEST"
#define CHECK_ADC  "TEST_ADC"

#define MSG_CHECK "Verificando funcionamiento del ADC"
#define MSG_ERROR_OPEN_SPI "No se ha podido abrir el dispositivo SPI."

/*
 * Formato del mensaje enviado:
 *
 * {
 * 		"status": ERROR,
 * 		"componente": "CHECK_ADC",
 * 		"msg" : "Cargando...",
 * 		"last" : "BOOLEAN"
 * }
 *
 */

void sendMsg(int status, char *component, char * msg, int last);
void checkADC();

int main(int argc, char *argv[]){

	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
	//if(openSOCKET("52.34.55.59",46500)< 0){
		exit(0);
	}

	checkADC();

	closeSOCKET();

	return 0;
}

void sendMsg(int status, char *component, char * msg, int last){

	json_object *jobj = json_object_new_object();

	json_object *jstatus = json_object_new_int(status);
	json_object *jtype = json_object_new_string(TYPE);
	json_object *jcomponent = json_object_new_string(component);
	json_object *jmsg = json_object_new_string(msg);
	json_object *jlast = json_object_new_boolean(last);

	json_object_object_add(jobj,"status", jstatus);
	json_object_object_add(jobj,"type", jtype);
	json_object_object_add(jobj,"component", jcomponent);
	json_object_object_add(jobj,"msg", jmsg);
	json_object_object_add(jobj,"last", jlast);

	//printf(json_object_to_json_string(jobj));
	writeSOCKET(json_object_to_json_string(jobj));
	sleep(1);

}

void checkADC(){

	char recvBuffer[6] = {0x00,};
	double voltaje = 0;
	double arrVoltajes[10000] = {0};
	double sumTotal = 0.0;
	char mensaje[100] = {0};


	if (openSPI(DECIVE_SPI) < 0){
		sendMsg(ERROR,CHECK_ADC,MSG_ERROR_OPEN_SPI,1);
		exit(0);
	}

	settingPins();
	settingTestADC();
//	settingADC();

	sendMsg(LOADING,CHECK_ADC,MSG_CHECK,0);

	sleep(1);

	time_t inicio,fin;
	double dif = 0.0;

	inicio = time(NULL);

	int count = 0;
	int ii = 0;
	while(1){

		readAINx(recvBuffer);
		arrVoltajes[count] = getVoltage(recvBuffer, 4.67);
		//arrVoltajes[count] = getVoltage(recvBuffer, EXTERNAL1_VREF);
		count++;

		fin = time(NULL);
		dif = difftime(fin,inicio);
		if(dif == 1.0){

			while(ii < count){
				sumTotal = sumTotal + arrVoltajes[ii];
				ii++;
			}
			voltaje = sumTotal / count;
			sprintf(mensaje,"Voltaje medido en AIN0 y AIN1 %lf",voltaje);
			sendMsg(SUCCESS,CHECK_ADC,mensaje,1);

			//printf("Voltaje de referencia es :%lf\n", voltaje);

			count = 0;
			break;
		}
	}
}
