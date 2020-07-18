/*
 * Can.h
 *
 *  Created on: 2 oct. 2019
 *      Author: Manuel
 */
#include <stdbool.h>

typedef enum {ROLIDO,CABECEO,ORIENTACION}coord_t;

typedef struct coords
{
	int ID;
	coord_t coordType;
	char data[5];
	int data_len;
}coords;

#ifndef COORDINATES_H_
#define COORDINATES_H_

//
void init_coords(void);

//
bool there_isData(void);

//
bool sendCoords(coords coord2send);

//
bool receiveCoord(coords * coord2receive);

#endif /* COORDINATE_H_ */
