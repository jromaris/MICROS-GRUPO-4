#include "board.h"
#include "MK64F12.h"
#include <stdio.h>

#define SPI_DRIVER_BR 5
#define SPI_DRIVER_PBR 1

typedef uint8_t pin_t;

static SPI_Type *SPI_ptr[] = {SPI0, SPI1, SPI2};
static void set_pin (pin_t pin);

static PORT_Type * ports[] = PORT_BASE_PTRS;
static uint32_t sim_port[] = {SIM_SCGC5_PORTA_MASK, SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK, SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK};

//
void SPI_init (void)
{
	//Configuro los pines que voy a utilizar
	set_pin(PIN_MOSI);
	set_pin(PIN_SS);
	set_pin(PIN_MISO);
	set_pin(PIN_SCK);

	//Habilito el clock gating del periférico SPI a utilizar
	SIM->SCGC6 |= SIM_SCGC6_SPI0(1);

	//Pongo el Halt en 1(no estoy transmitiendo) y me pongo en modo master
	SPI_ptr[0]->MCR=SPI_MCR_HALT_MASK|SPI_MCR_MSTR_MASK;

	//Hago las configuraciones en el CTAR(polaridad y fase del clock, frecuencia, framesize)
	SPI_ptr[0]->CTAR[0] = SPI_CTAR_CPOL(0) |  \
								 SPI_CTAR_CPHA(0)| \
								 SPI_CTAR_PBR(SPI_DRIVER_PBR) | \
								 SPI_CTAR_BR(SPI_DRIVER_BR) | \
								 SPI_CTAR_FMSZ(7)| \
								 SPI_CTAR_ASC(SPI_DRIVER_BR-3) | \
								 SPI_CTAR_PASC(SPI_DRIVER_PBR) | \
								 SPI_CTAR_CSSCK(SPI_DRIVER_BR-3) | \
								 SPI_CTAR_PCSSCK(SPI_DRIVER_PBR) |
								 SPI_CTAR_PDT(0)|\
								 SPI_CTAR_DT(0);

	//Elijo utilizar el primer Chip Select
	SPI_ptr[0]->MCR=(SPI_ptr[0]->MCR & (~SPI_MCR_PCSIS_MASK))|SPI_MCR_PCSIS(1);

	//Pongo en modo master y deshabilito las interrupciones del periférico
	SPI_ptr[0]->MCR =(SPI_ptr[0]->MCR & (~(SPI_MCR_MDIS_MASK | SPI_MCR_HALT_MASK | SPI_MCR_MSTR_MASK))) \
														| SPI_MCR_MDIS(0) |SPI_MCR_HALT(0) | SPI_MCR_MSTR(1);
}

//
void set_pin (pin_t pin){
	int pinPort = PIN2PORT(pin);
	int pinBit = PIN2NUM(pin);
	//Habilito el clock Gating del puerto correspondiente
	SIM->SCGC5 |= sim_port[pinPort];

	//Hago la configuración deseada en el PCR del Pin
	ports[pinPort]->PCR[pinBit]= PORT_PCR_MUX(2) | \
			PORT_PCR_IRQC(0) | \
			PORT_PCR_PS(0)| \
			PORT_PCR_PE(0);
}

//
char SPI_sendReceive(char * data2send, char data_len,char * recievedData)
{
	//Para el primer mensaje pongo Cont en 1 para que no se levante SS
	static uint32_t firstpush=SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1);

	//Para el último mensaje pongo Cont en 0 para que se levante SS
	static uint32_t pushrFinal=SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_EOQ(1);

	uint32_t push_a_mandar=0;
	unsigned int data_recibida=0;

	//Para la cantidad de datos que deseo que se manden en la transmisión
	for(int j=0;j<data_len;j++){
		//Si es el último dato que mando pongo el Cont en 0
		if((j+1)==data_len)
		{
			push_a_mandar=pushrFinal|SPI_PUSHR_TXDATA(data2send[j]);
		}
		//Si no es el último que mando pongo el Cont en 1
		else
		{
			push_a_mandar=firstpush|SPI_PUSHR_TXDATA(data2send[j]);
		}
		//Paro la transmisión
		SPI_ptr[0]->MCR =(SPI_ptr[0]->MCR & (~SPI_MCR_HALT_MASK))| SPI_MCR_HALT(1);

		 //Reinicio el TCF y agrego en la FIFO de salida el dato que deseo mandar
		SPI_ptr[0]->SR=(SPI_ptr[0]->SR & (~SPI_SR_TCF_MASK) ) | SPI_SR_TCF(1) ;
		SPI_ptr[0]->PUSHR=push_a_mandar;

		//Arranco la transmisión
		SPI_ptr[0]->MCR =(SPI_ptr[0]->MCR & (~SPI_MCR_HALT_MASK))| SPI_MCR_HALT(0);

		//Espero a que se mande el dato
		while(!(SPI_ptr[0]->SR & SPI_SR_TCF_MASK));

		SPI_ptr[0]->SR= (SPI_ptr[0]->SR & ~SPI_SR_TCF_MASK)| SPI_SR_TCF_MASK;

		//Si deseo recibir datos del MISO levanto la FIFO
		if(recievedData!=NULL)
		{
			if(SPI_ptr[0]->SR & SPI_SR_RXCTR_MASK)
			{
				recievedData[data_recibida]=SPI_ptr[0]->POPR;
				data_recibida++;
			}
		}
		//Si no deseo recibir igual popeo lo que haya en la FIFO
		else
		{
			SPI_ptr[0]->POPR;
		}
	}
	return data_recibida;
}
