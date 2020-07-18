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

static uint16_t signalBufferOnePeriod[SIGNAL_BUFFER_LENGTH];
static uint16_t signalBufferTwoPeriod[SIGNAL_BUFFER_LENGTH];
uint32_t * currSignalBuffer;

void updateBit(void);

//FUNCIONES

//FUNCIONAMIENTO:
//se popula el buffer con la señal que se desea mandar al DAC. CHEQUEAR LOS VALORES MAXIMOS Y MINIMOS PARA ESCALAR LOS VALORES DE LA SEÑAL QUE VAN AL DAC.
//la frecuencia con la que tengo que mandar valores al DAC es (el tamaño del buffer a vaciar)*(la f deseada) FIJARSE A QUE F FUNCIONA AL TIMER CONTADOR
//

//
static char checkParity(unsigned char myChar);

//
static void populateSignalBufferWithSin();

//
static void prep_init_FTM(FTM_params * params);

//
static void escribir2(void);

//
static int get_frec(void);

//
static int sendAcumulator = 0;

//
static int getBit = 0;

//
static int endOfIDLE=1;

//
static int SendViaFSK = 0;

//arreglo circular de 1s
static char char2send;

//
static char pariedad;

//
static int dataSentStatus = 1; //permite mandar chars para ecribirlos.

//
uint32_t temp_duty[1];

//
static void populateSignalBufferWithSin()
{
	int i=0;
	for(i=0;i<SIGNAL_BUFFER_LENGTH;i++)
	{
        double j = i;
	    //set offset for DAC range 0 a 4096 siendo 0 0V y 4096 3.3V
        signalBufferOnePeriod[i]=(uint16_t) ((1+sin((j*(2*M_PI))/SIGNAL_BUFFER_LENGTH))*50*getMod()/100);
        /*if(signalBufferOnePeriod[i]==0)
        	signalBufferOnePeriod[i]=1;
        if(signalBufferOnePeriod[i]==1000)
        	signalBufferOnePeriod[i]=999;*/
        signalBufferTwoPeriod[i]=(uint16_t) ((1+sin((j*(4*M_PI))/SIGNAL_BUFFER_LENGTH))*50*getMod()/100);
        /*if(signalBufferTwoPeriod[i]==0)
        	signalBufferTwoPeriod[i]=1;
        if(signalBufferTwoPeriod[i]==1000)
        	signalBufferTwoPeriod[i]=999;*/
	}
}

//
static void escribir2()
{
	//Leo la frecuencia
	update_PWM(temp_duty[0]);
	/*if(SendViaFSK>1)
	{ 			//solo puede valer mas de 1 SendViaFSK si se llamo a SendCgarViaFSK
		//COMIENZO DE MANDADA DE UN CHAR
		if(sendAcumulator<SIGNAL_BUFFER_LENGTH*2 && !endOfIDLE)
		{
			update_PWM(signalBufferOnePeriod[sendAcumulator]);
			sendAcumulator = sendAcumulator+1;
		}
		else
		{
			endOfIDLE=1;
			if(sendAcumulator>=SIGNAL_BUFFER_LENGTH)
			{
				//si estoy en que tengo que mandar un 1 y llego a signalbufferlength entonces tengo que poner mi contador a 0
				sendAcumulator = 0;
				getBit = char2send&1;
				char2send = char2send>>1;
				SendViaFSK--;
			}
			if(getBit && SendViaFSK>1)
			{ 	//cuando arranca getBit vale 0 por lo que se manda un startBit
				//estoy mandando un 1
				update_PWM(signalBufferOnePeriod[sendAcumulator]);
				sendAcumulator = sendAcumulator+1;
			}
			else if(!getBit && SendViaFSK>1)
			{
				//estoy mandando un 0
				update_PWM(signalBufferTwoPeriod[sendAcumulator]);
				sendAcumulator = sendAcumulator+1;
			}
		}

	}
	else
	{
		//ENVIO DEL BIT DE PARIEDAD
		if(SendViaFSK)
		{ //SendViaFSK == 1 si o si, no puede ser mayor
			//envio pariedad
			if(sendAcumulator>=SIGNAL_BUFFER_LENGTH)
			{
				//si estoy en que tengo que mandar un 1 y llego a signalbufferlength entonces tengo que poner mi contador a 0
				sendAcumulator = 0;
				SendViaFSK--;
			}
			if(pariedad && SendViaFSK)
			{ 					//cuando arranca getBit vale 0 por lo que se manda un startBit
				//estoy mandando un 1
				update_PWM(signalBufferOnePeriod[sendAcumulator]);
				sendAcumulator = sendAcumulator+1;
			}
			else if(!pariedad && SendViaFSK)
			{
				//estoy mandando un 0
				update_PWM(signalBufferTwoPeriod[sendAcumulator]);
				sendAcumulator = sendAcumulator+1;
			}
		}
		else
		{
			//STOP BIT = 1
			//termine de mandar asi que pongo en IDLE
			if(sendAcumulator>=SIGNAL_BUFFER_LENGTH)
			{
				//si estoy en que tengo que mandar un 1 y llego a signalbufferlength entonces tengo que poner mi contador a 0
				sendAcumulator = 0;
				dataSentStatus=1;
				getBit=0;
			}
			//estoy mandando un 1
			update_PWM(signalBufferOnePeriod[sendAcumulator]);
			sendAcumulator = sendAcumulator+1;
			//ESTADO IDLE
			//MUCHOS UNOS. EN CASO QUE NO ESTE TESTING: EL CODIGO DE ARRIBA NO VA Y DATA SENT STATUS ABAJO PAASA A 1
		}
	}*/
}

