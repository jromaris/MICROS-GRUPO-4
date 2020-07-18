#include "gpio.h"
#include "MK64F12.h"

static GPIO_Type *  gpios_ptr[]= GPIO_BASE_PTRS;
static PORT_Type * ports_p[]=PORT_BASE_PTRS;
static SIM_Type *  sim=SIM;

static pinIrqFun_t	irq_pointers[5][32];

void portIRQ_handler(uint8_t port);
/**
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 */


void gpioMode (pin_t pin, uint8_t mode)
{
	uint8_t port=PIN2PORT(pin);
	uint8_t num=PIN2NUM(pin);
	if(port==PA)//Hago Clock gating con el puerto deseado
	{
		sim->SCGC5 |= SIM_SCGC5_PORTA(num<<1);
	}
	else if(port==PB)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTB(num<<1);
	}
	else if(port==PC)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTC(num<<1);
	}
	else if(port==PD)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTD(num<<1);
	}
	else if(port==PE)
	{
		sim->SCGC5 |= SIM_SCGC5_PORTE(num<<1);
	}

	ports_p[port]->PCR[num]|=PORT_PCR_MUX(1);//Configuro el mux del pin para que apunte al GPIO
	ports_p[port]->PCR[num]|=PORT_PCR_IRQC(0);//DESHABILITO INTERRUPCIONES

	if(mode ==INPUT || mode ==INPUT_PULLUP || mode==INPUT_PULLDOWN)//Si es algun tipo de input mode
	{
		gpios_ptr[port]->PDDR&=!GPIO_PDDR_PDD(1<<num);//pongo el gpio en modo input
		if(mode==INPUT_PULLUP)
		{
			ports_p[port]->PCR[num]|=PORT_PCR_PE(1);//permito que haya pulldown/up
			ports_p[port]->PCR[num]|=PORT_PCR_PS(1);//pongo pullup

		}
		else if(mode==INPUT_PULLDOWN)
		{
			ports_p[port]->PCR[num]|=PORT_PCR_PE(1);//permito que haya pulldown/up
			ports_p[port]->PCR[num]|=PORT_PCR_PS(0);//pongo pulldown
		}
		else

		{
			ports_p[port]->PCR[num]|=PORT_PCR_PE(0);////permito que haya pulldown/up
			ports_p[port]->PCR[num]|=PORT_PCR_PS(0);//
		}

	}
	else if(mode == OUTPUT)
	{
		gpios_ptr[port]->PDDR|=GPIO_PDDR_PDD(1<<num);//pongo el gpio en modo output
		gpios_ptr[port]->PTOR|=GPIO_PTOR_PTTO(1<<num);
	}
	else
	{

	}

}

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 */
void gpioWrite (pin_t pin, bool value)
{
	uint8_t port=PIN2PORT(pin);
	uint8_t num=PIN2NUM(pin);
	if(value) gpios_ptr[port]->PSOR|=GPIO_PSOR_PTSO(1<<num);
	else gpios_ptr[port]->PCOR|=GPIO_PCOR_PTCO(1<<num);
}

/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 */
void gpioToggle (pin_t pin)
{
	uint8_t port=PIN2PORT(pin);
	uint8_t num=PIN2NUM(pin);
	gpios_ptr[port]->PTOR|=GPIO_PTOR_PTTO(1<<num);
}

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
bool gpioRead (pin_t pin)
{
	uint8_t port=PIN2PORT(pin);
	uint8_t num=PIN2NUM(pin);
	return gpios_ptr[port]->PDIR & GPIO_PDIR_PDI(1<<num);
}

/**
 * @brief Configures how the pin reacts when an IRQ event ocurrs
 * @param pin the pin whose IRQ mode you wish to set (according PORTNUM2PIN)
 * @param irqMode disable, risingEdge, fallingEdge or bothEdges
 * @param irqFun function to call on pin event
 * @return Registration succeed
 */
bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
{
	uint8_t port=PIN2PORT(pin);
	uint8_t num=PIN2NUM(pin);

	//tener en cuenta q hay q inicilizar el pcr en 0 para q funcione la linea de abajo
	ports_p[port]->PCR[num]|=PORT_PCR_MUX(1);//Configuro el mux del pin para que apunte al GPIO
	ports_p[port]->PCR[num]|=PORT_PCR_IRQC(irqMode);//Habilito el tipo de interrupcion deseada.

	irq_pointers[port][num]=irqFun;
	//ya hay un enum en mk64f12
	if(port==PA)//Habilitando las interrupciones para el puerto deseado.
	{
		__NVIC_EnableIRQ(PORTA_IRQn);
	}
	else if(port==PB)
	{
		__NVIC_EnableIRQ(PORTB_IRQn);
	}
	else if(port==PC)
	{
		__NVIC_EnableIRQ(PORTC_IRQn);
	}
	else if(port==PD)
	{
		__NVIC_EnableIRQ(PORTD_IRQn);
	}
	else if(port==PE)
	{
		__NVIC_EnableIRQ(PORTE_IRQn);
	}


	return 1; //FALTA HACER VERIFICACION

}

__ISR__ PORTA_IRQHandler (void)
{
	portIRQ_handler(PA);
}
__ISR__ PORTB_IRQHandler (void)
{
	portIRQ_handler(PB);
}
__ISR__ PORTC_IRQHandler (void)
{
	portIRQ_handler(PC);
}

void portIRQ_handler(uint8_t port)
{
	uint8_t pin=0;

	while(((ports_p[port]->PCR[pin])&PORT_PCR_ISF_MASK)!=PORT_PCR_ISF_MASK)pin++;



	irq_pointers[port][pin]();
	ports_p[port]->PCR[pin]|=PORT_PCR_ISF_MASK;


}


