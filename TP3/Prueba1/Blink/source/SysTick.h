/***************************************************************************//**
  @file     SysTick.h
  @brief    SysTick driver
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


//el clock de la fpga-k64 es 100MHz
#define K64_CLOCK_FREQUENCY				100000000U
//la frecuencia de systick es en base a la del clock
#define SYSTICK_ISR_FREQUENCY_HZ		10000U

//tiempo real en que se llama systick
#define REAL_FREQ_SYSTICK_IS_CALLED		K64_CLOCK_FREQUENCY/SYSTICK_ISR_FREQUENCY_HZ
//cantidad de veces q se llama en milisegundos
#define REAL_FREQ_SYSTICK_MS			REAL_FREQ_SYSTICK_IS_CALLED/1000


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef void (*SysTickFun_t)(void);


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize SysTic driver
 * @param funcallback Function to be call every SysTick
 * @return Initialization and registration succeed
 */
bool SysTick_Init (SysTickFun_t SysFunction);


/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_
