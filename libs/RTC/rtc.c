#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rtc.h"

rtcStr rtc;

int openI2C(char * rtcDevice){

	strcpy(rtc.device,rtcDevice);
	rtc.file = open(rtc.device, O_RDWR);
    if (rtc.file < 0) {
		errorRtc("Error intentando abrir el dispositivo I2C.");
		perror(rtc.device);
		return -1;
    }
    if (ioctl(rtc.file, I2C_SLAVE, SLAVE_ADDRESS) < 0) {
        errorRtc("Error al intentar conectar al dispositivo esclavo");
        perror(rtc.device);
        return -1;
    }
    return 1;

}

int readI2C(char * buffer){

	int resultado; /* Numero de bytes resultantes de la lectura. */

    /*char buffer[BUF_SIZE_I2C];*/
	//printf("buffer antes\n");
	//printData(buffer);

	writeAddr(0x00);
    resultado = read(rtc.file,buffer,BUF_SIZE_I2C);

    if(resultado < 0){
        errorRtc("Error al intentar leer el bus I2C");
        perror(rtc.device);
        return -1;
    }
	//printf("buffer despues\n");
	//printData(buffer);
    buffer[resultado] = 0;
    return resultado;
}

int writeAddr(unsigned int address){
	char buffer[1];
	buffer[0]=address;
	int res = write(rtc.file,buffer,1);
	if (res !=1 ){
		errorRtc("Error al intentar escribir en el dispositivo I2C.");
		perror(rtc.device);
		return -1;
	}
	return 1;
}

int writeI2C(unsigned int address, unsigned int content){
	char buffer[2];
	int bytes;

	buffer[0] = address;
	buffer[1] = content;
	//printf("Escribiendo (RTC): !%hhX! En la direccion  !%hhX! \n",content, address);
	bytes = write(rtc.file,buffer,2);

    if( bytes != 2) {
    	errorRtc("Error al intentar escribir en el dispositivo I2C.");
    	perror(rtc.device);
    	return -1;
    }
    return 1;
}

int closeI2C(){
	int fd = close(rtc.file);
	if(fd == -1 ){
		errorRtc("Error cerrando I2C.");
		return -1;
	}
	rtc.file = fd; /* fd debe ser cero si la sentencia close es correcta.*/
	rtc.device[0] = 0;
	rtc.device[1] = 0;
	rtc.device[2] = 0;
	rtc.device[3] = 0;

	return 1;
}

void errorRtc(char *msgError){
	printf("\nError RTCLIB: %s\n",msgError);
}

void printData(char * buffer){
	int i = 0;
	printf("Datos I2C : ");
    while(i < BUF_SIZE_I2C){
	   printf("!%x!", buffer[i]);
	   i = i+1;
    }
    printf("\n");
}


void saveDataRtc(char * buffer, char * dir){

	//FILE *archivo = fopen ("fecha.txt", "w+");

	char time[10] = {0};
	char date[10] = {0};

	getTimeRtc(time,buffer);
	getDateRtc(date,buffer);

	//createDirRtc(dir,date,time);

	FILE *archivo = fopen (dir, "r+");

	//sprintf(values.date,"%02x%02x%02x",buffer[4],buffer[5],buffer[6]);
	//sprintf(values.time,"%02x%02x%02x",buffer[2],buffer[1],buffer[0]);

	//printf("RTC archivo -> date: %s | time: %s \n",date,time);
	fprintf(archivo,"date: %s | time: %s",date,time);

	fclose(archivo);
}

int activeAlarmRtc(){

	//printf("Activando alarma RTC.\n");
	int res = 0;
	res = writeI2C(A1M1_SECONDS,0x81);
	if(res != -1){
		res = writeI2C(A1M2_MINUTES,0x80);
		if(res != -1){
			res = writeI2C(A1M3_HOUR,0x80);
 			if(res != -1){
 				res = writeI2C(A1M4_DAY_DATE,0x80);
 				if(res != -1){
 					res = writeI2C(EOSC,0x1D);
 					return 1;
 				}
			}
		}
	}

	errorRtc("Fallo Activar alarma.");
	writeI2C(0x00,0x00);
	return -1;
}

