#include <stdio.h>
#include "Coordinates.h"
#include "MCP25625.h"

//
void init_coords(void)
{
	int mask=0x7F8;
	int filter=0x100;
	MCP25625_init(mask,filter);
}

//
bool sendCoords(coords coord2send)
{
	char temp[8];
	if(coord2send.coordType==ROLIDO)
		temp[0]='R';
	else if(coord2send.coordType==CABECEO)
		temp[0]='C';
	else if(coord2send.coordType==ORIENTACION)
		temp[0]='O';
	for(int i=0;i<coord2send.data_len;i++)
	{
		temp[1+i]=coord2send.data[i];
	}
	MCP25625_send(coord2send.ID,temp, coord2send.data_len+1,0);
	return true;
}

//
bool receiveCoord(coords * coord2receive)
{
	int tempID;
	int tempLen;
	char tempData[8];
	bool hay_algo;
	hay_algo= receiveFromCAN(&tempID,tempData,&tempLen);
	if(hay_algo)
	{
		coord2receive->ID=tempID;
		coord2receive->data_len=tempLen;
		if(tempData[0]=='C')
			coord2receive->coordType=CABECEO;
		else if(tempData[0]=='R')
			coord2receive->coordType=ROLIDO;
		else if(tempData[0]=='O')
			coord2receive->coordType=ORIENTACION;
		int sign;
		int dataLen;
		for(int j=0;j<4;j++)
			coord2receive->data[j]='0';
		if(tempData[1]=='-')
		{
			coord2receive->data[0]='-';
			sign=1;
		}
		else if(tempData[1]=='+')
		{
			coord2receive->data[0]='+';
			sign=1;
		}
		else
			sign=0;
		if(sign==0)
		{
			dataLen=tempLen-1;
			for(int i=0;i<dataLen;i++)
			{
				coord2receive->data[4-dataLen+i]=tempData[1+i];
			}

		}
		else if(sign==1)
		{
			dataLen=tempLen-2;
			for(int i=0;i<dataLen;i++)
			{
				coord2receive->data[4-dataLen+i]=tempData[2+i];
			}
		}

		return true;
	}
	else
		return false;

}
