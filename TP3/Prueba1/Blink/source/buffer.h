/*
 * buffer.h
 *
 *  Created on: 2 oct. 2019
 *      Author: Tomas
 */

#ifndef BUFFER_H_
#define BUFFER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MAX_BUFFERS 10
#define BUFFER_SIZE	500
#define BUFFER_INVALID_ID 255
#define TRUE 1
#define FALSE 0

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum { INCREASE, DECREASE } indexActions;

typedef uint8_t buff_id_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


//initialice buffer
// -checks if there is a buffer available
// -sets all the values of the corresponding buffer for it to be used
// -returns the id the user is going to use to identify its buffer

buff_id_t bufferInitAndGetId(void);

// UART data transmit function
//  - checks if there's room in the transmit sw buffer
//  - if there's room, it transfers data byte to sw buffer
//  - automatically handles "uart_tx_buffer_full_flag"
//  - sets the overflow flag upon software buffer overflow (doesn't overwrite existing data)
//  - if this is the first data byte in the buffer, it enables the "hw buffer empty" interrupt
void push_buffer(uint8_t byte, buff_id_t id);


// UART data receive function
//  - checks if data exists in the receive sw buffer
//  - if data exists, it returns the oldest element contained in the buffer
//  - automatically handles "uart_rx_buffer_full_flag"
//  - if no data exists, it clears the uart_rx_flag
uint8_t pop_buffer(buff_id_t id);


/****************seters y getters*******/

//Indicates if buffer is complete
bool buffer_is_full(buff_id_t id);
//indicates if buffer is empty
bool buffer_is_not_empty(buff_id_t id);
//return overflow status
bool buffer_is_overflowed(buff_id_t id);


#endif /* BUFFER_H_ */
