#include <bufferCAN.h>
#include"SPI.h"
#include <stdio.h>
#include "gpio.h"
#include "board.h"
#include "MCP25625.h"

#define DLC 7

//Función que manda el comando de reset a la placa CAN
static void MCP25625_RESET(void);

//Función que aplica el comando BIT MODIFY al address deseado con la mascara y dato deseado
static void MCP25625_BIT_MODIFY(char address, char mask, char data);

//Función que escribe la cantidad de bytes deseadas en el registro deseado y en los contiguos
static void MCP25625_WRITE(char address, char * buffer, int bufflen);

//La función de lectura debe recibir un bufflen=longitd de caracteres deseados para leer +2 (el buffer debe estar igual de preparado)
static void MCP25625_READ(char address, char * buffer, int bufflen);

//
static void MCP25625_RTS(char buff2send);

//
static bool MCP25625_isTbuffer_empty(int buff_num, char CAN_FLAG_REGISTER);

//
static bool MCP25625_isRbuffer_full(int buff_num,char CAN_FLAG_REGISTER);

//
void MCP25625_interrupt_callback(void);

//Buffer temporario de recepción
static circ_buff_t recBuff;

//
void MCP25625_interrupt_callback(void)
{
	CANmsg data2add;

	char readf[30];
	char FLAG_RGSTR;

	//Me fijo que flag del registro esta prendido
	MCP25625_READ(CANINTF,readf,3);
	FLAG_RGSTR=readf[2];

	//Si el buffer de recepcion esta lleno, lo leo
	//if (MCP25625_isRbuffer_full(0,FLAG_RGSTR))
	{
		MCP25625_READ(RXBXCTRL(0),readf, 16);

		//Leo el ID del mensaje recibido
		data2add.id=(readf[3]<<3)+(readf[4]>>5);

		//Leo la cantidad de Bytes de datos recibido
		data2add.size=readf[DLC];

		//Guardo los bytes de datos recibidos
		for(int i=DLC+1;i<DLC+1+data2add.size;i++)
		{
			data2add.data[i-(DLC+1)]=readf[i];
		}

		//Agrego el mensaje de CAN al buffer
		push_bufferCAN(&recBuff, data2add);

		char temp=0;
		MCP25625_WRITE(CANINTF,&temp,1);
		//Reseteo el Flag
		//MCP25625_BIT_MODIFY(CANINTF,RX0IF_MASK,RX0IF_DATA(0));
	}

	//Si el buffer de transmisión esta vacío mando algo
}

//
void MCP25625_init(int mask, int filter)
{

	char auxbuffer[3];
	//Inicializo la estructura que utilizo como buffer de recepción
	init_bufferCAN(&recBuff,BUFFER_SIZE);

	//Inicializo el driver de SPI
	SPI_init();

	//Habilito el Pin que se va a utilizar para recibir las interrupciones que tira el controlador
	gpioMode(IRQ_CAN, INPUT_PULLUP);
	gpioIRQ(IRQ_CAN, GPIO_IRQ_MODE_FALLING_EDGE, MCP25625_interrupt_callback);

	//Mando un comando de Reset
	MCP25625_RESET();

	//Pongo el controlador en modo configuración
	MCP25625_BIT_MODIFY(CANCTRL,REQOP_MASK,REQOP_DATA(CONFIGURATION));

	//Seteo la longitud de un time quanta
	MCP25625_BIT_MODIFY(CNF1,SJW_MASK|BRP_MASK,SJW_DATA(SJW-1) | BRP_DATA(BAUDRATE_PRESCALER));

	//Seteo la longitud del PhaseSegment1 y del PropagationSegment
	MCP25625_BIT_MODIFY(CNF2,BTLMODE_MASK|SAM_MASK|PHSEG1_MASK|PRSEG_MASK,BTLMODE_DATA(BTLMODE)|SAM_DATA(SAM)|PHSEG1_DATA(PHSEG1-1)|PRSEG_DATA(PRSEG-1));

	//Seteo la longitud del PhaseSegment2 y prendo el Wake-Up filter
	MCP25625_BIT_MODIFY(CNF3,WAKFIL_MASK| PHSEG2_MASK,WAKFIL_DATA(WAKFIL)| PHSEG2_DATA(PHSEG2-1));

	MCP25625_BIT_MODIFY(TXRTSCTRL,B2RTSM_MASK|B1RTSM_MASK|B0RTSM_MASK,B2RTSM_DATA(0)|B1RTSM_DATA(0)|B0RTSM_DATA(1));

	//Cargo el filtro de IDS a utilizar
	auxbuffer[0]=(char)(0xFF &(filter>>3));
	auxbuffer[1]=(char)(0xFF &(filter<<5));
	MCP25625_WRITE(RXF0SIDH,auxbuffer,2);

	//Cargo la mascara a utilizar para luego aplicar el filtro de IDS
	auxbuffer[0]=(char)(0xFF & (mask>>3));
	auxbuffer[1]=(char)((mask<<5)& 0xFF);
	MCP25625_WRITE(RXMXSIDH(0),auxbuffer,2);

	//Cargo en el Buffer de Rececpción a utilizar las máscaras y filtros, además deshabilito el rollover entre buffers
	MCP25625_BIT_MODIFY(RXBXCTRL(0),RECEIVE_MODE_MASK|BUKT_MASK,RECEIVE_MODE_DATA(0)|BUKT_DATA(0));

	//Le doy la máxima prioridad de transmisión al buffer 0
	MCP25625_BIT_MODIFY(TXBXCTRL(0),T_PRIORITY_MASK,T_PRIORITY_DATA(3));

	//Pongo el controlador en el modo normal de operación
	MCP25625_BIT_MODIFY(CANCTRL,REQOP_MASK|OSM_MASK,REQOP_DATA(NORMAL)|OSM_DATA(0));//pongo en modo normal, habilito el clock y prescaler de 1.

	//Habilito la interrupción por buffer de recepción 0 lleno de un mensaje válido.
	MCP25625_BIT_MODIFY(CANINTE,RX0IE_MASK|TX0IE_MASK,RX0IE_DATA(1)|TX0IE_DATA(0));
}

