/*
 * pit.c
 *
 *  Created on: 21 oct. 2019
 *      Author: Tomas
 */


/***************************
 * INCLUDE HEADER FILES
 **************************/

#include "pit.h"
#include "MK64F12.h"
#include "hardware.h"
#include "board.h"


/***************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 **************************/


/***************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 **************************/


/***************************
 * VARIABLES WITH GLOBAL SCOPE
 **************************/

static pitIrqFun_t	irq_pointers[PIT_CANT_IDS];

/***************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 **************************/

void pitIRQ_handler(uint8_t pitID);
uint32_t configPitFreq(double ticks);

/***************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 **************************/

/***************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 **************************/

static SIM_Type *  sim=SIM;
static PIT_Type *  pits_ptr[]= PIT_BASE_PTRS; //{PIT0,PIT1,PIT2,PIT3}

/***************************
 ***************************
                        GLOBAL FUNCTION DEFINITIONS
 ***************************
 **************************/

void pitInit(uint8_t id){
	//habilito clock gaiting
	sim->SCGC6 |= SIM_SCGC6_PIT_MASK;
	//me aseguro que el modulo pit este habilitado
	pits_ptr[id]->MCR &= ~PIT_MCR_MDIS_MASK;
}

void pitStart(uint8_t id, double loadValue, pitIrqFun_t irqFun){
	//cargo el valor deseado del timer
	pits_ptr[0]->CHANNEL[id].LDVAL=configPitFreq(loadValue);
	//cargo la funcion que se desea ejecutar en la interrupcion
	irq_pointers[id] = irqFun;
	//habilito las interrupciones
	if(id==PIT0){
		__NVIC_EnableIRQ(PIT0_IRQn);
	}
	else if(id==PIT1){
		__NVIC_EnableIRQ(PIT1_IRQn);
	}
	else if(id==PIT2){
		__NVIC_EnableIRQ(PIT2_IRQn);
	}
	else if(id==PIT3){
		__NVIC_EnableIRQ(PIT3_IRQn);
	}
	pits_ptr[0]->CHANNEL[id].TCTRL |= PIT_TCTRL_TIE(1);
	//habilito el timer especifico
	pits_ptr[0]->CHANNEL[id].TCTRL |= PIT_TCTRL_TEN(1);
}

void stopPitTimer(uint8_t id){
	//deshabilito las interrupciones
	pits_ptr[0]->CHANNEL[id].TCTRL &= ~PIT_TCTRL_TIE_MASK;
	//deshabilito el timer especifico
	pits_ptr[0]->CHANNEL[id].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}


uint32_t getPitValue(uint8_t id){
	return pits_ptr[0]->CHANNEL[id].CVAL;
}

void setPitValue(uint8_t id, double ticks){
	pits_ptr[0]->CHANNEL[id].TCTRL &= ~PIT_TCTRL_TEN_MASK;
	pits_ptr[0]->CHANNEL[id].LDVAL = configPitFreq(ticks);
	pits_ptr[0]->CHANNEL[id].TCTRL |= PIT_TCTRL_TEN(1);
}

uint32_t configPitFreq(double ticks){
	uint32_t frq_return;
	//si me piden una frecuencia mayor a la que puedo manejar
	if(ticks > PIT_MAX_FREQ){
		 ticks= PIT_MAX_FREQ;
		 //le devuelvo la maxima frecuencia posible
	}
	//si me piden una frecuencia menor a la que puedo manejar
	else if(ticks < PIT_MIN_FREQ){
		ticks = PIT_MIN_FREQ;
	}
	//realizo la conversion para el contador de pit
	frq_return=PIT_BASE_FREQ/ticks;

	return frq_return;
}

__ISR__ PIT0_IRQHandler (void)
{
	pitIRQ_handler(PIT0);
}
__ISR__ PIT1_IRQHandler (void)
{
	pitIRQ_handler(PIT1);
}
__ISR__ PIT2_IRQHandler (void)
{
	pitIRQ_handler(PIT2);
}
__ISR__ PIT3_IRQHandler (void)
{
	pitIRQ_handler(PIT3);
}

//HACER DESPUES
void pitIRQ_handler(uint8_t pitID)
{
	//leo la isf, me fijo donde ocurrio la interrupcion
	//eso me lo dice pitID
	//para debuggear
	uint32_t flag = pits_ptr[0]->CHANNEL[pitID].TFLG;

	//ejecuto interrupcion
	irq_pointers[pitID]();


	//limpio el flag (w1c)
	pits_ptr[0]->CHANNEL[pitID].TFLG = PIT_TFLG_TIF(1);
}
