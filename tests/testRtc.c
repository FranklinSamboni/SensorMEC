#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../libs/RTC/rtc.h"
#include "../libs/GPIO/gpio.h"
#include "../libs/SOCKET/socketlib.h"


#define ERROR  -1
#define LOADING  1
#define SUCCESS  2

#define TYPE  "TEST"
#define CHECK_READ_RTC  "TEST_RTC"
#define CHECK_SYNC  "TEST_SYNC"

#define MSG_READ_RTC "Cargando datos del RTC"
#define MSG_CHECK_COMPONENT "Revisa la conexi�n del RTC."

#define MSG_SYNC "Verificando SYNC"
/*
 * Formato del mensaje enviado:
 *
 * {
 * 		"status": ERROR,
 * 		"componente": "CHECK_READ_RTC",
 * 		"msg" : "Cargando...",
 * 		"last" : "BOOLEAN"
 * }
 *
 */
void sendMsg(int status, char *component, char * msg, int last);
void checkReadRTC();
void checkingSYNC();

gpioParams gpio68_SYNC; // para RTC

int main(int argc, char *argv[]){

	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
		exit(0);
	}

	if(argc > 1){
		printf("-%s-\n", argv[1]);

	    if (strcmp("I", argv[1]) == 0)
	    {
	    	checkReadRTC();
	    }
		else if(strcmp("S", argv[1]) == 0){
			checkingSYNC();
		}
		else{
			printf("Opcion incorrecta - \n");
			printf("Disponibles:  \n");
			printf("I para leer el I2C\n");
			printf("S para comprobar la se�al SYNC \n");
			printf("ejemplo: ./testRtc U\n");
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

void checkReadRTC(){

	int res = 0;
	char buf[255] = {0};
	char dateBuf[10] = {0};
	char timeBuf[10] = {0};
	char msg[100] = {0};

	if(openI2C(DECIVE_I2C)< 0){
		sendMsg(ERROR,CHECK_READ_RTC,"Error intentado abrir el dispositivo I2C.",1);
		exit(0);
	}

	sendMsg(LOADING,CHECK_READ_RTC,MSG_READ_RTC,0);

	time_t inicio, fin;
	float diff = 0;
	float diff2 = 0;

	inicio = time(NULL);

	int count = 0;
	while(1){

		res = readI2C(buf);

		if(res != -1){

			if(count < 5){

				inicio = time(NULL);
				//printBuffer(res,buf);
				printData(buf);
				getTimeRtc(timeBuf,buf);
				getDateRtc(dateBuf,buf);

				sprintf(msg,"UTC %c%c/%c%c/%c%c %c%c:%c%c:%c%c",dateBuf[4],dateBuf[5],dateBuf[2],dateBuf[3],dateBuf[0],dateBuf[1], timeBuf[0],timeBuf[1],timeBuf[2],timeBuf[3],timeBuf[4],timeBuf[5]);

				if(count == 4){
					sendMsg(SUCCESS,CHECK_READ_RTC,msg,1);
				}
				else{
					sendMsg(SUCCESS,CHECK_READ_RTC,msg,0);
				}
				//printf("count es: %d\n", count);
				count++;
			}
			else{
				//printf("countssss es: %d\n", count);
				break;
			}
		}
		else{
			fin  = time(NULL);
			diff2 = difftime(fin,inicio);
			//printf("%f\n", diff2);
			if( diff2 > 20.0){
				if( diff2 > diff){

					sendMsg(ERROR,CHECK_READ_RTC,MSG_CHECK_COMPONENT,1);
					break;
				}
				diff = difftime(fin,inicio);
			}
		}
	}
}

void checkingSYNC(){

	int cont = 0;

	initGPIO(68, &gpio68_SYNC);
	setDirection(INPUT, &gpio68_SYNC);

	if(openI2C(DECIVE_I2C)< 0){
		sendMsg(ERROR,CHECK_READ_RTC,"Error intentado abrir el dispositivo I2C.",1);
		exit(0);
	}

	sendMsg(LOADING,CHECK_SYNC,MSG_SYNC,0);

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(1){

		if(getValue(&gpio68_SYNC) == LOW){
			writeI2C(0x0F,0x88);

			//printf("SYNC\n");
			inicio = time(NULL);
			if(cont == 5){

				sendMsg(SUCCESS,CHECK_SYNC,"El funcionamiento de SYNC es correcto",1);
				break;
			}
			cont++;
		}else{
			fin = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){

					sendMsg(ERROR,CHECK_SYNC,"Verifica el pin de SYNC.",1);
					break;
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

