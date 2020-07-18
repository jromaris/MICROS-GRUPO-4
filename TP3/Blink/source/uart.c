/*
 * uart.c
 *
 *  Created on: 30 sep. 2019
 *      Author: Tomas
 */

//includes
#include "uart.h"
#include "hardware.h"
#include "MK64F12.h"
#include "board.h"
#include "buffer.h"
#include "gpio.h"

static UART_Type *  uarts_ptr[]= UART_BASE_PTRS; //{URT0,URT1,URT2,URT3,URT4,URT5} donde cada UART apunta UARTX base address
static PORT_Type * ports_p[]=PORT_BASE_PTRS;
static SIM_Type *  sim=SIM;
static buff_id_t tx_buffer_id;
static buff_id_t rx_buffer_id;
#define UART_HAL_DEFAULT_BAUDRATE (9600)

//static variables for the file
static void UART_SetBaudRate(UART_Type *uart, uint32_t baudrate);
static void delayLoop(uint32_t veces);



static void uartMode(pin_t pin){
//Nada mas funciona o esta probada para pota 14 y 15

	uint8_t port=PIN2PORT(pin);
	uint8_t num=PIN2NUM(pin);
	if(port==PA)//Hago Clock gating con el puerto deseado
	{
		sim->SCGC5 |= SIM_SCGC5_PORTA(num<<1);
	}
	else if(port==PB)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTB(num<<1);
	}
	else if(port==PC)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTC(num<<1);
	}
	else if(port==PD)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTD(num<<1);
	}
	else if(port==PE)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTE(num<<1);
	}

	ports_p[port]->PCR[num]&=~PORT_PCR_MUX(7);
	ports_p[port]->PCR[num]|=PORT_PCR_MUX(3);//Configuro el mux del pin para que apunte al UART0
	//                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ports_p[port]->PCR[num]|=PORT_PCR_IRQC(0);//DESHABILITO INTERRUPCIONES

}

void uartInit (uint8_t id, uart_cfg_t config){
	//inicializacion pines
	uartMode(PIN_UART0_TX); //pa14
	uartMode(PIN_UART0_RX); //pa15

	//gpioMode(PIN_SS,OUTPUT);
	//gpioWrite(PIN_SS,false);
	//inicializacion buffer
	//hay q chequear que esto se inicializa bien (OJO CON INVALID ID)
	tx_buffer_id = bufferInitAndGetId();
	rx_buffer_id = bufferInitAndGetId();

	//id ENTRE 0 y 5 uarts number. creo que esto no es el puerto sino el pin
	//uint8_t port=PIN2PORT(pin);
	//uint8_t num=PIN2NUM(pin);
	if (id == 0){
		sim->SCGC4 |= SIM_SCGC4_UART0_MASK;
	}else if(id == 1){
		sim->SCGC4 |= SIM_SCGC4_UART1_MASK;
	} else if(id == 2){
		sim->SCGC4 |= SIM_SCGC4_UART2_MASK;
	}else if (id == 3){
		sim->SCGC4 |= SIM_SCGC4_UART3_MASK;
	}else if (id == 4){
		sim->SCGC1 |= SIM_SCGC1_UART4_MASK;
	}else if (id == 5){
		sim->SCGC1 |= SIM_SCGC1_UART5_MASK;
	}else {
		return;
	}




	//desabilito transmiter y reciever antse de hacer cualquier tipo de cambio,
	//TEnable y REenable en 0
	uarts_ptr[id]->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );

	uarts_ptr[id]->D = 0;		//necesario para configurar FIFO
	/*habilito FIFO*/


	//enable fifo receive and transmit
	//uarts_ptr[id]->PFIFO |= (UART_PFIFO_RXFE_MASK | UART_PFIFO_TXFE_MASK);

	//se levantara una interrupcion cuando el hw buffer esta vacio
	uarts_ptr[id]->TWFIFO = 0;



	/* Configure the UART for 8-bit mode, with parity */
	/* We need all default settings, so entire register is cleared */
	uarts_ptr[id]->C1 = 0x01;



	//habilito interrupciones
	//usar el handler correspondiente
	__NVIC_EnableIRQ(UART0_RX_TX_IRQn);
	//habilito las interrupciones de transmicion completa y buffer de recibir lleno
	//solo habilito las interrupciones,
	//las transmiciones se habilitan solo cuando yo quiero enviar
	uarts_ptr[id]->C2 |= (UART_C2_RIE_MASK);

	//baudrate configuration
	UART_SetBaudRate(uarts_ptr[id],config.baudrate);
	/* Enable receiver and transmitter */
	uarts_ptr[id]->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
	delayLoop(10000000);

}

