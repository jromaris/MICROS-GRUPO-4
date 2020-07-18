/*
 * buffer.c
 *
 *  Created on: 2 oct. 2019
 *      Author: Tomas
 */

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "buffer.h"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
	bool enable;							//inicializacion
	uint8_t  data_buf[BUFFER_SIZE]; 		// FIFO buffer
	uint16_t i_first;                    	// index of oldest data byte in buffer
	uint16_t i_last;                     	// index of newest data byte in buffer
	uint16_t num_bytes;                  	// number of bytes currently in buffer
	bool fifo_full_flag;      				// this flag is automatically set and cleared by the software buffer
	bool fifo_ovf_flag;       				// this flag is not automatically cleared by the software buffer
	bool fifo_not_empty_flag; 				// this flag is automatically set and cleared by the software buffer

}sw_buffer;

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static sw_buffer buffers_array[MAX_BUFFERS];
static uint8_t buffers_cant = 0;		//cantidad de buffers inicializados


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


buff_id_t bufferInitAndGetId(void)
{
	if (buffers_cant > MAX_BUFFERS)		//si no tengo buffers disponibles
    {
        return BUFFER_INVALID_ID;
    }
    else								//si tengo buffers disponibles inicializ
    {
    	buffers_array[buffers_cant].enable=TRUE;
		buffers_array[buffers_cant].i_first=0;
		buffers_array[buffers_cant].i_last=0;
		buffers_array[buffers_cant].num_bytes=0;
		buffers_array[buffers_cant].fifo_full_flag=0;
		buffers_array[buffers_cant].fifo_not_empty_flag=0;
		buffers_array[buffers_cant].fifo_ovf_flag=0;
        buffers_cant = buffers_cant + 1;			//hay un buffer mas
    	return (buffers_cant - 1);					//devuelvo el id correspondiente
    }
}


/***************************************************************************************************************/
// UART data transmit function
//  - checks if there's room in the transmit sw buffer
//  - if there's room, it transfers data byte to sw buffer
//  - automatically handles "uart_tx_buffer_full_flag"
//  - sets the overflow flag upon software buffer overflow (doesn't overwrite existing data)
//  - if this is the first data byte in the buffer, it enables the "hw buffer empty" interrupt
void push_buffer(uint8_t byte, buff_id_t id) {

	if(buffers_array[id].num_bytes == BUFFER_SIZE) {      // no room in the sw buffer
		buffers_array[id].fifo_ovf_flag = 1;                     // set the overflow flag
	}else if(buffers_array[id].num_bytes < BUFFER_SIZE) { // if there's room in the sw buffer
		buffers_array[id].data_buf[buffers_array[id].i_last] = byte;       // transfer data byte to sw buffer
		buffers_array[id].i_last++;                              // increment the index of the most recently added element
		buffers_array[id].num_bytes++;                           // increment the bytes counter
	}
	if(buffers_array[id].num_bytes == BUFFER_SIZE) {      // if sw buffer is full
		buffers_array[id].fifo_full_flag = 1;                    // set the TX FIFO full flag
	}
	if(buffers_array[id].i_last == BUFFER_SIZE) {         // if the "new data" index has reached the end of the buffer,
		buffers_array[id].i_last = 0;                            // roll over the index counter
	}
	//actualizo el valor del flag
	buffers_array[id].fifo_not_empty_flag = 1;

  ///////////////////////
  /* enable interrupts */
  ///////////////////////


}
/***************************************************************************************************************/


/***************************************************************************************************************/
// UART data receive function
//  - checks if data exists in the receive sw buffer
//  - if data exists, it returns the oldest element contained in the buffer
//  - automatically handles "uart_rx_buffer_full_flag"
//  - if no data exists, it clears the uart_rx_flag
uint8_t pop_buffer(buff_id_t id) {

  ///////////////////////////////////////////////////////////
  /* disable interrupts while manipulating buffer pointers */
  ///////////////////////////////////////////////////////////

	uint8_t byte = 0;
	if(buffers_array[id].num_bytes == BUFFER_SIZE) { // if the sw buffer is full
		buffers_array[id].fifo_full_flag = 0;               // clear the buffer full flag because we are about to make room
	}
	if(buffers_array[id].num_bytes > 0) {                 // if data exists in the sw buffer
		byte = buffers_array[id].data_buf[buffers_array[id].i_first]; // grab the oldest element in the buffer
		buffers_array[id].i_first++;                        // increment the index of the oldest element
		buffers_array[id].num_bytes--;                      // decrement the bytes counter
		//una vez q saque del buffer me fijo si quedo vacio o no
		if(buffers_array[id].num_bytes != 0) buffers_array[id].fifo_not_empty_flag=1;
		else{ buffers_array[id].fifo_not_empty_flag=0;}
	}else{                                      // RX sw buffer is empty
		buffers_array[id].fifo_not_empty_flag = 0;          // clear the rx flag
	}
	if(buffers_array[id].i_first == BUFFER_SIZE) {   // if the index has reached the end of the buffer,
		buffers_array[id].i_first = 0;                      // roll over the index counter
	}

  ///////////////////////
  /* enable interrupts */
  ///////////////////////

	return byte;                                // return the data byte
}

bool buffer_is_full(buff_id_t id){
	return buffers_array[id].fifo_full_flag;
}

bool buffer_is_not_empty(buff_id_t id){
	return buffers_array[id].fifo_not_empty_flag;
}

bool buffer_is_overflowed(buff_id_t id){
	return buffers_array[id].fifo_ovf_flag;
}


