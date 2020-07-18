/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/

// On Board User LEDs
#define PIN_LED_RED     // ???
#define PIN_LED_GREEN   // ???
#define PIN_LED_BLUE    PORTNUM2PIN(PB,21) // PTB21

#define LED_ACTIVE      LOW


// On Board User Switches
#define PIN_SW2         // ???
#define PIN_SW3         // ???

#define SW_ACTIVE       // ???
#define SW_INPUT_TYPE   // ???

#define PIN_UART0_RX	PORTNUM2PIN(PB,16)
#define PIN_UART0_TX	PORTNUM2PIN(PB,17)

#define PIN_SCK			PORTNUM2PIN(PD,1)
#define PIN_MOSI		PORTNUM2PIN(PD,2)
#define PIN_MISO		PORTNUM2PIN(PD,3)
#define PIN_SS			PORTNUM2PIN(PD,0)

#define IRQ_CAN			PORTNUM2PIN(PB,2)

#define PIN_FTM0_CH0	PORTNUM2PIN(PC,1)
#define PIN_FTM0_CH1	PORTNUM2PIN(PC,2)

#define PIN_CMP0_IN0	PORTNUM2PIN(PC,6)
#define PIN_CMP0_IN1	PORTNUM2PIN(PC,7)
#define PIN_CMP0_OUT	PORTNUM2PIN(PC,5)


/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
