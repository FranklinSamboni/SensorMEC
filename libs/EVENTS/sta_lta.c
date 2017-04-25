#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/*
staSeconds	->   Ventana de tiempo STA
ltaSeconds 	-> 	 Ventana de tiempo LTA
thOn 		->	 Disparador de evento "ON" cuando la relacion STA/LTA excede su valor
thOff		-> 	 Parador de evento "OFF" cuando la relacion STA/LTA es menor que su valor
minimunDurationSeconds -> La duracion del disparo debe exceder su valor para ser almacenada
*/

paramsSTA_LTA params;

int eventNum = 0;
int EVENT_ON = 0;
int eventstart=0;
int eventend=0;

float trigArr[200] = {0};

float sta[200] = {0};
float lta[200] = {0};
float sta_to_lta[200] = {0};

float currentSamples [200] = {0};


void defaultParams(int freq){

	params.freq = freq;
	params.staSeconds = 0.7*freq;
	params.ltaSeconds = 7.0*freq;
	params.thOn = 3.0;
	params.thOff = 1.5;
	params.minimunDurationSeconds = 2.0;
	params.ltaMode = "";

}

void setParamsSTA_LTA(int freq,  float staSeconds, float ltaSeconds, float thOn, float thOff, float minimunDurationSeconds){

	params.freq = freq;
	params.staSeconds = staSeconds;
	params.ltaSeconds = ltaSeconds;
	params.thOn = thOn;
	params.thOff = thOff;
	params.minimunDurationSeconds = minimunDurationSeconds;
	params.ltaMode = "";

}

void sta_lta(){



    wave = fillgaps(wave,'interp'); % replace NaN values using splines
    wave = detrend(wave); % critical to work with detrended data
    y = abs(get(wave,'data')); % Absolute value of time series
    t = get(wave,'timevector');

    trig_array = [];
    sta = ones(size(y));
    lta = sta;
    sta_to_lta = sta;
}




