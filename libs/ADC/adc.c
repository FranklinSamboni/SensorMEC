#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "../GPIO/gpio.h"
#include "adc.h"

/*
 * Usar 0x?B o 0x?C en el registro MODE2 del adc para conseguir valores de 200 muestras por segundo.
 * Usar 0x?9 en el registro MODE2 del adc para conseguir valores de 160 muestras por segundo.
 * Usar 0x?8 en el registro MODE2 del adc para conseguir valores de 80 muestras por segundo.
 */

adcStr adc;
gpioParams pinReset; // para RESET - out
gpioParams pinStart; // para START - out
gpioParams pinDrdy; // para DRDY activo bajo - in

int openSPI(char * SPIDevice){

	int ret = 0;

	adc.mode = 1;
	adc.bits = 8;
	adc.speed = 1000000;
	adc.delay = 1;

	strcpy(adc.device,SPIDevice);
	adc.file = open(adc.device, O_RDWR);
	if (adc.file < 0)
		errorADC("Error abriendo el dispositivo SPI.");
		perror(adc.device);

	// spi mode

	ret = ioctl(adc.file, SPI_IOC_WR_MODE, &adc.mode);
	if (ret < 0){
		errorADC("Error cambiando el modo del SPI.");
		perror(adc.device);
	}

	ret = ioctl(adc.file, SPI_IOC_RD_MODE, &adc.mode);
	if (ret < 0){
		errorADC("Error obteniendo el modo del SPI.");
		perror(adc.device);
	}

	// bits per word

	ret = ioctl(adc.file, SPI_IOC_WR_BITS_PER_WORD, &adc.bits);
	if (ret < 0){
		errorADC("Error cambiando los bits por palabra.");
		perror(adc.device);
	}

	ret = ioctl(adc.file, SPI_IOC_RD_BITS_PER_WORD, &adc.bits);
	if (ret < 0){
		errorADC("Error obteniendo los bits por palabra.");
		perror(adc.device);
	}

	 // max speed hz

	ret = ioctl(adc.file, SPI_IOC_WR_MAX_SPEED_HZ, &adc.speed);
	if (ret < 0){
		errorADC("Error cambiando la velocidad del SPI.");
		perror(adc.device);
	}

	ret = ioctl(adc.file, SPI_IOC_RD_MAX_SPEED_HZ, &adc.speed);
	if (ret < 0){
		errorADC("Error obteniendo la velocidad del SPI.");
		perror(adc.device);
	}

	/*printf("Modo del SPI: %d\n", adc.mode);
	printf("bits por palabra: %d\n", adc.bits);
	printf("Velocidad: %d Hz - %d KHz\n", adc.speed, adc.speed/1000);*/

	return ret;
}

int closeSPI(){
	int ret = 0;
	ret = close(adc.file);
	if(ret < 0){
		errorADC("Error cerrando el dispositivo SPI.");
		perror(adc.device);
	}
	return ret;
}

void settingTestADC(){

	writeRegister(POWER, 0x01); // clear bit reset to detect new reset
	usleep(10);

	writeRegister(MODE0, 0x40); // enable pulse conversion mode
	usleep(10);

	writeRegister(MODE1, 0x80); // Select sinc 0 for digital filter
	usleep(10);

	//writeRegister(MODE2, 0x08); // PGA disabled y 400 SPS
	//writeRegister(MODE2, 0x88);
	writeRegister(MODE2, 0x84); // 20 sps

	usleep(10);

	writeRegister(INPMUX, 0x01); // Select AIN0 como AINP Y AIN1 COMO AINN
	usleep(10);

	writeRegister(REFMUX, 0X24); // Seleccionar voltaje de alimentacion VSUPPY como voltaje de referencia
	usleep(10);

}

void settingADC(){

	writeRegister(POWER, 0x01); // clear bit reset to detect new reset
	usleep(10);

	writeRegister(MODE0, 0x40); // enable pulse conversion mode
	usleep(10);

	writeRegister(MODE1, 0x00); // Select sinc 0 for digital filter
	usleep(10);

	//writeRegister(MODE2, 0x08); // PGA disabled y 400 SPS
	//writeRegister(MODE2, 0x88);
	writeRegister(MODE2, 0x8B);
	usleep(10);

	writeRegister(INPMUX, 0x23); // Select AIN2 como AINP Y AIN3 COMO AINN
	usleep(10);

	writeRegister(REFMUX, 0X09); // Select AIN0 como REFP y AIN1 como REFN
	usleep(10);

	/*
	char recvBuf[2] = {0x00,};

	if(readRegister(recvBuf , POWER) > 1){
		printf("valor leido POWER: %.2X \n",recvBuf[0]);
		usleep(10);
	}

	if(readRegister(recvBuf , MODE0) > 1){
		printf("valor leido MODE0 : %.2X \n",recvBuf[0]);
		usleep(10);
	}

	if(readRegister(recvBuf , MODE1) > 1){
		printf("valor leido MODE1: %.2X \n",recvBuf[0]);
		usleep(10);
	}

	if(readRegister(recvBuf , MODE2) > 1){
		printf("valor leido MODE2 : %.2X \n",recvBuf[0]);
		usleep(10);
	}

	if(readRegister(recvBuf , INPMUX) > 1){
		printf("valor leido INPMUX: %.2X \n",recvBuf[0]);
		usleep(10);
	}

	if(readRegister(recvBuf , REFMUX) > 1){
		printf("valor leido REFMUX : %.2X \n",recvBuf[0]);
		usleep(10);
	}*/


}