//Esta función recibe el ID a mandar
bool MCP25625_send(int ID,char * databuffer, int bufflen, int whichbuffer)//por ahora lo hago solo con el buffer 0
{
	//Longitud máxima de mensaje a transmitir es de 8 bytes, si se recibe algo más grande se lo recorta
	if(bufflen>8)
		bufflen=8;

	bool retVal=false;
	//char TXREQ;
	char bufferaux[3];

	//Me fijo si el buffer deseado no tiene una transmisión pendiente
	//MCP25625_READ(TXBXCTRL(whichbuffer),bufferaux,3);
	//TXREQ=(bufferaux[2]>>3) & 1;

	//Si no hay transmisión pendiente
	//if(TXREQ==0)
	{
		//Cargo el ID recibido al buffer
		bufferaux[0]=(char)(0xFF &(ID>>3));
		bufferaux[1]=(char)((ID&0x07)<<5);
		MCP25625_WRITE(TXBXSIDH(whichbuffer), bufferaux,2);

		//Cargo la cantidad de datos a mandar al buffer
		bufferaux[0]=bufflen;
		MCP25625_WRITE(TXBXDLC(whichbuffer), bufferaux,1);//seteo dataframe y data len

		//Cargo los datos deseados para mandar al buffer
		MCP25625_WRITE(TXBXD0(whichbuffer), databuffer,bufflen);

		//Dependiendo que buffer estoy utilizando,habilito su transmisión
		if(whichbuffer==1)
			MCP25625_RTS(2);
		else if(whichbuffer==2)
			MCP25625_RTS(4);
		else
			MCP25625_RTS(1);
		retVal=true;
	}
	return retVal;
}

//
bool MCP25625_isTbuffer_empty(int buff_num, char CAN_FLAG_REGISTER)
{
	bool retVal=false;
	if(buff_num==0)
	{
		if(CAN_FLAG_REGISTER&(char)TX0IF_MASK)
			retVal=true;
	}
	else if(buff_num==1)
	{
		if(CAN_FLAG_REGISTER&(char)TX1IF_MASK)
			retVal=true;

	}
	else if(buff_num==2)
	{
		if(CAN_FLAG_REGISTER&(char)TX2IF_MASK)
			retVal=true;

	}
	return retVal;
}

//
bool MCP25625_isRbuffer_full(int buff_num,char CAN_FLAG_REGISTER)
{
	bool retVal=false;
	if(buff_num==0)
	{
		if(CAN_FLAG_REGISTER&(char)RX0IF_MASK)
			retVal=true;
	}
	else if(buff_num==1)
	{
		if(CAN_FLAG_REGISTER&(char)RX1IF_MASK)
			retVal=true;
	}

	return retVal;
}


//
bool receiveFromCAN(int * ID,char * data, int * data_len)
{
	CANmsg data2pop;
	if(buffer_is_emptyCAN(&recBuff)==0)
	{
		data2pop= pop_bufferCAN(&recBuff);
		ID[0]=data2pop.id;
		data_len[0]=data2pop.size;
		for(int i=0;i<data2pop.size;i++)
		{
			data[i]=data2pop.data[i];
		}
		return true;
	}
	else
		return false;

}

//FUNCIONES ESTÁTICAS UTILIZADAS SOLO EN ESTE .c

//
void MCP25625_RESET(void)
{
	char frame2send = RESET;
	SPI_sendReceive(&frame2send, 1,NULL);
}

//
void MCP25625_BIT_MODIFY(char address, char mask, char data)
{
	char buffer[4];
	buffer[0]=BIT_MODIFY;
	buffer[1]=address;
	buffer[2]=mask;
	buffer[3]=data;
	SPI_sendReceive(buffer,4,NULL);
}

//
void MCP25625_WRITE(char address, char * buffer, int bufflen)
{
	char send_buff[MAX_BUFF_LEN];
	send_buff[0]=WRITE;
	send_buff[1]=address;

	for(int i=0;(i<bufflen)&&(i<MAX_BUFF_LEN);i++)
	{
		send_buff[2+i]=buffer[i];
	}
	SPI_sendReceive(send_buff,bufflen+2,NULL);
}

//La función de lectura debe recibir un bufflen=longitd de caracteres deseados para leer +2 (el buffer debe estar igual de preparado)
void MCP25625_READ(char address, char * buffer, int bufflen)
{
	char send_buff[MAX_BUFF_LEN];
	send_buff[0]=READ;
	send_buff[1]=address;

	for(int i=0;(i<bufflen-2)&&(i<MAX_BUFF_LEN);i++)
	{
		send_buff[2+i]=NO_COMMAND;
	}
	SPI_sendReceive(send_buff,bufflen,buffer);
}

//
void MCP25625_RTS(char buff2send)
{
	char frame2send=0;
	frame2send=RTS+buff2send;
	SPI_sendReceive(& frame2send,1,NULL);
}
