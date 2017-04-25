
#ifndef _GPS_H_
#define _GPS_H_

	typedef struct paramsSTA_LTA paramsSTA_LTA;

	struct paramsSTA_LTA {
		int freq;
		float staSeconds;
		float ltaSeconds;
		float thOn;
		float thOff;
		float minimunDurationSeconds;
		char ltaMode[32];
	};

	struct gpsData {
		//char gpgga[14][24];
		//char gprmc[14][24];

		char date[15], time[15];
		char lat[15], lng[15];
		char alt[15];

	};


#endif
