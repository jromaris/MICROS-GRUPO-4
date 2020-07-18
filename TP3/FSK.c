/*
 * FSK.c
 *
 *  Created on: 18 oct. 2019
 *      Author: Nicolas
 *
 */
#include <stdio.h>
#include <math.h>
#include "FSK.h"
#include "DAC.h"
#include "pit.h"
#include "gpio.h"
#include "board.h"


#define MODO1
//#define MODO2
#define OUTPUT_FRECUENCY_0 (SIGNAL_BUFFER_LENGTH*2400)
#define OUTPUT_FRECUENCY_1 (SIGNAL_BUFFER_LENGTH*1200)
#define PIT_SET_RATE_OUT
#define M_PI 3.14159265358979323846
static int signalBuffer[SIGNAL_BUFFER_LENGTH];


//FUNCIONES

//FUNCIONAMIENTO:
//se popula el buffer con la señal que se desea mandar al DAC. CHEQUEAR LOS VALORES MAXIMOS Y MINIMOS PARA ESCALAR LOS VALORES DE LA SEÑAL QUE VAN AL DAC.
//la frecuencia con la que tengo que mandar valores al DAC es (el tamaño del buffer a vaciar)*(la f deseada) FIJARSE A QUE F FUNCIONA AL TIMER CONTADOR
//


void populateSignalBufferWithSin(){
	int i=0;
	for(i=0;i<SIGNAL_BUFFER_LENGTH;i++){
        double j = i;
	    //set offset for DAC range 0 a 4096 siendo 0 0V y 4096 3.3V
        signalBuffer[i]=(int) ((1+sin((j*(2*M_PI))/SIGNAL_BUFFER_LENGTH))*2048);
	}
}

int* getSignalBuffer(){
	return signalBuffer;
}

static float FaseAcummulator=0;
static float sendAcumulator = 0;
static int i = 0;
static int* AppSignalBuffer;
static int init = 0;
static int bol = 1;

void escribir(){
//	gpioWrite(TOGLEPIN, 1);
	if(init){
		if (FaseAcummulator>255 ){
			FaseAcummulator=0;
		}
		i = FaseAcummulator;
		DAC_SetData(DAC0,AppSignalBuffer[i]);
		if(bol){
			FaseAcummulator= FaseAcummulator+6.12;
		}else{
			FaseAcummulator= FaseAcummulator+12.24;
		}
		sendAcumulator = sendAcumulator + 6.12;
		if(sendAcumulator>255 ){
			sendAcumulator = 0;
			bol = !bol;
		}
		//setPitValue(0,OUTPUT_FRECUENCY_0); //esto me retraza x4 mi recorrida del buffer
/*		if(i==(SIGNAL_BUFFER_LENGTH)){
			//ACA TENGO QUE CHEQUEAR CON QE FRECUENCIA CARGO EL TIMER PARA EL SIGUIENTE DATO
			//LLAMO A TIMER CON MI SIGUIENTE FREC.
			if(bol){
				setPitValue(0,OUTPUT_FRECUENCY_1);
				bol = 0;
			}else{
				setPitValue(0,OUTPUT_FRECUENCY_0);
				bol = 1;
			}
		}*/
	}else{
		populateSignalBufferWithSin();
		AppSignalBuffer = getSignalBuffer();
		init = 1;
	}
//	gpioWrite(TOGLEPIN, 0);
}


void FSK_init(){
	//inicializo DAC
	DAC_Init();
	//INICIALIZO PIT
	pitInit(0);
	pitStart(0,50000,escribir);
	gpioMode(TOGLEPIN, OUTPUT);
}



