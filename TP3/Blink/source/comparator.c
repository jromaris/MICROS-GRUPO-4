/*
 * comparator.c
 *
 *  Created on: 22 oct. 2019
 *      Author: Manuel
 */

#include "comparator.h"
#include "MK64F12.h"
#include "board.h"
#include <stdint.h>


//Punteros a las estructuras de cada comparador
CMP_Type * cmp_ptr[]= CMP_BASE_PTRS;

//Vectores de interrupción para el periférico comparador
static IRQn_Type  irqEnable[]=CMP_IRQS;

//Puntero al periférico SIM
static SIM_Type *  sim=SIM;

//Punteros a las estructuras de cada Puerto
static PORT_Type * ports[] = PORT_BASE_PTRS;

//Arreglo de los SIM de cada puerto
static uint32_t sim_port[] = {SIM_SCGC5_PORTA_MASK, SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK, SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK};

//Función que modifica los registros necesarios para utilizar el pin deseado
static void set_pin (pin_t pin);

//
int cmp_number;

//Función que inicializa al módulo comparador
void comparator_init(int cmp_num)
{
	//Habilito los pines asociados a cada canal del módulo de FTM
	set_pin(PIN_CMP0_IN0);
	set_pin(PIN_CMP0_IN1);

	int pinPort = PIN2PORT(PIN_CMP0_OUT);
	int pinBit = PIN2NUM(PIN_CMP0_OUT);

	//Habilito el clock al puerto correspondiente
	SIM->SCGC5 |= sim_port[pinPort];

	//Configuro el pcr del pin
	ports[pinPort]->PCR[pinBit]= PORT_PCR_MUX(6) | \
			PORT_PCR_IRQC(0) | \
			PORT_PCR_PS(0)| \
			PORT_PCR_PE(0);

	//
	cmp_number=cmp_num;

	//Hago el Clock Gating apropiado
	sim->SCGC4 |= SIM_SCGC4_CMP_MASK;

	//Seteo la cantidad de puntos que utiliza el filtro
	cmp_ptr[cmp_num]->CR0=(cmp_ptr[cmp_num]->CR0 & ~CMP_CR0_FILTER_CNT_MASK) | CMP_CR0_FILTER_CNT(1);

	//Seteo el Nivel de Histeresis del comparador
	cmp_ptr[cmp_num]->CR0=(cmp_ptr[cmp_num]->CR0 & ~CMP_CR0_HYSTCTR_MASK) | CMP_CR0_HYSTCTR(3);

	//Seteo el Modo sampleo
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_SE_MASK) | CMP_CR1_SE(0);

	//Seteo el Modo de ventaneo
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_WE_MASK) | CMP_CR1_WE(0);

	//Seteo la velocidad del comparador
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_PMODE_MASK) | CMP_CR1_PMODE(0);

	//Seteo si invierto la salida del comparador
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_INV_MASK) | CMP_CR1_INV(0);

	//COS
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_COS_MASK) | CMP_CR1_COS(0);

	//Habilito la salida del pin del comparador
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_OPE_MASK) | CMP_CR1_OPE(1);

	//No habilito la interrupción del periférico por flanco ascendente
	cmp_ptr[cmp_num]->SCR=(cmp_ptr[cmp_num]->SCR & ~CMP_SCR_IER_MASK) | CMP_SCR_IER(0);

	//No habilito la interrupción del periférico por flanco descendente
	cmp_ptr[cmp_num]->SCR=(cmp_ptr[cmp_num]->SCR & ~CMP_SCR_IEF_MASK) | CMP_SCR_IEF(0);

	//Habilito las interrupciones de ese periférico
	NVIC_EnableIRQ(irqEnable[cmp_num]);

	//Elijo la entrada inversora del comparador
	cmp_ptr[cmp_num]->MUXCR=(cmp_ptr[cmp_num]->MUXCR & ~CMP_MUXCR_MSEL_MASK) | CMP_MUXCR_MSEL(7);

	//Elijo la entrada no inversora del comparador
	cmp_ptr[cmp_num]->MUXCR=(cmp_ptr[cmp_num]->MUXCR & ~CMP_MUXCR_PSEL_MASK) | CMP_MUXCR_PSEL(1);

	//Habilito el DAC del periférico
	cmp_ptr[cmp_num]->DACCR=(cmp_ptr[cmp_num]->DACCR & ~CMP_DACCR_DACEN_MASK) | CMP_DACCR_DACEN(1);

	//Elijo a que input del comparador se conecta el DAC
	cmp_ptr[cmp_num]->DACCR=(cmp_ptr[cmp_num]->DACCR & ~CMP_DACCR_VRSEL_MASK) | CMP_DACCR_VRSEL(1);

	//Fijo la tensión de referencia del DAC
	cmp_ptr[cmp_num]->DACCR=(cmp_ptr[cmp_num]->DACCR & ~CMP_DACCR_VOSEL_MASK) | CMP_DACCR_VOSEL	(35);

	//Habilito el periférico
	cmp_ptr[cmp_num]->CR1=(cmp_ptr[cmp_num]->CR1 & ~CMP_CR1_EN_MASK) | CMP_CR1_EN(1);
}

//Función a llamar cuando ocurra una interrupción del comparador 0
void CMP0_DriverIRQHandler(void)
{
	//Leo el registro que contiene los flags asociadoas a las interrupciones
	uint8_t cntrl_reg=cmp_ptr[cmp_number]->SCR;

	//Me fijo si se levantó el flag de rising edge
	uint8_t CFR_flag=cntrl_reg & CMP_SCR_CFR_MASK;

	//Me fijo si se levantó el flag de falling edge
	uint8_t CFF_flag=cntrl_reg & CMP_SCR_CFF_MASK;

	if(CFR_flag)
	{
		//Codigo en caso de ser necesario
	}

	if(CFF_flag)
	{
		//Codigo en caso de ser necesario
	}
}

//
static void set_pin (pin_t pin)
{
	int pinPort = PIN2PORT(pin);
	int pinBit = PIN2NUM(pin);

	//Habilito el clock al puerto correspondiente
	SIM->SCGC5 |= sim_port[pinPort];

	//Configuro el pcr del pin
	ports[pinPort]->PCR[pinBit]= PORT_PCR_MUX(0) | \
			PORT_PCR_IRQC(0) | \
			PORT_PCR_PS(0)| \
			PORT_PCR_PE(0);
}
