
#ifndef _FILESJ_H_
#define _FILESJ_H_

	#include <json/json.h>

	#define CORRECT_STATUS_COMPONENT  "Correcto"
	#define ERROR_STATUS_COMPONENT  "Error"

	#define COMPONENTS_DIR "/home/debian/Sensor-IOT/SensorIoT/componentsFiles/"
	#define ACCELEROMETER_FILE "accelerometer.json"
	#define ADC_FILE "adc.json"
	#define CPU_FILE "cpu.json"
	#define BATTERY_FILE "battery.json"
	#define GPS_FILE "gps.json"
	#define RTC_FILE "rtc.json"
	#define LOCATION_FILE "location.json"

	void basicJson(json_object * jobjData, char * status, char * descript, char * error);
	void writeFileJson(char * dir, const char * data);
	void accelerometerJson(char * status, char * descript, char * error);
	void adcJson(char * status, char * descript, char * samples,char * error);
	void cpuJson(char * status, char * descript, char * error);
	void batteryJson(char * status, char * descript, char * charge, char * error);
	void gpsJson(char * status, char * descript, char * baudRate, char * msjNMEA, char * error);
	void rtcJson(char * status, char * descript, char * dateHour, char * error);
	void locationJson(char * latitude, char * longitude, char * altitude);
#endif

