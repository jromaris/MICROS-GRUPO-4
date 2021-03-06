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
static char checkParity(unsigned char myChar);

static void populateSignalBufferWithSin();

static int* getSignalBuffer();

static void escribir();

static float FaseAcummulator=0;
static float sendAcumulator = 0;
static int i = 0;
static int* AppSignalBuffer;
static int getBit = 0;
static int SendViaFSK = 0;
//arreglo circular de 1s
static char char2send;
static char pariedad;
static int dataSentStatus = 1; //permite mandar chars para ecribirlos.


static int* getSignalBuffer(){
	return signalBuffer;
}

static void populateSignalBufferWithSin(){
	int i=0;
	for(i=0;i<SIGNAL_BUFFER_LENGTH;i++){
        double j = i;
	    //set offset for DAC range 0 a 4096 siendo 0 0V y 4096 3.3V
        signalBuffer[i]=(int) ((1+sin((j*(2*M_PI))/SIGNAL_BUFFER_LENGTH))*2048);
	}
}

static void escribir(){
	//gpioWrite(TOGLEPIN, 1);
	if(SendViaFSK>1){ 			//solo puede valer mas de 1 SendViaFSK si se llamo a SendCgarViaFSK
		//COMIENZO DE MANDADA DE UN CHAR
		if (FaseAcummulator>255 ){
				FaseAcummulator=0;
			}
		i = FaseAcummulator;
		DAC_SetData(DAC0,AppSignalBuffer[i]);
		if(getBit){ 					//cuando arranca getBit vale 0 por lo que se manda un startBit
			//estoy mandando un 1
			FaseAcummulator= FaseAcummulator+6.12;
		}else{
			//estoy mandando un 0
			FaseAcummulator= FaseAcummulator+11.22;
		}

		sendAcumulator = sendAcumulator + 6.12; //paso de 833us
		if(sendAcumulator>255 ){
			sendAcumulator = 0;
			//bol = !bol;
			//si estoy aca es que ya mande un dato
			getBit = char2send&1;
			char2send = char2send>>1;
			SendViaFSK--;
		}

	}else{
		//ENVIO DEL BIT DE PARIEDAD
		if(SendViaFSK){ //SendViaFSK == 1 si o si, no puede ser mayor
			//envio pariedad
			if (FaseAcummulator>255 ){
					FaseAcummulator=0;
				}
			i = FaseAcummulator;
			DAC_SetData(DAC0,AppSignalBuffer[i]);
			if(pariedad){ 			//me fijo cuanto me quedo que vale pariedad para mi char
				//estoy mandando un 1
				FaseAcummulator= FaseAcummulator+6.12;
			}else{
				//estoy mandando un 0
				FaseAcummulator= FaseAcummulator+11.22;
			}
			sendAcumulator = sendAcumulator + 6.12; //paso de 833us
			if(sendAcumulator>255 ){
				sendAcumulator = 0;
				SendViaFSK--; //SendVia FSK == 0 mientras no se llame a SendCharViaFSK
			}
		}else{
			//STOP BIT = 1
			//termine de mandar asi que pongo en IDLE
			if (FaseAcummulator>255 ){
					FaseAcummulator=0;
				}
				i = FaseAcummulator;
				DAC_SetData(DAC0,AppSignalBuffer[i]);
				FaseAcummulator= FaseAcummulator+6.12; //NO TENGO SEND ACUMULATOR PORQUE ASUMO MISMA FREC Q 1
			}
			if(FaseAcummulator>255){
				FaseAcummulator=0;
				dataSentStatus=1;
			}
			//ESTADO IDLE
			//MUCHOS UNOS. EN CASO QUE NO ESTE TESTING: EL CODIGO DE ARRIBA NO VA Y DATA SENT STATUS ABAJO PAASA A 1
		}
	//gpioWrite(TOGLEPIN, 0);
}

int sendData(){
	return dataSentStatus;
}

void SendCharViaFSK(char a){
	if(sendData()){
		dataSentStatus=0;
		pariedad = checkParity(a);
		char2send = a;
		SendViaFSK= 10;
	}
}

static char checkParity(unsigned char myChar){
  int parity = 0;

  while(myChar){    //while myChar != 0
    parity += (myChar&1);   //add result of myChar AND 1 to parity
    myChar = myChar>>1;     //shift bits left by 1
  }
  //printf("parity equals: %d\n", parity);
  if(parity%2){ // if odd parity
    return 1;
  }
  else { //even parity
    return 0;
  }
}


void FSK_init(){
	//inicializo DAC
	DAC_Init();
	//INICIALIZO FSK BUFFER SENOIDAL
	populateSignalBufferWithSin();
	AppSignalBuffer = getSignalBuffer();
	//INICIALIZO PIT
	pitInit(0);
	pitStart(0,50000,escribir); //intentar ver de reducir 50000. cuentas en mi cuaderno de IEEE
	//INTERRUPT TIME TEST PIN
	gpioMode(TOGLEPIN, OUTPUT);

}
