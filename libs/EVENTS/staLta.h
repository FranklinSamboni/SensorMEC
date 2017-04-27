
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
		char ltaMode[32];
	};

	struct eventData {

		int isPendingSaveEvent;

		int eventNum;
		int EVENT_ON;

		char date[15];
		char time[15];
		char axis[15];
		int isGPS;

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
	void setParamsSTA_LTA(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, int minimunDurationSeconds);
	void sta_lta(eventData *  event, float * newSamples, char * axis, char * date, char * time, int isGPS);
	void inizializeFirstSamples(eventData *  event);
	void detectEvent(eventData *  event, float sample, char *axis,char * date, char * time , int isGPS);
	void checkMinimunDuration(eventData *  event, char * date, char *time);
	void moveRegister(float * tempData ,float * newSamples);
	int getSeconds(char * date, char *time);
	/*void createEventFile(char *axis,char * date, char *time);
	void initDataofSamples(char * date, char *time, int isGPS);
	void writeSac(int npts, float *arr, float dt, char *axis, char *filename);*/

#endif
