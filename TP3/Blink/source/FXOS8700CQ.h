/*
 * FXOS8700CQ.h
 *
 *  Created on: 28 sep. 2019
 *      Author: marcos
 *
 *       la mayoria de las cosas se sacaron la datasheet del FXOS8700CQ
 */

#ifndef FXOS8700CQ_H_
#define FXOS8700CQ_H_

#include "i2c.h"

#define FXOS8700CQ_STATUS         0x00
#define FXOS8700CQ_WHOAMI         0x0D
#define FXOS8700CQ_XYZ_DATA_CFG   0x0E
#define FXOS8700CQ_CTRL_REG1      0x2A
#define FXOS8700CQ_M_CTRL_REG1    0x5B
#define FXOS8700CQ_M_CTRL_REG2    0x5C
#define FXOS8700CQ_WHOAMI_VAL     0xC7

#define FXOS8700CQ_SLAVE_ADDR 0x1D

#define FXOS8700CQ_READ_LEN	13

typedef struct
{

int16_t x;
int16_t y;
int16_t z;

} SRAWDATA;

void FXOS8700CQ_init(void);

void ReadAccelMagnData(SRAWDATA *pAccelData, SRAWDATA *pMagnData);

bool isDataReady(void);



#endif /* FXOS8700CQ_H_ */
