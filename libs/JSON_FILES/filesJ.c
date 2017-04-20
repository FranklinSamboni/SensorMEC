#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "filesJ.h"


void basicJson(json_object * jobjData, char * status, char * descript, char * error){

	json_object *jstatus = json_object_new_string(status);
	json_object *jdescript = json_object_new_string(descript);
	json_object *jerror = json_object_new_string(error);

	json_object_object_add(jobjData,"status", jstatus);
	json_object_object_add(jobjData,"descript", jdescript);
	json_object_object_add(jobjData,"error", jerror);

}

void writeFileJson( char * dir, const char * data){
	FILE * file = fopen (dir, "w");
	fprintf(file,"%s",data);
	fclose(file);
}

void accelerometerJson(char * status, char * descript, char * error){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,ACCELEROMETER_FILE);

	json_object *jobjAccelerometerData = json_object_new_object();
	basicJson(jobjAccelerometerData,status,descript,error);

	writeFileJson(dir,json_object_to_json_string(jobjAccelerometerData));
	//return json_object_to_json_string(jobjAccelerometerData);
}

void adcJson(char * status, char * descript, char * samples,char * error){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,ADC_FILE);

	json_object *jobjAdcData = json_object_new_object();
	basicJson(jobjAdcData,status,descript,error);

	json_object *jsamples = json_object_new_string(samples);
	json_object_object_add(jobjAdcData,"samples", jsamples);

	writeFileJson(dir,json_object_to_json_string(jobjAdcData));
	//return json_object_to_json_string(jobjAdcData);

}

void cpuJson(char * status, char * descript, char * error){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,CPU_FILE);

	json_object *jobjCpuData = json_object_new_object();
	basicJson(jobjCpuData,status,descript,error);

	writeFileJson(dir,json_object_to_json_string(jobjCpuData));
	//return json_object_to_json_string(jobjCpuData);
}

void batteryJson(char * status, char * descript, char * charge, char * error){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,BATTERY_FILE);

	json_object *jobjBatteryData = json_object_new_object();
	basicJson(jobjBatteryData,status,descript,error);

	json_object *jcharge = json_object_new_string(charge);
	json_object_object_add(jobjBatteryData,"charge", jcharge);

	writeFileJson(dir,json_object_to_json_string(jobjBatteryData));
	//return json_object_to_json_string(jobjBatteryData);
}

void gpsJson(char * status, char * descript, char * baudRate, char * msjNMEA, char * error){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,GPS_FILE);

	json_object *jobjGpsData = json_object_new_object();
	basicJson(jobjGpsData,status,descript,error);

	json_object *jbaudRate = json_object_new_string(baudRate);
	json_object_object_add(jobjGpsData,"baudRate", jbaudRate);

	json_object *jmsjNMEA = json_object_new_string(msjNMEA);
	json_object_object_add(jobjGpsData,"msjNMEA", jmsjNMEA);

	writeFileJson(dir,json_object_to_json_string(jobjGpsData));
	//return json_object_to_json_string(jobjGpsData);

}

void rtcJson(char * status, char * descript, char * dateHour, char * error){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,RTC_FILE);

	json_object *jobjRtcData = json_object_new_object();
	basicJson(jobjRtcData,status,descript,error);

	json_object *jdateHour = json_object_new_string(dateHour);
	json_object_object_add(jobjRtcData,"dateHour", jdateHour);

	writeFileJson(dir,json_object_to_json_string(jobjRtcData));
	//return json_object_to_json_string(jobjRtcData);

}

void locationJson(char * latitude, char * longitude, char * altitude){

	char dir[255] = {0};
	sprintf(dir,"%s%s",COMPONENTS_DIR,LOCATION_FILE);

	json_object *jobjLocationData = json_object_new_object();

	json_object *jlatitude = json_object_new_string(latitude);
	json_object *jlongitude = json_object_new_string(longitude);
	json_object *jaltitude = json_object_new_string(altitude);
	json_object *jaddress = json_object_new_string("");

	json_object_object_add(jobjLocationData,"latitude", jlatitude);
	json_object_object_add(jobjLocationData,"longitude", jlongitude);
	json_object_object_add(jobjLocationData,"altitude", jaltitude);
	json_object_object_add(jobjLocationData,"address", jaddress);


	writeFileJson(dir,json_object_to_json_string(jobjLocationData));
	//return json_object_to_json_string(jobjLocationData);
}

