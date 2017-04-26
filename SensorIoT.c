#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libs/ADC/adc.h"
#include "libs/GPS/gps.h"
#include "libs/RTC/rtc.h"
#include "libs/GPIO/gpio.h"
#include "libs/SOCKET/socketlib.h"
#include "libs/SAC_FILES/sacsubc.h"
#include "libs/JSON_FILES/filesJ.h"
#include "libs/EVENTS/sta_lta.h"

/*
 * Directions INPUT, OUTPUT
 * Values LOW, HIGH
 */


#define TYPE "MAIN"

/*Tipo de processos*/
#define ALERTS_ERROR "ALERTS_ERROR"
#define PUT_LOCATION "PUT_LOCATION_GPS"
#define PUT_RTC_DATE "PUT_RTC_DATEHOUR"
#define PUT_SPS "PUT_SPS"
#define UPLOAD_FILES "UPLOAD_FILES"
#define REAL_TIME "REAL_TIME"

/*Tipo de componentes*/
#define GPS "GPS"
#define PPS "PPS"
#define RTC "RTC"
#define SYNC "SYNC"
#define ADC "ADC"
#define ACC "ACC"
#define BAT "BAT"
#define WIFI "WIFI"

#define AXI_X "BH2"
#define AXI_Y "BH1"
#define AXI_Z "BHZ"

/*-----*/

#define SAMPLES_DIR_R "/home/debian/Sensor-IOT/SensorIoT/muestras"
#define BILLION 1000000000L

#define MAX_SPS 200

int SPS = 0;
float DT = 0.0;

char currentDirectoryX[100] = {0};
char currentDirectoryY[100] = {0};
char currentDirectoryZ[100] = {0};


/*float dt = 0.005 ;
int npts = 0;
int dataNumber = 200;*/

// de la libreria de sacsub.h se agregaron para mantener orden en los datos.
fullDate strFullDate;
depValues strDepValues;

gpioParams gpio68_SYNC; // para RTC
gpioParams gpio26_PPS; // para GPS

void openDevices();
void closeDevices();
void loadingGpsData();
void checkingPPS();
void sincronizarRtc();
void checkingSYNC();
void readAndSaveData();
void clearBuffer(char * buffer, int ln);
void sendMsg(char * process, char *component, char * msg, int last);
void sendSamples(float * samplesX, float * samplesY, float * samplesZ);
void readWithRTC();
int readAnalogInputsAndSaveData(char * date, char * time, int isGPS);
void createDirRtc(char *dir, char *axis,char * date, char *time, int isGPS, int last);
void writeSac(int npts, int numData, float *arr, float dt, char *axis ,char *filename);
void initDataofSamples(char * date, char *time, int isGPS);
void subMuestreo_xxx(float *currentData, float *newData, int factor);

int main(int argc, char *argv[]){


	if(argc > 1){
		printf("-%s-\n", argv[1]);
		SPS = atoi(argv[1]);

		if(SPS != 0){
			if(SPS == 40 || SPS == 50 || SPS == 100 || SPS == 200 ){
				DT = (float)(1.0 / SPS);
				printf("dt es : %f \n", DT);
			}
			else{
				printf("Opcion incorrecta \n");
				printf("Disponibles:  \n");
				printf("40 - 50 - 100 - 200\n");
				printf("ejemplo: ./SensorIoT 40 \n");
				exit(0);
			}
		}
		else{
			printf("Opcion incorrecta \n");
			printf("Disponibles:  \n");
			printf("40 - 50 - 100 - 200\n");
			printf("ejemplo: ./SensorIoT 40 \n");
			exit(0);
		}
	}
	else{
		printf("No se ingresaron opciones\n");
		exit(0);
	}


	openDevices();

	//(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, float minimunDurationSeconds)
	setParamsSTA_LTA(SPS, (float) 0.8, 8,(float) 3.0,(float) 1.5, (float) 3.0);

	printf("se llamo a settingPins\n");
	settingPins(); // Configurar pines de control del ADC
	printf("se llamo a settingADC\n");
	settingADC();

	printf("se llamo a loadingGpsData\n");
	loadingGpsData();
	printf("se llamo a checkingPPS\n");
	checkingPPS();
	printf("se llamo a sincronizarRtc\n");
	sincronizarRtc();
	printf("se llamo a checkingSYNC\n");
	checkingSYNC();

	readAndSaveData();

	closeDevices();

	return 0;
}

