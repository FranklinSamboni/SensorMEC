
#ifndef _STALTA_H_
#define _STALTA_H_


	typedef struct paramsSTA_LTA paramsSTA_LTA;

	struct paramsSTA_LTA {
		int freq;
		int lengthSTA;
		int lengthLTA;
		float thOn;
		float thOff;
		float minimunDurationSeconds;
		char ltaMode[32];
	};

	void defaultParams(int freq);
	void setParamsSTA_LTA(int freq,  float staSeconds, int ltaSeconds, float thOn, float thOff, float minimunDurationSeconds);
	void sta_lta(float * newSamples, char * axis, char * date, char * time, int isGPS);
	void inizializeFirstSamples(float * tempData);
	void detectEvent(int count, float sample, char * axis, char * date, char * time , int isGPS);
	void checkMinimunDuration(char * axis, char * date, char * time);
	void movePositionRegister(float * newSamples);
	/*void createEventFile(char *axis,char * date, char *time);
	void initDataofSamples(char * date, char *time, int isGPS);
	void writeSac(int npts, float *arr, float dt, char *axis, char *filename);*/

#endif