//
int sendData()
{
	return dataSentStatus;
}

//
void SendCharViaFSK(char a)
{
	if(sendData())
	{
		dataSentStatus=0;
		endOfIDLE=0;
		pariedad = checkParity(a);
		char2send = a;
		SendViaFSK= 10;
	}
}

//
static char checkParity(unsigned char myChar)
{
  int parity = 0;

  while(myChar)
  {    //while myChar != 0
    parity += (myChar&1);   //add result of myChar AND 1 to parity
    myChar = myChar>>1;     //shift bits left by 1
  }
  //printf("parity equals: %d\n", parity);
  if(parity%2)
  { // if odd parity
    return 1;
  }
  else
  { //even parity
    return 0;
  }
}

//
void FSK_init()
{
	//inicializo DAC
	DAC_Init();

	//INICIALIZO PIT
	pitInit(0);

	//Inicializo FTM

	//Inicializo el comparador
	comparator_init(0);

	FTM_params ftm_vars;
	prep_init_FTM(&ftm_vars);
	FTM_init(ftm_vars);

	//Creo el buffer con las señales senoidales
	populateSignalBufferWithSin();

	DMA1_Config(signalBufferOnePeriod, temp_duty, updateBit);

	DMA0_ConfigCounters(1, sizeof(signalBufferOnePeriod), sizeof(signalBufferOnePeriod[0]));

	DMA0_EnableRequest(1);




}

//
void updateBit(void)
{
	if(!dataSentStatus)
	{
		DMA0_DisableRequest(1);
		//COMIENZO A MANDAR EL BITSTREAM SI
		if(SendViaFSK==10)
		{
			//mando mi primer bit, un 0 de start, cuando llamo a sendCharViaFSK sendViaFSK se pone en 10
			DMA0_ConfigSourceAddress(1, signalBufferTwoPeriod);
			SendViaFSK = SendViaFSK-1;
			DMA0_EnableRequest(1);
		}
		else if(SendViaFSK>1)
		{
			//mando el bitstream del char para sendViaFSK entre 9 y 2 inclusives
			getBit = char2send & 0x01;
			if(getBit)
			{
				DMA0_ConfigSourceAddress(1, signalBufferOnePeriod);

			}
			else
			{
				DMA0_ConfigSourceAddress(1, signalBufferTwoPeriod);
			}
			//decremento sendViaFSK que es mi contador de bits a mandar, y shifteo char2send para mandar
			//el proximo bit
			DMA0_EnableRequest(1);
			SendViaFSK=SendViaFSK-1;
			char2send = char2send>>1;

		}
		else if (SendViaFSK)
		{
			//mando la pariedad
			getBit = pariedad;
			if(getBit)
			{
				DMA0_ConfigSourceAddress(1, signalBufferOnePeriod);
			}
			else
			{
				DMA0_ConfigSourceAddress(1, signalBufferTwoPeriod);
			}
			DMA0_EnableRequest(1);
			SendViaFSK=SendViaFSK-1;
		}
		else
		{
			//aca SendViaFSK llego a 0 y queda solo mandar el stopBit
			//y colocar dataSentStatus en 1 para permitir llamados a sendViaFSK y pasar a estado IDLE
			DMA0_ConfigSourceAddress(1, signalBufferOnePeriod);
			DMA0_EnableRequest(1);
			dataSentStatus=1;
		}
	}
	else
	{
		DMA0_DisableRequest(1);
		//ESTADO IDLE MANDO 1s
		DMA0_ConfigSourceAddress(1, signalBufferOnePeriod);
		DMA0_EnableRequest(1);
	}
}

//Contador de semi-periodos de 0s
int period_0_cnt=0;

//Contador de semi-periodos de 1s
int period_1_cnt=0;

