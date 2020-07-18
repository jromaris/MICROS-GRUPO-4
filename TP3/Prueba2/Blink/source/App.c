
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
#include "FSK.h"
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
	FSK_init();
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
		if(sendData()){
			uartReadMsg(0, msj, 1);
			SendCharViaFSK(msj[0]);
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
