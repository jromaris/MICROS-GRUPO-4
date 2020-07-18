/*
 * SysTick.c
 *
 *  Created on: 3 sep. 2019
 *      Author: Tomas
 */

//includes
#include "SysTick.h"
#include "hardware.h"
#include "MK64F12.h"


//static variables for the file
static SysTick_Type * systemTimer = SysTick;
static SysTickFun_t SysTickFunction;


//functions
bool SysTick_Init (SysTickFun_t SysFunction){

	systemTimer->CTRL = 0x00; //lo declaro
	//timer es downcounter
	systemTimer->LOAD = SYSTICK_ISR_FREQUENCY_HZ; //duracion timer,
	systemTimer->VAL = 0x00;
	SysTickFunction = SysFunction;		//guardo funcion de callback para el handler
	systemTimer->CTRL = SysTick_CTRL_CLKSOURCE_Msk | //clock del procesador
						SysTick_CTRL_TICKINT_Msk | 	//habilito las excepciones (interrupcion)
						SysTick_CTRL_ENABLE_Msk;	//lo habilito
	__NVIC_EnableIRQ(SysTick_IRQn);
	return 1;	//falta hacer verificacion
}

__ISR__ SysTick_Handler (void){
	SysTickFunction();
}