//Si el frec_counter esta asociado a un 1 lógico se devuelve un 1, en caso de recibir el primer periodo de una señal de 0 se devuelve 0, de lo contrario se devuelve 2
static int get_frec(void)
{
	//Excepto que haya encontrado un valor válido me quedo con el caso de que no encontre nada.
	int retVal=2;

	//Me fijo cuantos ticks hubo entre los flancos del input capture
	float frec_meas= get_frec_count();

	//Si la cantidad de flancos esta dentro de la tolerancia para detectar un 1
	if( ( (uint32_t)((float)(2*FREC_1)/(1+TOLERANCE)) < (uint32_t)(frec_meas) ) && ( (uint32_t)((float)(2*FREC_1)/(1-TOLERANCE)) > (uint32_t)(frec_meas) ) )
	{

		period_0_cnt=0;

		//Dado que input capture funciona con ambos rising y falling edge, en realidad estoy midiendo medio periodo de la senoidal
		//Si registre el primer semi-periodo de la senoidal
		if(period_1_cnt==0)
		{
			//Es un 1 lógico
			retVal=1;

			//Actualizo el contador de medios-periodos utilizado
			period_1_cnt=1;
		}

		//Si registro el segundo semi-periodo no mando ningun bit solo actualizo el contador de semi-periodos
		else if(period_1_cnt==1)
		{
			period_1_cnt=0;
		}
	}

	//Si la cantidad de flancos esta dentro de la tolerancia para detectar un 0
	else if( ( (uint32_t)((float)(2*FREC_0)/(1+TOLERANCE)) < (uint32_t)(frec_meas) ) && ( (uint32_t)((float)(2*FREC_0)/(1-TOLERANCE)) > (uint32_t)(frec_meas) ) )
	{
		period_1_cnt=0;

		//Dado que input capture funciona con ambos rising y falling edge, en realidad estoy midiendo medio periodo de la senoidal
		//Si registre el primer semi-periodo de la senoidal
		if(period_0_cnt==0)
		{
			//Si es el primer periodo marco el 0
			retVal=0;

			//Actualizo el contador de medios-periodos utilizado
			period_0_cnt=1;
		}

		//Si registro el segundo  o tercer semi-periodo (un 0 son dos periodos de la senoidal de más alta frecuencia, por lo que hay 4 semi-periodos)
		//no mando ningun bit solo actualizo el contador de semi-periodos
		else if(period_0_cnt==1 || period_0_cnt==2)
		{
			period_0_cnt+=1;
		}

		//Si registro el cuarto semi-periodo no mando ningun bit solo actualizo el contador de semi-periodos
		else if(period_0_cnt==3)
		{
			period_0_cnt=0;
		}
	}
	return retVal;
}

//Flag que se utiliza para ver si se esta recibiendo algun mensaje de Uart por FSK actualmente
bool no_uart_msg=true;

//Contador de bits de la transferencia actual
int uart_bit_cnt=0;

//Char temporario donde se va guardando el char recibido en la transferencia
char temp_uart_char=0;

//
uint32_t buffer_temp[50];

//
int index=0;

//
bool creat_uart_char(char* msg)
{
	int logic_rcv;

	//Bool que avisa si ya se recibio un char completo
	bool finished=false;

	//Si hay una frecuencia leida disponible
	if(is_frec_count_rdy())
	{
		//Leo la frecuencia
		logic_rcv= get_frec();
		buffer_temp[index]=logic_rcv;
		index+=1;
		if(index==49)
		{
			index=0;
		}



		//Si no hay mensajes pendientes y me llega un bit de start arranco a recibir un mensaje
		if(logic_rcv==0 && no_uart_msg)
		{
			//Estoy recibiendo un mensaje de Uart por FSK
			no_uart_msg=false;
			uart_bit_cnt++;
		}

		//Si la frecuencia recibida es alguna de las válidas
		else if( (logic_rcv==0 || logic_rcv==1) && !no_uart_msg)
		{
			//Si la frecuencia recibida no es la de Start empieza a crear el char
			if(uart_bit_cnt>0 && uart_bit_cnt<=9)
			{
				//Sin tener en cuenta el bit de paridad, agrego el bit nuevo y muevo el contador de bits
				if(uart_bit_cnt<=8)
					temp_uart_char |= logic_rcv<<(uart_bit_cnt-1);
				uart_bit_cnt++;
			}

			//Cuando me llegue un stop después de la paridad
			else if(uart_bit_cnt==10 && logic_rcv==true)
			{
				//Se termino de leer el mensaje
				finished=true;

				//Reseteo el contador de bits de Uart
				uart_bit_cnt=0;

				//No hay un mensaje pendiente de Uart
				no_uart_msg=true;

				//Guardo el char creado para el usuario
				*msg=temp_uart_char;

				//Limpio el char temporario
				temp_uart_char=0;
			}
		}
	}
	return finished;
}

//
void prep_init_FTM(FTM_params * params)
{
	int i;
	for(i=0;i<8;i++)
	{
		params->FTM_clk_POL[i]=true;
	}
	params->FTM_num=0;
	params->FTM_init_cnt=0;
	params->FTM_adv_func=true;
	params->FTM_prescaler=0;
	params->FTM_clk_mod=1500;
	params->FTM_clk_select=1;
	params->ftm_callback = escribir2;
}
