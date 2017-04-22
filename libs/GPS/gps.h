/* --------------------------------------------------------------------------------------------------
 * Para la configuración del GPS se utiliza los mensajes binarios de "SkyTraq Venus 6 GPS receiver".
 * Sintaxis de los mensajes: <0xA0,0xA1><PL><Message ID><Message Body><CS><0x0D,0x0A>.
 * - <0xA0,0xA1>: Cabecera de inicio de mensaje.
 * - PL: PayLoad length o carga útil.
 * - <Message ID>: Id del mensaje.
 * - <Message Body>: Cuerpo del mensaje.
 * - CS: CheckSum.
 * - <0x0D,0x0A>: Cabecera de fin de mensaje.
 *
 * Para mas información sobre estos mensaje binarios consultar :
 * http://cdn.sparkfun.com/datasheets/Sensors/GPS/AN0003_v1.4.19.pdf
 * --------------------------------------------------------------------------------------------------*/

#ifndef _GPS_H_
#define _GPS_H_

/*
 * El dispositivo Venus GPS Logger se comunica por medio del puerto UART, a través del cual
 * envia la información ya sea en forma binaria o en formato NMEA.
 *
 * Este dispositivo envia la información cada segundo, ademas cuenta con una señal PPS
 * que se activa cada segundo y nos servir� de sincronizació del sistema.
 *
 * Para mas información consultar su datasheet.
 * http://cdn.sparkfun.com/datasheets/Sensors/GPS/Venus638FLPx.pdf
 *
 * */

	#include <stdio.h>
	#include <termios.h>

	#define DEVICE_UART "/dev/ttyO2" /* Ubicación del dispositivo, en este caso UART2. */
	#define BAUDRATE0 B4800
	#define BAUDRATE1 B9600 	/* 9600 es la tasa de baudios establecida por defecto. */
	#define BAUDRATE2 B38400
	#define BAUDRATE3 B115200

	#define SAMPLES_DIR "muestras"
//#define TRUE 1
//#define FALSE 0

	typedef struct gpsStr gpsStr;
	typedef struct gpsData gpsData;

	//char currentDirectory[100];

	struct gpsStr {
		int file;
		char device[24];
		struct termios options; /* Opciones de configuración del UART. */
	};

	struct gpsData {
		//char gpgga[14][24];
		//char gprmc[14][24];

		char date[15], time[15];
		char lat[15], lng[15];
		char alt[15];

	};

/* Estas funciones retornan -1 si su proceso se ha ejecutado con algun error.*/
	int openUART(int baudRate, char * gpsDevice);
	int readUART(char * buf);
	int writeUART(char * buffer);
	int closeUART();

/* Función que permite general el checksum de los mensajes binarios.  */
	char checkSum(int pl, char * payload);

/* Restaurar los parametros del GPS a valores de fabrica.*/
	int setFactoryDefaults();

/* Configurar el puerto serial.
 * Se elige alguna de las siguientes tasas de baudios:
 * - bauds = 0 -> 4800.
 * - bauds = 1 -> 9600.
 * - bauds = 2 -> 38400.
 * - bauds = 3 -> 115200.
 * El Venus GPS logger no maneja las tasas de baudios de : 19200 y 57600. */
	int configureSerialPort(int bauds);

/* Configurar el tipo de mensajes a leer.
 * - type = 1 ->  mensajes NMEA.
 * - type = 2 -> mensajes Binarios.
 * - type == 0 -> no output, desabilitar salida de mensajes. */
	int configureMessageType(int type);

/* Configurar el tipo de mensajes NMEA que se desean leer.
 * Para habilitar o deshabilitar los mensasjes NMEA se envia un 0x00 o un 0x01, en
 * el parametro del pensaje que se quiere modificar.
 * Condiciones:
 * - 0x00 o 0 -> deshabilitado.
 * - 0x01 o 1 -> habilitado. */
	int configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA);

	void errorGps(char *msgError);
	void printBuffer(int size, char * buffer);

/* Guarda los datos del GPS en un archivo de texto e inizializa la estructura gpsData. */
	void saveDataGps(char * buffer, char * dir);

/* Verifica que el gps haya obtenido los datos basicos.
 * Para ello verifica el byte Status de la trama RMC.*/
	int isRmcStatusOk(char * buffer);

	int isGGA(char * buffer);
	int isRMC(char * buffer);

	int getTimeGps(char * buffer, char * GGA_NEMEA);
	int getDateGps(char * buffer, char * RMC_NEMEA);
	int getLat(char * buffer, char * GGA_NEMEA);
	int getLng(char * buffer, char * GGA_NEMEA);
	int getAlt(char * buffer, char * GGA_NEMEA);
	void clearUartBuffer();


#endif
