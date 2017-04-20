#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../libs/GPS/gps.h"
#include "../libs/GPIO/gpio.h"
#include "../libs/SOCKET/socketlib.h"

#define ERROR  -1
#define LOADING  1
#define SUCCESS  2

#define TYPE  "TEST"
#define CHECK_READ_GPS  "TEST_UART"
#define CHECK_PPS  "TEST_PPS"

#define MSG_READ_GPS "Cargando datos del GPS"
#define MSG_CHECK_COMPONENT "Revisa la conexi�n del GPS."

#define MSG_PPS "Verificando PPS"
/*
 * Formato del mensaje enviado:
 *
 * {
 * 		"status": ERROR,
 * 		"componente": "CHECK_READ_GPS",
 * 		"msg" : "Cargando...",
 * 		"last" : "BOOLEAN"
 * }
 *
 */
void sendMsg(int status, char *component, char * msg, int last);
void checkReadGPS();
void checkingPPS();

gpioParams gpio26_PPS; // para GPS


int main(int argc, char *argv[]){

	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
	//if(openSOCKET("52.34.55.59",46500)< 0){
		exit(0);
	}

	if(argc > 1){
		printf("-%s-\n", argv[1]);

	    if (strcmp("U", argv[1]) == 0)
	    {
	    	checkReadGPS();
	    }
		else if(strcmp("P", argv[1]) == 0){
			checkingPPS();
		}
		else{
			printf("Opcion incorrecta - \n");
			printf("Disponibles:  \n");
			printf("U para leer el UART\n");
			printf("P para verificar la PPS \n");
			printf("ejemplo: ./testGps U\n");
		}
	}
	else{
		printf("No se ingresaron opciones\n");
	}

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

void checkReadGPS(){

	int res = 0;
	char buf[255] = {0};

	//TASA DE BAUDIOS 3 DE = B115200
	if(openUART(3,DEVICE_UART)< 0){
		sendMsg(ERROR,CHECK_READ_GPS,"Error intentado abrir el dispositivo UART.",1);
		exit(0);
	}

	//configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA)
	configureNMEA_Messages(1,0,0,0,1,0,0);
	sleep(1); // waiting for GPS

	sendMsg(LOADING,CHECK_READ_GPS,MSG_READ_GPS,0);
	sleep(1);

	time_t inicio, fin;
	float diff = 0;
	float diff2 = 0;

	inicio = time(NULL);

	int count = 0;
	while(1){

		res = readUART(buf);

		if(res != -1){

			if(isGGA(buf) == 1 || isRmcStatusOk(buf) == 1){

				if(count < 5){
					inicio = time(NULL);
					//printBuffer(res,buf);
					buf[res] = 0;
					buf[res-1] = 0;
					buf[res-2] = 0;

					if(count == 4){
						sendMsg(SUCCESS,CHECK_READ_GPS,buf,1);
					}
					else{
						sendMsg(SUCCESS,CHECK_READ_GPS,buf,0);
					}


					//printf("count es: %d\n", count);
					count++;
				}
				else{
					//printf("countssss es: %d\n", count);
					break;
				}
			}
		}
		else{
			fin  = time(NULL);
			diff2 = difftime(fin,inicio);
			//printf("%f\n", diff2);
			if( diff2 > 20.0){
				if( diff2 > diff){

					sendMsg(ERROR,CHECK_READ_GPS,MSG_CHECK_COMPONENT,1);
					break;
				}
				diff = difftime(fin,inicio);
			}
		}
	}
}

void checkingPPS(){

	int cont = 0;

	initGPIO(26, &gpio26_PPS);
	setDirection(INPUT, &gpio26_PPS);

	sendMsg(LOADING,CHECK_PPS,MSG_PPS,0);

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(1){

		if(getValue(&gpio26_PPS) == HIGH){
			usleep(4000); // delay de 4 ms para no volver a capturar el mismo pulso

			//printf("PPS\n");
			inicio = time(NULL);
			if(cont == 5){

				sendMsg(SUCCESS,CHECK_PPS,"El funcionamiento de PPS es correcto",1);
				break;
			}
			cont++;
		}else{
			fin = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){

					sendMsg(ERROR,CHECK_PPS,"Verifica el pin de PPS.",1);
					break;
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

