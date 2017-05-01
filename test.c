#include "stdio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

//int round(float myfloat);
void printfOpt();

int main(int argc, char *argv[]){

	
	for (index = optind; index < argc; index++)
		printf ("Opciones %s\n", argv[index]);
	return 0;
}

/*
int round(float myfloat){
  double integral;
  float fraction = (float)modf(myfloat, &integral);
 
  if (fraction >= 0.5)
    integral += 1;
  if (fraction <= -0.5)
    integral -= 1;
 
  return (int)integral;
}*/

void printfOpt(){
	printf("La opción '-f' y su argumento es requerido. \nOpciones disponibles '-f 40' '-f 50' '-f 100' '-f 200'\n");
	printf("ej. './SensorIoT -f 200'\n");
	
	printf("\n(Opcional)\n");
	printf("Al activar la opción '-v' (Eventos). Debes ingresar obligatoriamente las opciones:\n");
	printf("'-s' segundos de la ventana STA				ejm. '-s 0.8'\n");
	printf("'-l' segundos de la ventana LTA 			emj. '-l 8'\n");
	printf("'-o' disparador 							emj. '-o 14.0'\n");
	printf("'-p' parador 								emj. '-p 10.0'\n");
	printf("'-m' dirección minima de eventos en segundos emj. '-m 3.0'\n");
	
	printf("\n Ejemplo : './SensorIoT -f 200 -v -s 0.8 -l 8 -o 14.0 -p 10.0 -m 3.0'\n");
}
