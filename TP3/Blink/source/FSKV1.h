/*
 * FSK.h
 *
 *  Created on: 22 oct. 2019
 *      Author: Usuario
 */

#ifndef FSK_H_
#define FSK_H_
#define SIGNAL_BUFFER_LENGTH 255

//global Variables

//FUNCIONES
void FSK_initV1();

int sendDataV1(); //retorna si puedo escribir, es decir, si estoy en estado IDLE

void SendCharViaFSKV1(char a); //manda char via fsk con start stop y parity

#endif /* FSK_H_ */
