#ifndef _ADS1262_H_
#define _ADS1262_H_

/*
 * Esta libreria corresponde al ADS1262, que es un ADC de 32 bit de bajo ruido,
 * dise�ado por TI, se puede encontrar el data sheet de este dispositivo en:
 * http://www.ti.com/lit/ds/symlink/ads1262.pdf
 *
 * El ADC tiene una interfaz SPI con la cual se puede interacuar con el. A trav�s de la
 * interfaz SPI se realiza los procesos de configuracion de los registros del ADC, el ADC tiene
 * 27 registros de 8 bits. Ademas, con los comandos del ADC se puede leer y escribir los registros,
 * leer los datos de conversi�n, controlar los modos de conversion, etc.
 * */

	#include <stdint.h>
	#include <stdlib.h>
	#define DECIVE_SPI "/dev/spidev1.0"

	/*
	 * Pines de control del ADC, GPIOs de la Beagle bone black
	 */

	#define AIN2 "AIN2"
	#define AIN4 "AIN4"
	#define AIN6 "AIN6"
	#define INTERNAL_VREF 2.5
	#define VSUPPLY_VREF 5.0
	#define EXTERNAL1_VREF 3.4
	#define EXTERNAL2_VREF 1.8

	#define GPIO_RESET 9
	#define GPIO_DRDY 10
	#define GPIO_START 20
	/*
	 * Comandos del ADC.
	 * */

	#define RESET_COMMAND 0x06 // o 0x07

	#define START_CONVERSION 0x08 // o 0x09
	#define STOP_CONVERSION 0x0A // o 0x0B

	#define READ_DATA_COMMAND 12 // o 13

	#define READ_REG 0x20
	#define WRITE_REG 0x40

	/*
	 * Direcciones de los registros
	 */
	#define	POWER		0x01
	#define	INTERFACE	0x02
	#define	MODE0		0x03
	#define	MODE1		0x04
	#define	MODE2		0x05
	#define	INPMUX		0x06
	#define	OFCAL0		0x07
	#define	OFCAL1		0x08
	#define	OFCAL2		0x09
	#define	FSCAL0		0x0A
	#define	FSCAL1		0x0B
	#define	FSCAL2		0x0C
	#define	IDACMUX		0x0D
	#define	IDACMAG		0x0E
	#define	REFMUX		0x0F
	#define	TDACP		0x10
	#define	TDACN		0x11
	#define	GPIOCON		0x12
	#define	GPIODIR		0x13
	#define	GPIODAT		0x14
	#define	ADC2CFG		0x15
	#define	ADC2MUX		0x16
	#define	ADC2OFC0	0x17
	#define	ADC2OFC1	0x18
	#define	ADC2FSC0	0x19
	#define	ADC2FSC1	0x1A

	typedef struct adcStr adcStr;

	struct adcStr {
		int file; 	   			/* Identificador del archivo. */
		char device[24];
		uint8_t mode;
		uint8_t bits;
		uint32_t speed;
		uint16_t delay;
		//struct termios options; /* Opciones de configuración del UART. */
	};

	int openSPI(char * SPIDevice);
	int closeSPI();

	void settingTestADC();
    void settingADC();
    void settingPins();
    void resetADC();
    void resetADCByCommand();

	int transfer(unsigned char * sendBuffer, unsigned char * recvbuffer, int length);

    int writeRegister(unsigned char address, unsigned char data);
    int readRegister(char  * recvBuffer , unsigned char address);

    int writeCommand(unsigned char command);

    int readData(char * recvBuffer);
    int readDataByCommand(char * recvBuffer);

    int readAINx(char * recvBuffer);

    int readAIN2_3(char * recvBuffer);
    int readAIN4_5(char * recvBuffer);
    int readAIN6_7(char * recvBuffer);

    double getVoltage(char *ADC_data, double VREF);

    void Start_HIGH();
    void Start_LOW();

    void StartConversionByCommand();
    void StopConversionByCommand();

	void errorADC(char *msgError);


#endif