int desactiveAlarmRtc(){

	if(writeI2C(EOSC,0x1C) == 1){
		return 1;// Para actualizar la hora.
	}

	errorRtc("Fallo desactivar alarma.");
	writeI2C(0x00,0x00);
	return -1;
}

/* Verificar que el I2C_DATA con la información venga organizada. para ello primero sincronizar con setTime. */
void getTimeRtc(char * buffer, char *I2C_DATA){

	sprintf(buffer,"%02x%02x%02x",I2C_DATA[2],I2C_DATA[1],I2C_DATA[0]);

	/*if(values.time[0] == 0 && values.time[1] == 0 && values.time[2] == 0){
		errorRtc("Error Tiempo no se ha capturado\n");
	    return -1;
	}

	int tam = sizeof(values.time);
	int count = 0;
	while(count < tam){

		if(values.time[count] == 0){
			break;
		}
		buffer[count] = values.time[count];
		count = count + 1;
	}
	return count;*/
}

/* Verificar que el I2C_DATA con la información venga organizada. para ello primero sincronizar con setTime. */

void getDateRtc(char * buffer, char *I2C_DATA){

	sprintf(buffer,"%02x%02x%02x",I2C_DATA[4],I2C_DATA[5],I2C_DATA[6]);

	/*if(values.date[0] == 0 && values.date[1] == 0 && values.date[2] == 0){
		errorRtc("Error La fecha no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(values.date);
	int count = 0;
	while(count < tam){
		if(values.date[count] == 0){
			break;
		}
		buffer[count] = values.date[count];
		count = count + 1;
	}
	return count;*/
}


int setTimeRtc(char * buffer){

	//printf("\nSincronizando hora  RTC y GPS\n");
	int res = 0;

	char hor[2] = {buffer[0],buffer[1]};
	char min[2] = {buffer[2],buffer[3]};
	char vseg[2] = {buffer[4],buffer[5]};

	unsigned int nuh = (int)strtol(hor, NULL, 16);
	unsigned int num = (int)strtol(min, NULL, 16);
	unsigned int nus = (int)strtol(vseg, NULL, 16);

	/*printf("hora en Hexa es : !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
	printf("hora en ASCII es : !%c!!%c!!%c!!%c!!%c!!%c!\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
	printf("Convertido  !%hhX! !%hhX! !%hhX!\n", nuh,num,nus);*/

	res = writeI2C(HOURS,nuh); // 0x02 direcci�n del regitro de hora
	if(res != -1){
		res = writeI2C(MINUTES,num); // 0x01 direcci�n del regitro de minutos
 		if(res != -1){
			res = writeI2C(SECONDS,nus); // 0x00 direcci�n del regitro de segundos
 			if(res != -1){
				return 1;
			}
		}
	}

	errorRtc("Fallo Sincronización de la hora");
	writeI2C(0x00,0x00);
	return -1;
}

int setDateRtc(char * buffer){

	//printf("\nSincronizando fecha RTC y GPS\n");
	int res = 0;

	char dia[2] = {buffer[0],buffer[1]};
	char mes[2] = {buffer[2],buffer[3]};
	char anio[2] = {buffer[4],buffer[5]};

	unsigned int nud = (int)strtol(dia, NULL, 16);
	unsigned int num = (int)strtol(mes, NULL, 16);
	unsigned int nua = (int)strtol(anio, NULL, 16);

	/*printf("Date en Hexa es : !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
	printf("Date en ASCII es : !%c!!%c!!%c!!%c!!%c!!%c!\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
	printf("Convertido  !%hhX! !%hhX! !%hhX!\n", nuh,num,nus);*/

	res = writeI2C(DAY_OF_MONTH,nud); // 0x04 direccion del dia del mes
	if(res != -1){
		res = writeI2C(MONTH_CENTURY,num);
		if(res != -1){
			res = writeI2C(YEAR,nua);
 			if(res != -1){
				return 1;
			}
		}
	}

	errorRtc("Fallo Sincronización de ka fecha");
	writeI2C(0x00,0x00);
	return -1;
}

