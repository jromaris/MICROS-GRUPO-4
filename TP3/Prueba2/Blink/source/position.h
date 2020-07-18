/*
 * position.h
 *
 *  Created on: 3 oct. 2019
 *      Author: marcos
 */

#ifndef POSITION_H_
#define POSITION_H_

#include "FXOS8700CQ.h"
#include <math.h>

#define ACC_THRESHOLD	200
#define GRAV_THRESHOLD 2000

int getRollAngle(void);
int getHeadAngle(void);

int calcRoll(SRAWDATA acc);
int calcOr(SRAWDATA mag);
int calcHead(SRAWDATA acc);

#endif /* POSITION_H_ */
