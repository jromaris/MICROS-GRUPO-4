/*
 * FSK.h
 *
 *  Created on: 22 oct. 2019
 *      Author: Usuario
 */

#ifndef FSK_H_
#define FSK_H_
#define SIGNAL_BUFFER_LENGTH 29
//34
#include "ftm.h"
#include "comparator.h"

//global Variables

//FUNCIONES

//
void FSK_init();

//
void escribirSinDDs(void);

//
void SendCharViaFSK(char a);

//
int sendData();

//La función va levantando bits hasta que puede mandar un char por UART. Cuando ocurre esto devuelve un true, de lo contrario devuelve un falso. Cuando devuelve true se puede leer la dirección del char que se pasa.
bool creat_uart_char(char* msg);

#endif /* FSK_H_ */
