#ifndef _GPIOBBB_H_
#define _GPIOBBB_H_

	#define GPIO_PATH "/sys/class/gpio/"
	//typedef int (*CallbackType)(int);

	typedef enum gpioDirection gpioDirection;
	typedef enum gpioValue gpioValue;
	typedef enum gpioEdge gpioEdge;

	enum gpioDirection{ INPUT, OUTPUT };
	enum gpioValue{ LOW=0, HIGH=1 };
	enum gpioEdge{ NONE, RISING, FALLING, BOTH };

	typedef struct gpioParams gpioParams;

	struct gpioParams{
		int number;
		char name[10], path[24];
	};

	void initGPIO(int number, gpioParams* params);

	int exportGPIO(gpioParams* params);
	int unexportGPIO(gpioParams* params);

	int writeGPIOInt(char * path, char * filename, int value);
	int writeGPIO(char * path, char * filename, char * value);
	int readGPIO(char * path, char * filename, char * buffer);

	int setDirection(gpioDirection direction, gpioParams* params);
	int setValue(gpioValue value, gpioParams* params);
	int setEdgeType(gpioEdge edge, gpioParams* params);

	int setActiveLow(gpioParams* params);  //low=1, high=0
	int setActiveHigh(gpioParams* params);

	gpioDirection getDirection(gpioParams* params);
	gpioValue getValue(gpioParams* params);
	gpioEdge getEdgeType(gpioParams* params);

	int switchOutputValue(gpioParams* params); // toggleOutput

	void destroyGPIO(gpioParams* params);  //destructor will unexport the pin

	void eraseParams(gpioParams* params);
	void printErrorMsgGpio(char *msgError);

#endif
