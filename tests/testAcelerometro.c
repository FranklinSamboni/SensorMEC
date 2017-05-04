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
#define CHECK_ACCELEROMETRO  "TEST_ACC"

#define MSG_CHECK_MODE "Configurando modo test Accelerometro ADXL354."
#define MSG_VALUES_MODE_TEST "Valores en modo test de X - Y - Z."
#define MSG_VALUES_MODE_FORCE "Aplicando fuerza electro-estática en X - Y - Z."

#define MSG_ERROR_OPEN_SPI "Para probar el Accelerometro se necesita el adc conectado - No se ha podido abrir el dispositivo SPI."

gpioParams gpio65_ST1_ACC; // st1 acelerometro
gpioParams gpio27_ST2_ACC; // st2 acelerometro

void sendMsg(int status, char *component, char * msg, int last);
void testAcelerometro();

int main(int argc, char *argv[]){

	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
	//if(openSOCKET("52.34.55.59",46500)< 0){
		exit(0);
	}

	testAcelerometro();

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
	printf("%s\n", msg);
	writeSOCKET(json_object_to_json_string(jobj));
	sleep(1);

}

void testAcelerometro(){

	char recvX[6] = {0x00,};
	char recvY[6] = {0x00,};
	char recvZ[6] = {0x00,};

	char msg[255] = {0};
	char msg2[255] = {0};

	double voltajeX = 0;
	double voltajeY = 0;
	double voltajeZ = 0;

	if (openSPI(DECIVE_SPI) < 0){
		sendMsg(ERROR,CHECK_ACCELEROMETRO,MSG_ERROR_OPEN_SPI,1);
		exit(0);
	}

	settingPins();
	settingADC();

	initGPIO(65, &gpio65_ST1_ACC);  // ST1 EN ALTO INDICA MODO TEST
	setDirection(OUTPUT, &gpio65_ST1_ACC);

	initGPIO(27, &gpio27_ST2_ACC);
	setDirection(OUTPUT, &gpio27_ST2_ACC); // ST2 ALTO APLICA FUERZA AL ACCELEROMETRO.

	setValue(LOW,&gpio65_ST1_ACC);
	setValue(LOW,&gpio27_ST2_ACC);

	sendMsg(LOADING,CHECK_ACCELEROMETRO,MSG_CHECK_MODE,0);

	/* ---------------------------------   */

	setValue(HIGH,&gpio65_ST1_ACC); // MODO TEST
	sleep(1);


	readAIN2_3(recvX);
	readAIN4_5(recvY);
	readAIN6_7(recvZ);

	voltajeX = getVoltage(recvX,1.8);
	voltajeY = getVoltage(recvY,1.8);
	voltajeZ = getVoltage(recvZ,1.8);

	sprintf(msg,"%s -> %lf - %lf - %lf",MSG_VALUES_MODE_TEST,voltajeX,voltajeY,voltajeZ);
	sendMsg(LOADING,CHECK_ACCELEROMETRO,msg,0);

	/* ---------------------------------   */

	setValue(HIGH,&gpio27_ST2_ACC); // APLICA FUERZA
	sleep(1);

	readAIN2_3(recvX);
	readAIN4_5(recvY);
	readAIN6_7(recvZ);

	voltajeX = getVoltage(recvX,1.8);
	voltajeY = getVoltage(recvY,1.8);
	voltajeZ = getVoltage(recvZ,1.8);

	sprintf(msg2,"%s -> %lf - %lf - %lf",MSG_VALUES_MODE_FORCE,voltajeX,voltajeY,voltajeZ);

	sendMsg(LOADING,CHECK_ACCELEROMETRO,msg2,1);

	setValue(LOW,&gpio65_ST1_ACC);
	setValue(LOW,&gpio27_ST2_ACC); // TERMINA

}