void openDevices(){
	initGPIO(68, &gpio68_SYNC);
	setDirection(INPUT, &gpio68_SYNC);

	initGPIO(26, &gpio26_PPS);
	setDirection(INPUT, &gpio26_PPS);

	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
		exit(0);
	}

	//TASA DE BAUDIOS 3 DE = B115200
	if(openUART(3,DEVICE_UART)< 0){
		exit(0);
	}

	if(openI2C(DECIVE_I2C)< 0){
		exit(0);
	}

	if (openSPI(DECIVE_SPI) < 0){
		exit(0);
	}
}

void closeDevices(){
	destroyGPIO(&gpio68_SYNC);
	destroyGPIO(&gpio26_PPS);
	desactiveAlarmRtc();
	closeUART();
	closeI2C();
	closeSOCKET();
	closeSPI();
}

void sendMsg(char * process, char *component, char * msg, int last){

	json_object *jobj = json_object_new_object();

	json_object *jtype = json_object_new_string(TYPE);
	json_object *jprocess = json_object_new_string(process);
	json_object *jcomponent = json_object_new_string(component);
	json_object *jmsg = json_object_new_string(msg);
	json_object *jlast = json_object_new_boolean(last);

	json_object_object_add(jobj,"type", jtype);
	json_object_object_add(jobj,"process", jprocess);
	json_object_object_add(jobj,"component", jcomponent);
	json_object_object_add(jobj,"msg", jmsg);
	json_object_object_add(jobj,"last", jlast);

	writeSOCKET(json_object_to_json_string(jobj));
	writeSOCKET("\r\n");
	//sleep(1);

}

void sendSamples(float * samplesX, float * samplesY, float * samplesZ){

	json_object *jobj = json_object_new_object();

	json_object *jtype = json_object_new_string(TYPE);
	json_object *jprocess = json_object_new_string(REAL_TIME);

	json_object *jarrayX = json_object_new_array();
	json_object *jarrayY = json_object_new_array();
	json_object *jarrayZ = json_object_new_array();

	json_object *jelementsX[2] = {0};
	json_object *jelementsY[2] = {0};
	json_object *jelementsZ[2] = {0};

	/*float dataX[50] = {0};
	float dataY[50] = {0};
	float dataZ[50] = {0};*/
	//int factor = MAX_SPS/50;

	//subMuestreo_xxx(samplesX, dataX, factor);
	//subMuestreo_xxx(samplesY, dataY, factor);
	//subMuestreo_xxx(samplesZ, dataZ, factor);

	//int i = 0;
	//while(i < 2){
		jelementsX[0] = json_object_new_double(samplesX[0]);
		jelementsY[0] = json_object_new_double(samplesY[0]);
		jelementsZ[0] = json_object_new_double(samplesZ[0]);

		json_object_array_add(jarrayX,jelementsX[0]);
		json_object_array_add(jarrayY,jelementsY[0]);
		json_object_array_add(jarrayZ,jelementsZ[0]);

		jelementsX[1] = json_object_new_double(samplesX[1]);
		jelementsY[1] = json_object_new_double(samplesY[1]);
		jelementsZ[1] = json_object_new_double(samplesZ[1]);

		json_object_array_add(jarrayX,jelementsX[1]);
		json_object_array_add(jarrayY,jelementsY[1]);
		json_object_array_add(jarrayZ,jelementsZ[1]);
		//i++;
	//}

	json_object_object_add(jobj,"type", jtype);
	json_object_object_add(jobj,"process", jprocess);

	json_object_object_add(jobj,"x", jarrayX);
	json_object_object_add(jobj,"y", jarrayY);
	json_object_object_add(jobj,"z", jarrayZ);

	writeSOCKET(json_object_to_json_string(jobj));
	writeSOCKET("\r\n");

}

