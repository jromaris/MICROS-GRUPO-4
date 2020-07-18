/*
 * MCP25625.h
 *
 *  Created on: 2 oct. 2019
 *      Author: Manuel
 */

#ifndef MCP25625_H_
#define MCP25625_H_
#include <stdbool.h>

//DEFINE DE LOS PARÁMETROS QUE VOY A USAR
#define BAUDRATE_PRESCALER 7
#define SJW 1
#define BTLMODE 1
#define SAM 0
#define WAKFIL 1
#define PHSEG1 4
#define PHSEG2 2
#define PRSEG 1

typedef enum{NORMAL,SLEEP,LOOPBACK,LISTEN_ONLY,CONFIGURATION} operation_mode_t;

#define MAX_BUFF_LEN 30

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//MCP25625 SPI COMMANDS
#define NO_COMMAND 0x00
#define RESET 0xC0
#define READ 0x03
#define READ_RX_BUFFER 0x90
#define WRITE 0x02
#define LOAD_RX_BUFFER 0x40
#define RTS 0x80
#define READ_STATUS 0xA0
#define RX_STATUS 0xB0
#define BIT_MODIFY 0x05

//MCP25625 Registers
//Configuration Registers
#define CNF1 0x2A
#define CNF2 0x29
#define CNF3 0x28

//Standard Mask Identifier Registers
#define RXMXSIDH(x) 0x20+x*0x04
#define RXMXSIDL(x) 0x21+x*0x04

//Standard Filter Identifier Registers
#define RXF0SIDH 0x00
#define RXF0SIDL 0x01

#define TXRTSCTRL 0x0D

//Can Control Register
#define CANCTRL 0x0F

//Transmit Buffer Registers
#define TXBXCTRL(x) 0x30+x*0x10
#define TXBXSIDH(x) 0x31+x*0x10
#define TXBXDLC(x) 0x35+x*0x10
#define TXBXD0(x) 0x36+x*0x10

//Receive Buffer Registers
#define RXBXCTRL(x) 0x60+x*0x10
#define RXBXSIDH(x) 0x61+x*0x10
#define RXBXDLC(x) 0x65+x*0x10
#define RXBXD0(x) 0x66+x*0x10

//Interrupt Registers
#define CANINTE 0x2B
#define CANINTF 0x2C
/*********************************/
//MASCARAS POR REGISTRO
/*********************************/
//CNF1
#define SJW_MASK 0xC0
#define SJW_DATA(x) (char)((char)(x)<<6)
#define	BRP_MASK 0x3F
#define BRP_DATA(x) (char)((char)x<<0)

//CNF2
#define BTLMODE_MASK 0x80
#define BTLMODE_DATA(x) (char)((char)x<<7)
#define SAM_MASK 0x40
#define SAM_DATA(x) (char)((char)x<<6)
#define PHSEG1_MASK 0x38
#define PHSEG1_DATA(x) (char)((char)((x)<<3))
#define PRSEG_MASK 0x07
#define PRSEG_DATA(x) (char)((char)((x)<<0))

//CNF3
#define WAKFIL_MASK 0x40
#define WAKFIL_DATA(x) (char)((char)x<<6)
#define PHSEG2_MASK 0x07
#define PHSEG2_DATA(x) (char)((char)(x)<<0)

//CANCTRL
#define REQOP_MASK 0xE0
#define REQOP_DATA(x) (char)((char)(x)<<5)
#define ABAT_MASK 0x10
#define ABAT_DATA(x) (char)((char)x<<4)
#define OSM_MASK 0x08
#define OSM_DATA(x) (char)((char)x<<3)
#define CLKEN_MASK 0x04
#define CLKEN_DATA(x) (char)((char)x<<2)
#define CLKPRE_MASK 0x03
#define CLKPRE_DATA(x) (char)((char)x<<0)

//TXBXCTRL
#define TXREQ_MASK 0x08
#define TXREQ_DATA(x) (char)((char)x<<3)
#define T_PRIORITY_MASK 0x03
#define T_PRIORITY_DATA(x) (char)((char)x<<0)

//RXB0CTRL
#define RECEIVE_MODE_MASK 0x60
#define RECEIVE_MODE_DATA(x) (char)((char)x<<5)
#define BUKT_MASK 0x40
#define BUKT_DATA(x) (char)((char)x<<2)

//TXRTSCTRL
#define B2RTSM_MASK 0x04
#define B2RTSM_DATA(x) (char)((char)x<<2)
#define B1RTSM_MASK 0x02
#define B1RTSM_DATA(x) (char)((char)x<<1)
#define B0RTSM_MASK 0x01
#define B0RTSM_DATA(x) (char)((char)x<<0)

//CANINTE
#define MERRE_MASK 0x80
#define MERRE_DATA(x) (char)((char)x<<7)
#define WAKIE_MASK 0x40
#define WAKIE_DATA(x) (char)((char)x<<6)
#define ERRIE_MASK 0x20
#define ERRIE_DATA(x) (char)((char)x<<5)
#define TX2IE_MASK 0x10
#define TX2IE_DATA(x) (char)((char)x<<4)
#define TX1IE_MASK 0x08
#define TX1IE_DATA(x) (char)((char)x<<3)
#define TX0IE_MASK 0x04
#define TX0IE_DATA(x) (char)((char)x<<2)
#define RX1IE_MASK 0x02
#define RX1IE_DATA(x) (char)((char)x<<1)
#define RX0IE_MASK 0x01
#define RX0IE_DATA(x) (char)((char)x<<0)

//CANINTF
#define MERRF_MASK 0x80
#define MERRF_DATA(x) (char)((char)x<<7)
#define WAKIF_MASK 0x40
#define WAKIF_DATA(x) (char)((char)x<<6)
#define ERRIF_MASK 0x20
#define ERRIF_DATA(x) (char)((char)x<<5)
#define TX2IF_MASK 0x10
#define TX2IF_DATA(x) (char)((char)x<<4)
#define TX1IF_MASK 0x08
#define TX1IF_DATA(x) (char)((char)x<<3)
#define TX0IF_MASK 0x04
#define TX0IF_DATA(x) (char)((char)x<<2)
#define RX1IF_MASK 0x02
#define RX1IF_DATA(x) (char)((char)x<<1)
#define RX0IF_MASK 0x01
#define RX0IF_DATA(x) (char)((char)x<<0)

//FUNCIONES//

//Función en la que se inicializa el controlador, recibe la máscara y el filtro a utilizar para filtrar IDs.
void MCP25625_init(int mask, int filter);

//Función que manda
bool MCP25625_send(int ID,char * databuffer, int bufflen, int whichbuffer);

//
bool receiveFromCAN(int * ID, char * data, int * data_len);
#endif /* MCP25625_H_ */
