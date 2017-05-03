
#ifndef _STALTA_H_
#define _STALTA_H_

	#define MAX_LENGTH_EVENT 240000
	#define MAX_WINDOWS 72000

	typedef struct paramsSTA_LTA paramsSTA_LTA;
	typedef struct eventData eventData;

	struct paramsSTA_LTA {
		int freq;
		int lengthSTA;
		int lengthLTA;
		float thOn;
		float thOff;
		int minimunDurationSeconds;
		int preEvent;
		int postEvent;
	};

	struct eventData {

		int isPendingSaveEvent;
		int isEnableAddPostEvent;

		int eventNum;
		int EVENT_ON;

		char date[15];
		char time[15];
		char axis[15];
		int isGPS;

		float preEvent[MAX_LENGTH_EVENT];
		int countPreEvent;

		float postEvent[MAX_LENGTH_EVENT];
		int countPostEvent;

		float eventSamples[MAX_LENGTH_EVENT];
		int countEventSamples;

		float sta[MAX_WINDOWS];
		float lta[MAX_WINDOWS];
		float sta_to_lta[MAX_WINDOWS];
		int countLTA_STA;

		int countTempData;
		float tempData[MAX_WINDOWS];
	};

	void defaultParams(int freq);
	void setParamsSTA_LTA(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, int minimunDurationSeconds, int preEvent, int postEvent);
	void sta_lta(eventData *  event, float * newSamples, char * axis, char * date, char * time, int isGPS);
	void inizializeFirstSamples(eventData *  event);
	void addSamplesPreEvent(eventData *  event, float * samples, int samplesNumber , int firstPosition);
	void detectEvent(eventData *  event, float * newSamples, int countNewSamples, char *axis,char * date, char * time , int isGPS);
	int checkMinimunDuration(eventData *  event, char * date, char *time);
	void moveRegister(float * tempData ,float * newSamples, int samplesNumber, int bufferSize, int firstPosition);
	int getSeconds(char * date, char *time);
	void setStartTime(int seconds, char * date, char *time);
	void printfBuff(float * buffer, int ln, char * name);
	/*void createEventFile(char *axis,char * date, char *time);
	void initDataofSamples(char * date, char *time, int isGPS);
	void writeSac(int npts, float *arr, float dt, char *axis, char *filename);*/

#endif
