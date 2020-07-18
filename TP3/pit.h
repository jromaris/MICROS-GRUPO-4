/*
 * pit.h
 *
 *  Created on: 21 oct. 2019
 *      Author: Tomas
 */

#ifndef PIT_H_
#define PIT_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
enum { PIT0, PIT1, PIT2, PIT3 };

#define PIT_CANT_IDS	4
#define PIT_BASE_FREQ	50000000U

//registro 32 bits de pit con el valor 1 (minimo valor(maxima frecuencia))
#define PIT_MAX_FREQ	50000000U
#define PIT_MIN_FREQ	0.02

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef void (*pitIrqFun_t)(void);

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize PIT driver
 * @param id pit peripheral that is going to be accessed
*/
void pitInit(void);

/**
 * @brief Sets the timeout period
 * @param id pit peripheral that is going to be accessed
 * @param loadValue the value wished for the timeout period
*/
void pitStart(uint8_t id, double loadValue, pitIrqFun_t irqFun);

/**
 * @brief Gets actual timer value
 * @param id pit peripheral that is going to be accessed
*/
uint32_t getPitValue(uint8_t id);

/**
 * @brief Sets actual timer value
 * @param id pit peripheral that is going to be accessed
 * @param ticks value in Hz for pit timer
*/
void setPitValue(uint8_t id, double ticks);

/**
 * @brief Stops actual timer
 * @param id pit peripheral that is going to be accessed
*/
void stopPitTimer(uint8_t id);


/*******************************************************************************
 ******************************************************************************/

#endif /* PIT_H_ */
