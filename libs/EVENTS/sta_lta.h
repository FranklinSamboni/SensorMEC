
#ifndef _GPS_H_
#define _GPS_H_

	typedef struct paramsSTA_LTA paramsSTA_LTA;

	struct paramsSTA_LTA {
		int freq;
		int lengthSta;
		int lengthLta;
		float thOn;
		float thOff;
		float minimunDurationSeconds;
		char ltaMode[32];
	};


#endif
