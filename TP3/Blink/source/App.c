#define V2

#ifdef V2
/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "fsk.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
uart_cfg_t uartConfig;

void App_Init (void)
{
	//Inicializo Uart
	uart_cfg_t var;
	var.parity=0;
	var.baudrate=1200;
	uartInit (0, var);

	//Inicializo el módulo de FSK
	FSK_init();
}

//Char temporario donde se va a guardar el mensaje que se desea mandar por Uart
char read;

//Arreglo de Chars donde se guarda lo recibido por Uart
char msj[1];

char temp[50];
int indx=0;

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	//Si hay algun mensaje para recibir de Uart
	if(uartIsRxMsg(0))
	{
		//Y si no se esta mandando nada por FSK en el momento
		if(sendData())
		{
			//Leo el mensaje de Uart
			uartReadMsg(0,msj,1);

			//Lo modulo en FSK y lo mando por el canal
			SendCharViaFSK(msj[0]);
		}
	}

	/*if(sendData())
			{
				//Leo el mensaje de Uart
				//Lo modulo en FSK y lo mando por el canal
				SendCharViaFSK('a');
			}*/
	//Si la cola de entrada de Uart no esta llena
	if(uartIsTxMsgComplete(0))
	{
		//Si hay algun mensaje recibido en el canal
		if( creat_uart_char(&read) )
		{
			//Si hay algun mensaje lo mando por Uart
			uartWriteMsg(0,&read,1);
			temp[indx]=read;
			if(indx==49)
			{
				indx=0;
			}
			indx+=1;
		}
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
#endif /* DMA_H_ */


#ifdef V1


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
#include "FSKV1.h"
#include "gpio.h"
#include "DAC.h"
#include "timer.h"
#include "pit.h"
#include "buffer.h"
#include "uart.h"
#include "demodulador_FSK.h"

char testChar;

void sendChar(void);

static void delayLoop(uint32_t veces);


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */


char msjByte;
buff_id_t bufferId;
uart_cfg_t uartConfig;
void App_Init (void)
{
	demFSKfun_t uartCallback=sendChar;
	uartConfig.baudrate=1200;
	uartConfig.parity=0; //????????????
	uartInit(0, uartConfig);
	FSK_initV1();
	init_dem_FSK(&testChar,uartCallback);
}
/* Función que se llama constantemente en un ciclo infinito */


int i = 0;
char msj[1];
void App_Run (void)
{

	if(uartIsRxMsg(0)){
		//si tengo cosas en mi buffer de uart
		//guardo 4 bytes en mi buffer msj
		if(sendDataV1()){
			uartReadMsg(0, msj, 1);
			SendCharViaFSKV1(msj[0]);
		}
	}
	if(isDemDataReady())
	{
		processDemData();
	}

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void delayLoop(uint32_t veces)
{
    while (veces--);
}


void sendChar(void)
{
	uartWriteMsg(0,&testChar, 1);
}

/*******************************************************************************
 ******************************************************************************/


#endif
