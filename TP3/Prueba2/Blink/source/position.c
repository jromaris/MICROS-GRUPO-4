/*
 * position.c
 *
 *  Created on: 3 oct. 2019
 *      Author: marcos
 */


#include "position.h"


SRAWDATA accData;
SRAWDATA magData;

static int roll_angle=0;

static void rollAngleInc(void);
static void rollAngleDec(void);

void actRollAngle(void);


void actRollAngle(void)
{
	if((accData.x>ACC_THRESHOLD)&&((accData.z-GRAV_THRESHOLD)>ACC_THRESHOLD))rollAngleInc();
	else if((accData.x<ACC_THRESHOLD)&&((accData.z-GRAV_THRESHOLD)<ACC_THRESHOLD))rollAngleDec();

}


static void rollAngleInc(void)
{
	roll_angle++;
	if(roll_angle>181)roll_angle=-179;
}

static void rollAngleDec(void)
{
	roll_angle--;
	if(roll_angle<-180)roll_angle=180;
}

int getRollAngle(void)
{
	return roll_angle;
}


int calcRoll(SRAWDATA acc)
{
	int tita=0;
	double titaRad=0;

	if(acc.z==0)
	{
		tita=90;
	}
	else
	{
	titaRad=atan((double)acc.x/(double)acc.z);
	tita=(int)(180*titaRad/M_PI);


	if(acc.z>0)tita=-tita;
	else if(acc.x<0)tita=180-tita;
	else tita=-179-tita;



	}

	return tita;
}


int calcOr(SRAWDATA mag)
{
	int tita;
	double titaRad;

	if(mag.y==0)tita=90;
	else
	{
		titaRad=atan((double)mag.x/(double)mag.y);
		tita=(int)(180*titaRad/M_PI);
		tita=-tita;
		if((mag.x<0)&&(mag.y<0))tita=tita+180;
		else if((mag.x>0)&&(mag.y<0))tita=tita-179;
	}

	return tita;
}

int calcHead(SRAWDATA acc)
{
	int tita=0;
	double titaRad=0;

	if(acc.z==0)
	{
		tita=90;
	}
	else
	{
	titaRad=atan((double)acc.y/(double)acc.z);
	tita=(int)(180*titaRad/M_PI);


	if(acc.z>0)tita=-tita;
	else if(acc.y<0)tita=180-tita;
	else tita=-179-tita;



	}

	return tita;
}