void loadingGpsData(){

	int res = 0, flag = 0;
	char buf[255] = {0};
	char lat[24] = {0}, lng[24] = {0}, alt[24] = {0};
	char dateGps[24] = {0}, timeGps[24] = {0};
	char msg[255] = {0};

	/*configureSerialPort(3); Configurar tasa de baudios GPS a 115200
	sleep(1);  waiting for GPS*/

	//configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA)
	configureNMEA_Messages(1,0,0,0,1,0,0);
	sleep(1); // waiting for GPS

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);
	printf("limpiando\n");
	clearUartBuffer();
	printf("termino limpieza\n");
	while(1){

		res = readUART(buf);
		if(res != -1){
			//printBuffer(res,buf);
			inicio = time(NULL);

			if(isGGA(buf) == 1){
				if(getAlt(alt,buf) != -1) {
					flag = 1;
				}
			}
			else if(isRmcStatusOk(buf) == 1){
				getLat(lat,buf);
				getLng(lng,buf);
				getDateGps(dateGps,buf);
				getTimeGps(timeGps,buf);
				if(flag == 1){

					gpsJson(CORRECT_STATUS_COMPONENT,"Venus GPS logger","115200","GGA - RMC","");
					locationJson(lat, lng,alt);

					/*char sps[32] = {0};
					sprintf(sps,"%s",SPS);
					adcJson(CORRECT_STATUS_COMPONENT,"ads1262 de 32 bits",sps,"");*/

					sendMsg(PUT_LOCATION,GPS,"-",1);
					sleep(1);
					break;
				}
			}
		}
		else{
			fin  = time(NULL);
			if(difftime(fin,inicio) > 30.0){
				if(difftime(fin,inicio) > diff){

					sprintf(msg,"%s","Revisa la conexi�n del GPS, no se ha podido leer el dispositivo UART en mas 30 segundos de ejecuci�n.");
					gpsJson(ERROR_STATUS_COMPONENT,"Venus GPS logger","115200","GGA - RMC",msg);

					sendMsg(ALERTS_ERROR,GPS,msg,1);
					sleep(1);
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

void checkingPPS(){

	char msg[255] = {0};
	int cont = 0;
	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(1){
		if(getValue(&gpio26_PPS) == HIGH){
			inicio = time(NULL);
			if(cont == 5){
				sleep(1);
				break;
			}
			cont++;
		}else{
			fin = time(NULL);
			if(difftime(fin,inicio) > 30.0){
				if(difftime(fin,inicio) > diff){
					sprintf(msg,"%s","Verifica la conexi�n de la se�al PPS, no se ha podido capturar en mas 30 segundos de ejecuci�n.");
					sendMsg(ALERTS_ERROR,GPS,msg,1);
					gpsJson(ERROR_STATUS_COMPONENT,"Venus GPS logger","115200","GGA - RMC",msg);
					sleep(1);
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

void sincronizarRtc(){

	int res = 0;
	char buf[255] = {0};
	char timeBuf[12] = {0}, dateBuf[12] = {0};

	int count = 0;

	printf("limpiando\n");
	clearUartBuffer();
	printf("termino limpieza\n");
	while(1){
		if(getValue(&gpio26_PPS) == HIGH){
			res = readUART(buf);
			//printBuffer(res,buf);

			if(res != -1){
				if(isRmcStatusOk(buf) == 1){

					getTimeGps(timeBuf,buf); // configurando Hora
					setTimeRtc(timeBuf); // Sincroniza RTC y GPS.

					getDateGps(dateBuf,buf); // configurando fecha
					setDateRtc(dateBuf);

					if(count > 4){
						break;
					}
					count++;
				}
			}

		}
	}
}

void checkingSYNC(){

	int res = 0;
	int cont = 0;
	char bufRtc[255] = {0};;
	char timeBufRtc[24] = {0}, dateBufRtc[24] = {0};
	char fecha[50] = {0};
	char msg[255] = {0};

	activeAlarmRtc();

	time_t inicio, fin;
	double diff = 0;
	inicio = time(NULL);

	while(1){

		if(getValue(&gpio68_SYNC) == LOW){
			inicio = time(NULL);

			// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
			writeI2C(0x0F,0x88);

			if(cont > 4){

				res = readI2C(bufRtc);
				if(res != -1){

					getTimeRtc(timeBufRtc,bufRtc);
					getDateRtc(dateBufRtc,bufRtc);

					sprintf(fecha,"%c%c/%c%c/%c%c %c%c:%c%c:%c%c",dateBufRtc[4],dateBufRtc[5],dateBufRtc[2],dateBufRtc[3],dateBufRtc[0],dateBufRtc[1], timeBufRtc[0],timeBufRtc[1],timeBufRtc[2],timeBufRtc[3],timeBufRtc[4],timeBufRtc[5]);
					printf("fecha es : %s\n", fecha);

					rtcJson(CORRECT_STATUS_COMPONENT,"DS3231",fecha,"");
					sendMsg(PUT_RTC_DATE,RTC,"-",1);
					sleep(1);
					break;
				}
				else{
					sprintf(msg,"%s","No se ha podido leer el dispositivo I2C, revisa la conexi�n del RTC.");
					rtcJson(ERROR_STATUS_COMPONENT,"DS3231","",msg);
					sendMsg(ALERTS_ERROR,RTC,msg,1);
					sleep(1);
					break;
				}
			}
			cont++;
		}
		else{
			fin = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){
					sprintf(msg,"%s","Verifica la conexi�n de SQW (pin de SYNC) del RTC, su lectura ha demorado demasiado.");
					rtcJson(ERROR_STATUS_COMPONENT,"DS3231","",msg);
					sendMsg(ALERTS_ERROR,RTC,msg,1);
					sleep(1);
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

void readAndSaveData(){

	char buf[255] = {0};
	char bufTime[15] = {0}, bufDate[15] = {0}, bufLat[15] = {0};
	int flag = 0;
	int gps = 0;
	int isData = 0;

	int isGPS = 1;

	int sendNotiPPS = 1;
	int sendNotiSYNC = 1;

	//uint64_t diff;
	//struct timespec start, end;

	time_t inicio,fin;
	double count_PPS = 0.0;


	printf("limpiando\n");
	clearUartBuffer();
	printf("termino limpieza\n");

	inicio = time(NULL);

	while(1){

		if(getValue(&gpio26_PPS) == HIGH){
			inicio = time(NULL);
			//printf("\n ----- Senial pps ------- \n");
			//clearBuffer(buf,255);
			gps = readUART(buf);
			//printBuffer(gps,buf);
			if(gps != -1){
				while(gps != -1){
					flag = 0;

					if(isRMC(buf) == 1) {

						getTimeGps(bufTime,buf);
						getDateGps(bufDate,buf);

						isData = getLat(bufLat,buf);
						//printf("Tiempo: %s - Fecha: %s - Latitud %s\n",bufTime,bufDate,bufLat );
					}
					clearBuffer(buf,255);
					gps = readUART(buf);
				}
				if(isData != -1){
					if(sendNotiPPS == 1){
						sendNotiPPS = 0;
						sendNotiSYNC = 1;
						sendMsg(ALERTS_ERROR,GPS,"Sensor activado con señal PPS",1);
					}

					readAnalogInputsAndSaveData(bufDate, bufTime,isGPS);
				}
				else{
					if(getValue(&gpio68_SYNC) == LOW){
						flag = 1;
						sendNotiPPS = 1;
						readWithRTC(&sendNotiSYNC);
					}
				}
			}
			else{
				if(getValue(&gpio68_SYNC) == LOW){
					flag = 1;
					sendNotiPPS = 1;
					readWithRTC(&sendNotiSYNC);
				}
			}
		}
		else{
			fin = time(NULL);
			count_PPS = difftime(fin,inicio);
			if(flag == 1 || count_PPS > 2.0){
				if(getValue(&gpio68_SYNC) == LOW){
					//clock_gettime(CLOCK_MONOTONIC, &start);
					//printf("Low pps.\n");
					sendNotiPPS = 1;
					readWithRTC(&sendNotiSYNC);
					//clock_gettime(CLOCK_MONOTONIC, &end);
					//diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
					//printf("Tiempo milisegundos = %llu ms\n", (long long unsigned int) diff/1000000);
				}
			}
		}
	}

}

void clearBuffer(char * buffer, int ln){
	int i = 0;
	while(i < ln){
		buffer[i] = 0;
		i++;
	}
}

void readWithRTC(int * sendNotification){
	char bufRtc[255] = {0};
	char time[10] = {0};
	char date[10] = {0};
	int rtc = 0;
	int isGPS = 0;
	printf("Con RTC.\n");
	// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
	writeI2C(0x0F,0x88);

	if(*sendNotification == 1){
		sendMsg(ALERTS_ERROR,RTC,"Sensor activado con señal SYNC",1);
		*sendNotification = 0;
	}

	rtc = readI2C(bufRtc);
	if(rtc != -1){
		printData(bufRtc);
		getTimeRtc(time,bufRtc);
		getDateRtc(date,bufRtc);
		readAnalogInputsAndSaveData(date,time,isGPS);
	}
}

int readAnalogInputsAndSaveData(char * date, char * time, int isGPS){

	char recvX[6] = {0x00,};
	char recvY[6] = {0x00,};
	char recvZ[6] = {0x00,};

	float dataX[MAX_SPS] = {0};
	float dataY[MAX_SPS] = {0};
	float dataZ[MAX_SPS] = {0};

	float samplesX[MAX_SPS] = {0};
	float samplesY[MAX_SPS] = {0};
	float samplesZ[MAX_SPS] = {0};

	int factor =  MAX_SPS/SPS;

	// FACTOR_SPS_200 = 2
	//FACTOR_SPS_100 = 4
	// FACTOR_SPS_50  = 8

	createDirRtc(currentDirectoryX, AXI_X, date, time, isGPS,0);
	createDirRtc(currentDirectoryY, AXI_Y, date, time, isGPS,0);
	createDirRtc(currentDirectoryZ, AXI_Z, date, time, isGPS,1);

	int count = 0;

	//printf("Capturando datos ADC\n");
	while (count != MAX_SPS){

		readAIN2_3(recvX);
		readAIN4_5(recvY);
		readAIN6_7(recvZ);

		dataX[count] = (float) (((unsigned long)recvX[1]<<24)|((unsigned long)recvX[2]<<16)|(recvX[3]<<8)|recvX[4]);
		dataY[count] = (float) (((unsigned long)recvY[1]<<24)|((unsigned long)recvY[2]<<16)|(recvY[3]<<8)|recvY[4]);
		dataZ[count] = (float) (((unsigned long)recvZ[1]<<24)|((unsigned long)recvZ[2]<<16)|(recvZ[3]<<8)|recvZ[4]);

		//printf("Counter: %d\n",count);
		count++;
	}

	if(factor != 1){
		subMuestreo_xxx(dataX, samplesX, factor);
		subMuestreo_xxx(dataY, samplesY, factor);
		subMuestreo_xxx(dataZ, samplesZ, factor);

		strDepValues.npts = strDepValues.npts + strDepValues.dataNumber;
		writeSac(strDepValues.npts,strDepValues.dataNumber,samplesX,strDepValues.dt,AXI_X,currentDirectoryX);
		writeSac(strDepValues.npts,strDepValues.dataNumber,samplesY,strDepValues.dt,AXI_Y,currentDirectoryY);
		writeSac(strDepValues.npts,strDepValues.dataNumber,samplesZ,strDepValues.dt,AXI_Z,currentDirectoryZ);
		//printf("Termino camptura de datos ADC factor %d\n", factor);
		sendSamples(samplesX,samplesY,samplesZ);
	}
	else{
		strDepValues.npts = strDepValues.npts + strDepValues.dataNumber;
		writeSac(strDepValues.npts,strDepValues.dataNumber,dataX,strDepValues.dt,AXI_X,currentDirectoryX);
		writeSac(strDepValues.npts,strDepValues.dataNumber,dataY,strDepValues.dt,AXI_Y,currentDirectoryY);
		writeSac(strDepValues.npts,strDepValues.dataNumber,dataZ,strDepValues.dt,AXI_Z,currentDirectoryZ);
		//printf("Termino camptura de datos ADC factor %d\n", factor);
		sendSamples(dataX,dataY,dataZ);
	}

	count = 0;
	return 0;
}

void subMuestreo_xxx(float *currentData, float *newData, int factor){

	int i = 0;
	int samples = MAX_SPS/factor;

	while(i<samples){
		newData[i] = currentData[i*factor];
		//printf("i es %d y el dato tomado es %d\n",i,i*factor);
		i++;
	}

}

void createDirRtc(char *dir, char *axis,char * date, char *time, int isGPS, int last){
	char fecha[100] = {0};
	struct stat st = {0};
	sprintf(fecha,"%s/%s",SAMPLES_DIR_R,date);

	if(dir[0] == 0){

		if (stat(SAMPLES_DIR_R, &st) == -1) {
		    mkdir(SAMPLES_DIR_R, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}

		if (stat(fecha, &st) == -1) {
		    mkdir(fecha, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}

		sprintf(dir,"%s/%s/%s_%c%c_%s.sac",SAMPLES_DIR_R,date,date,time[0],time[1],axis);
		initDataofSamples(date,time,isGPS);
		createFile(dir);

	}
	else if (time[2]=='0' && time[3]=='0' && time[4]=='0' && time[5]=='0'){ //Nueva Hora
	//else if (time[4]=='0' && time[5]=='0'){ //Nueva Hora
		if (stat(SAMPLES_DIR_R, &st) == -1) {
		    mkdir(SAMPLES_DIR_R, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}

		if (stat(fecha, &st) == -1) {
		    mkdir(fecha, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}

		if ( last == 1){
			sendMsg(UPLOAD_FILES,ADC,dir,1);
		}

		sprintf(dir,"%s/%s/%s_%c%c_%s.sac",SAMPLES_DIR_R,date,date,time[0],time[1],axis);
		//sprintf(dir,"%s/%s/%s%s_%c%c%c%c%c%c.sac",SAMPLES_DIR_R,date,axis,date,time[0],time[1],time[2],time[3],time[4],time[5]);
		initDataofSamples(date,time,isGPS);
		createFile(dir);

	}
	//printf("Dir: %s\n", dir);
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
	//int numeroDiasPorMes[13] = {0,31,28,31,30,31,30  ,31,31,  30,31,30,31};
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

	strFullDate.year = 2000 + atoi(year);
	strFullDate.month = atoi(month);

	strFullDate.day = numeroDiasPorMes[strFullDate.month - 1] + atoi(day);

	strFullDate.hour = atoi(hour);
	strFullDate.min = atoi(min);
	strFullDate.seg = atoi(seg);
	strFullDate.mseg = atoi(mseg);

	// se reinician el numero de muestras para que comience a contar nuevamente en el siguiente archivo
	strDepValues.npts = 0;
	strDepValues.dt = DT;
	strDepValues.dataNumber = SPS;
	/// se definene los valores de DELTA, NTPS, y dataNumber que es el numero de datos por segundo
	// este no se incluye como tal en el archivo.
}

void writeSac(int npts, int dataNumber, float *arr, float dt, char *axis ,char *filename)
{

        int nerr;
        float b = 0, e = 0;

        e = b + (npts -1 )*dt;
        /* get the extrema of the trace */

        scmxmn(arr,dataNumber,&strDepValues.depmax,&strDepValues.depmin,&strDepValues.depmen);

        /* create a new header for the new SAC file */
        newhdr();

        /* set some header values */
        setfhv("DEPMAX", strDepValues.depmax, &nerr);
        setfhv("DEPMIN", strDepValues.depmin, &nerr);
        setfhv("DEPMEN", strDepValues.depmen, &nerr);
        setnhv("NPTS    ",npts,&nerr);
        setfhv("DELTA   ",dt  ,&nerr);

        setfhv("B       ",b  ,&nerr);
        setihv("IFTYPE  ","ITIME   ",&nerr);

        setfhv("E       ",e     ,&nerr);
        setlhv("LEVEN   ",1,&nerr);
        setlhv("LOVROK  ",1,&nerr);
        setlhv("LCALDA  ",1,&nerr);

        /* put is a default time for the plot */

        setnhv("NZYEAR", strFullDate.year, &nerr);
        setnhv("NZJDAY", strFullDate.day, &nerr);
     	setnhv("NZHOUR", strFullDate.hour, &nerr);
     	setnhv("NZMIN" , strFullDate.min, &nerr);
     	setnhv("NZSEC" , strFullDate.seg, &nerr);
    	setnhv("NZMSEC", strFullDate.mseg, &nerr);

    	setkhv("KNETWK", "MEC",&nerr);
    	setkhv("KSTNM", "POP",&nerr);
    	setkhv("KCMPNM", axis,&nerr);
     	updateHeaders(filename);
     	updateData(filename,dataNumber,arr);

}