void settingPins(){
	initGPIO(GPIO_RESET, &pinReset);
	setDirection(OUTPUT, &pinReset);
	// RESET debe ser alto para que el adc funcione
	setValue(HIGH,&pinReset);

	initGPIO(GPIO_START, &pinStart);
	setDirection(OUTPUT, &pinStart);
	// como vamos a realizar configuraci�n de los registro mantenemos START en bajo
	setValue(LOW,&pinStart);

	initGPIO(GPIO_DRDY, &pinDrdy);
	setDirection(INPUT, &pinDrdy); // para leer DRDY activo bajo
	//setValue(LOW,&drdy10);
}

void Start_HIGH(){
	setValue(HIGH,&pinStart);
}

void Start_LOW(){
	setValue(LOW,&pinStart);
}

void resetADC(){
	setValue(HIGH,&pinReset);
	usleep(1000);
	setValue(LOW,&pinReset);
	usleep(1000);
	setValue(HIGH,&pinReset);
	usleep(100);
}

void resetADCByCommand(){
	writeCommand(RESET_COMMAND);
	usleep(1000);
}

void StartConversionByCommand(){
	writeCommand(START_CONVERSION);
	usleep(10);
}

void StopConversionByCommand(){
	writeCommand(STOP_CONVERSION);
	usleep(10);
}

int transfer(unsigned char * sendBuffer, unsigned char * recvbuffer, int length){
	int result = 0;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)sendBuffer,
		.rx_buf = (unsigned long)recvbuffer,
		.len = length,
		.delay_usecs = adc.delay,
		.speed_hz = adc.speed,
		.bits_per_word = adc.bits,
	};

	result = ioctl(adc.file, SPI_IOC_MESSAGE(1), &tr);
	if (result < 1){
		errorADC("Error enviando mensaje atrav�s del SPI.");
		perror(adc.device);
	}
	return result;
}

int writeRegister(unsigned char address, unsigned char data){
	int result = 0;
	unsigned char tx[3] = { WRITE_REG|address, 0x00, data };
	unsigned char rx[3] = { 0x00, };

	result = transfer(tx,rx,3);
	return result;
}

int readRegister(char  * recvBuffer , unsigned char address){
	int result = 0;
	unsigned char tx[3] = { READ_REG|address, 0x00};
	unsigned char rx[3] = { 0x00, };

	result = transfer(tx,rx,3);
	if(result > 1){
		recvBuffer[0] = rx[2];
	}
	return result;
}

int writeCommand(unsigned char command){
	int result = 0;
	unsigned char tx[2] = { command, 0x00};
	unsigned char rx[2] = { 0x00, };

	result = transfer(tx,rx,2);
	return result;
}

int readDataByCommand(char * recvBuffer){
	int result = 0;
	unsigned char tx[8] = { READ_DATA_COMMAND, 0x00};
	unsigned char rx[8] = { 0x00, };

	result = transfer(tx,rx,8);

	for (result = 0; result < 8; result++) {
		if (!(result % 6))
			puts("");
		printf("%.2X ", rx[result]);
	}
	if(result > 1){
		int count = 0;
		while(count < 6){
			recvBuffer[count] = rx[count + 1];
			count++;
		}
	}

	return result;
}

int readData(char * recvBuffer){
	int result = 0;
	unsigned char tx[7] = { 0x00, };
	unsigned char rx[7] = { 0x00, };

	result = transfer(tx,rx,7);

	/*for (result = 0; result < 6; result++) {
		if (!(result % 6))
			puts("");
		printf("%.2X ", rx[result]);
	}*/
	if(result > 1){
		int count = 0;
		while(count < 6){
			recvBuffer[count] = rx[count];
			count++;
		}
	}
	return result;
}

int readAINx(char * recvBuffer){
	Start_LOW();
	Start_HIGH();
	//usleep(1);
	Start_LOW();
	int ret = -1;
	while(1){
		if(getValue(&pinDrdy) == LOW){
			ret = readData(recvBuffer);
			break;
		}
	}
	return ret;
}

int readAIN2_3(char * recvBuffer){ // PARA LEER LA ENTRADA AINP Y AINN CONFIGURADAS EN EL REGISTRO DE INPMUX

	Start_LOW();
	//usleep(1);
	writeRegister(INPMUX, 0x23); // Select AIN2 como AINP Y AIN3 COMO AINN
	return readAINx(recvBuffer);
}

int readAIN4_5(char * recvBuffer){
	Start_LOW();
	//usleep(1);
	writeRegister(INPMUX, 0x45); // Select AIN4 como AINP Y AIN5 COMO AINN
	return readAINx(recvBuffer);
}

int readAIN6_7(char * recvBuffer){
	Start_LOW();
	//usleep(1);
	writeRegister(INPMUX, 0x67); // Select AIN6 como AINP Y AIN7 COMO AINN
	return readAINx(recvBuffer);
}

double getVoltage(char *ADC_data, double VREF){
	unsigned long adc_count = 0;
	double resolution = 0;
	double Vol_V = 0;
	//double Vol_mV = 0;

	adc_count = (((unsigned long)ADC_data[1]<<24)|((unsigned long)ADC_data[2]<<16)|(ADC_data[3]<<8)|ADC_data[4]);
	resolution = VREF / pow(2.0, 31);

	Vol_V = resolution * (double) adc_count;
	//Vol_mV = Vol_V * 1000;


	//printf("\nadc_count es: %lu\n", adc_count);
	//printf("RESOLUTION es: %lu\n", resolution);
	//printf("Vol_V es: %f\n", Vol_V);
	//printf("Vol_mV es: %f\n", Vol_mV);

	return Vol_V;
}

void errorADC(char *msgError){
	printf("\nError ADCLIB: %s\n",msgError);
}