void UART_SetBaudRate(UART_Type *uart, uint32_t baudrate){

	uint16_t sbr, brfa;
	uint32_t clock;

	//configuro el clock del baudrate que tiene que ser 16 veces menor al de la kinetis o 32 veces menor si
	//viene del bus.
	//seteo el clock correspondiente
	clock = ((uart==UART0) || (uart == UART1))?(__CORE_CLOCK__ ):(__CORE_CLOCK__ >>1);

	//seteamos el baudrate entre los limites admisibles, numeros negativos no se tienen en cuenta por el tipo de dato
	baudrate = ((baudrate == 0)?(UART_HAL_DEFAULT_BAUDRATE):((baudrate > 0x1FFF)?(UART_HAL_DEFAULT_BAUDRATE):(baudrate)));

	sbr = clock / (baudrate << 4); //seteo el baudrate correspondiente con el clokc correspondiente bajo la formula, dividiendolo por 16
	brfa = (clock << 1) / baudrate - (sbr << 5);

	uart->BDH = UART_BDH_SBR(sbr>>8); //shifteo para poner la info de los primeros 8bits, siempre los primeros 3 bits van a quedar en 0 por limite de 1FFF
	uart->BDL = UART_BDL_SBR(sbr); //coloco los 8 ultimos bits de sbr en BDL
	//Tiene que ver con un ajuste fino con demoras fraccionarias para que coincida la velocicad de transmision del sistema
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);
}

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
*/



uint8_t uartWriteMsg(uint8_t id,char* msg, uint8_t cant){
	//__NVIC_DisableIRQ(UART0_RX_TX_IRQn);
	//uarts_ptr[id]->C2 &= ~(UART_C2_TIE_MASK | UART_C2_RIE_MASK);
	int i =0;
	for(i=0;i<cant;i++){
		if(!buffer_is_full(tx_buffer_id)){
			push_buffer(msg[i], tx_buffer_id);
		}
		else{
			return i;
		}

	}
	//habilito la interrupcion de enviar data
	uarts_ptr[id]->C2 |= (UART_C2_TIE_MASK);
	//__NVIC_EnableIRQ(UART0_RX_TX_IRQn);
	return i;
}


__ISR__ UART0_RX_TX_IRQHandler(void){
	//primero deshabilito el flag de interrupcion.
	//Se deberia hacer solo segun userguide en este caso

	//gpioWrite(PIN_SS,true);
	uint8_t s1 = uarts_ptr[0]->S1;
	//si el buffer de uart esta vacio y tengo elementos en el buffer

	if((s1 | UART_S1_TDRE_MASK) && (buffer_is_not_empty(tx_buffer_id))){
		uarts_ptr[0]->D = pop_buffer(tx_buffer_id); // place oldest data element in the TX hardware buffer;
		if(!buffer_is_not_empty(tx_buffer_id)){
			uarts_ptr[0]->C2 &= ~(UART_C2_TIE_MASK);
			//__NVIC_DisableIRQ(UART0_RX_TX_IRQn);
		}
	}
	//si tengo que recibir algo
	else if((s1 | UART_S1_RDRF_MASK) && (!buffer_is_full(rx_buffer_id))){
		uint8_t byte = uarts_ptr[0]->D;
		push_buffer(byte, rx_buffer_id); // place oldest data element in the TX hardware buffer;
	}
	//gpioWrite(PIN_SS,false);
}

/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
*/
bool uartIsTxMsgComplete(uint8_t id){
	return !(buffer_is_not_empty(tx_buffer_id));
}

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant){

	int i;
	for(i=0; i<cant;i++){
		if(buffer_is_not_empty(rx_buffer_id)){
			msg[i]=(char)pop_buffer(rx_buffer_id);
		}
		else{
			return i;
		}

	}
	return i;
}


/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return A new byte has being received
*/
uint8_t uartGetRxMsgLength(buff_id_t id){
	return getNumBytes(rx_buffer_id);
}

/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
*/
uint8_t uartIsRxMsg(buff_id_t id){
	return buffer_is_not_empty(rx_buffer_id);
}



static void delayLoop(uint32_t veces)
{
    while (veces--);
}
