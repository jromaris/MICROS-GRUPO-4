#include "i2c.h"
#include "gpio.h"

static I2C_Type * i2c_ptr[]=I2C_BASE_PTRS;


#define ACC_ADRESS 0x1D

typedef enum {
	IIC0,
	IIC1,
	IIC2
}i2c_channel_t;

typedef enum
{
	RX_START,
	ADRESS,
	REGISTER,
	RX_ADRESS,
	DATA,
	DUMMY,


}rx_state_t;


typedef struct
{
	uint8_t*	dataArray;
	uint8_t dataLength;
	uint8_t	currentData;
	uint8_t startCount;
	uint8_t byteNumber;

	uint8_t slaveAdress;
	uint8_t regAdress;

	rx_state_t rx_state;

	i2cIrqFun_t callback;

	bool end;
	bool tx;

	bool i2c_bus_busy;



}i2c_data_t;

//PRIVATE VARIABLES

static i2c_data_t i2c_data;

//PRIVATE FUNCTION
void init_i2c0(void);
void i2c0_repeat_start(void);
void i2c0_start_signal(void);
void i2c0_stop_signal(void);
void i2c0_set_TXAK(void);
void i2c0_set_TX(void);
void i2c0_set_RX(void);
void i2c0_switch_to_rx(void);
bool i2c0_check_RXAK(void);
void i2c0_clean_TXAK(void);

void i2c0_write_data(uint8_t data);

uint8_t set_slave_adress_byte(uint8_t slaveAdress,bool tx);







void init_i2c0(void)
{
	//voy a usar el i2c0 para todoooo, pero despues habria que poder elegir cual usar
	static SIM_Type *  sim=SIM;
	sim->SCGC4|= SIM_SCGC4_I2C0_MASK; //enable clock gating
	sim->SCGC5|=SIM_SCGC5_PORTE_MASK;

	//i2c_ptr[0]->A1|=I2C_A1_AD(ACC_ADRESS);
	PORT_Type * ports_p[]=PORT_BASE_PTRS;

	ports_p[PE]->PCR[24]|=PORT_PCR_MUX(5);
	ports_p[PE]->PCR[25]|=PORT_PCR_MUX(5);

	ports_p[PE]->PCR[24]|=PORT_PCR_ODE_MASK;
	ports_p[PE]->PCR[25]|=PORT_PCR_ODE_MASK;

	ports_p[PE]->PCR[24]|=PORT_PCR_PE_MASK;
	ports_p[PE]->PCR[25]|=PORT_PCR_PE_MASK;

	ports_p[PE]->PCR[24]|=PORT_PCR_PS(1);
	ports_p[PE]->PCR[25]|=PORT_PCR_PS(1);

	uint32_t i2c_irqs[] = I2C_IRQS;

	i2c_ptr[0]->F|=I2C_F_ICR(0x20); //0x20
	i2c_ptr[0]->F|=I2C_F_MULT(0x02); //0x02


	NVIC_EnableIRQ(i2c_irqs[0]);

	i2c_ptr[0]->C1|=I2C_C1_IICEN_MASK;
	i2c_ptr[0]->C1|=I2C_C1_IICIE_MASK;

	i2c_ptr[0]->FLT|=I2C_FLT_SSIE_MASK;

	//i2c_ptr[0]->C1|=I2C_C1_TX_MASK;

	//i2c_ptr[0]->C1|=I2C_C1_MST_MASK;

	//uint8_t AdressData = (ACC_ADRESS << 1) & 0b11111110;

	//i2c_ptr[0]->D=AdressData;
}

void i2c0_repeat_start(void)
{
	i2c_ptr[0]->C1|=I2C_C1_RSTA_MASK;
}

void i2c0_start_signal(void)
{
	i2c_data.i2c_bus_busy=true;
	i2c_ptr[0]->C1|=I2C_C1_MST_MASK;
}

void i2c0_stop_signal(void)
{
	i2c_data.i2c_bus_busy=0;
	i2c_ptr[0]->C1&=~I2C_C1_MST_MASK;
}

void i2c0_set_TXAK(void)
{
	i2c_ptr[0]->C1|=I2C_C1_TXAK_MASK;
}

void i2c0_clean_TXAK(void)
{
	i2c_ptr[0]->C1&=~I2C_C1_TXAK_MASK;
}

void i2c0_set_TX(void)
{
	i2c_ptr[0]->C1|=I2C_C1_TX_MASK;
}

void i2c0_set_RX(void)
{
	i2c_ptr[0]->C1&=~I2C_C1_TX_MASK;
}



void I2C0_IRQHandler()
{
	I2C_Type* i2c =i2c_ptr[0];
	//CASO STOP
	if(i2c->FLT&I2C_FLT_STOPF_MASK)
	{
		i2c->FLT|=I2C_FLT_STOPF_MASK;
		i2c->S|=I2C_S_IICIF_MASK;
		i2c0_clean_TXAK();
		//if(!i2c_data.tx) i2c_data.dataArray[i2c_data.currentData]=i2c->D;

		i2c_data.callback();


	}
	else
	//CASO START
	{
		if(i2c->FLT&I2C_FLT_STARTF_MASK)
		{
			i2c->FLT|=I2C_FLT_STARTF_MASK;
			i2c->S|=I2C_S_IICIF_MASK;
			i2c_data.startCount++;
			if(i2c_data.startCount==1)
			{
				i2c0_write_data(set_slave_adress_byte(i2c_data.slaveAdress,1));
				return;
			}
			if(i2c_data.startCount==2)
			{
				i2c0_write_data(set_slave_adress_byte(i2c_data.slaveAdress,0));
				return;
			}
			//if(i2c_data.startCount<2)return;
		}
		else
		{
			i2c->S|=I2C_S_IICIF_MASK;
		}
		//CASO DATO
		if(i2c->C1&I2C_C1_TX_MASK) //TX
		{
			if(!(i2c0_check_RXAK()))
			{
				i2c0_stop_signal();
				return;
			}
			if(i2c_data.tx==0) //PARTE ESCRIBIR DE RX
			{
				if(i2c_data.rx_state==ADRESS)
				{
					i2c_data.rx_state=REGISTER;
					i2c0_write_data(i2c_data.regAdress);
				}
				else if(i2c_data.rx_state==REGISTER)
				{
					i2c_data.rx_state=RX_START;
					i2c0_repeat_start();
					return;
				}
				else if(i2c_data.rx_state==RX_START)
				{
					i2c_data.rx_state=DATA;
					i2c0_switch_to_rx();
					if(i2c_data.dataLength==1)i2c0_set_TXAK();
					uint8_t dummyRead=i2c->D;
				}
			}
			else //CASO ESCRIBIR
			{
				if(i2c_data.rx_state==ADRESS)
				{
					i2c_data.rx_state=DATA;
					i2c0_write_data(i2c_data.regAdress);
				}
				else if(i2c_data.rx_state==DATA)
				{
					if(i2c_data.currentData==i2c_data.dataLength)
					{
						//i2c_data.callback();
						i2c0_stop_signal();
						return;
					}
					uint8_t aux=i2c_data.dataArray[i2c_data.currentData];
					i2c0_write_data(aux);
					i2c_data.currentData++;
				}
			}
		}
		else //RX
		{
			/*if(i2c_data.rx_state==DUMMY)
			{
				i2c_data.rx_state=DATA;
				uint8_t dummyRead=i2c->D;
			}
			else*/ if(i2c_data.rx_state==DATA)
			{

				if(i2c_data.currentData==i2c_data.dataLength-1)
				{
					//i2c_data.callback();
					i2c0_stop_signal();
					i2c_data.dataArray[i2c_data.currentData]=i2c->D;
				}
				else
				{

					if(/*(i2c_data.dataLength!=1)&&*/(i2c_data.currentData==(i2c_data.dataLength-2)))i2c0_set_TXAK();
					//if((i2c_data.dataLength!=1)&&(i2c_data.currentData==(i2c_data.dataLength-1)))i2c0_set_TXAK();

					i2c_data.dataArray[i2c_data.currentData]=i2c->D;
					i2c_data.currentData++;

				}
			}
		}
	}
}

void i2c_read_reg(uint8_t slave_adress, uint8_t reg_adress, uint8_t* data, uint8_t dataLength, i2cIrqFun_t callback)
{

	if(i2c_ptr[0]->S&I2C_S_BUSY_MASK)return;
	i2c_data.dataArray=data;
	i2c_data.dataLength=dataLength;
	i2c_data.slaveAdress=slave_adress;
	i2c_data.regAdress=reg_adress;
	i2c_data.tx=0;
	i2c_data.rx_state=ADRESS;

	i2c_data.currentData=0;
	i2c_data.startCount=0;

	i2c_data.callback=callback;


	i2c0_set_TX();
	i2c0_start_signal();
	//i2c0_write_data(set_slave_adress_byte(slave_adress,1));
}

void i2c_write_reg(uint8_t slave_adress, uint8_t reg_adress, uint8_t* data, uint8_t dataLength, i2cIrqFun_t callback)
{
	if(i2c_ptr[0]->S&I2C_S_BUSY_MASK)return;
	i2c_data.dataArray=data;
	i2c_data.dataLength=dataLength;
	i2c_data.slaveAdress=slave_adress;
	i2c_data.regAdress=reg_adress;
	i2c_data.tx=1;
	i2c_data.rx_state=ADRESS;

	i2c_data.currentData=0;
	i2c_data.startCount=0;

	i2c_data.callback=callback;


	i2c0_set_TX();
	i2c0_start_signal();
	//i2c0_write_data(set_slave_adress_byte(slave_adress,1));
}

uint8_t set_slave_adress_byte(uint8_t slaveAdress,bool tx)
{
	uint8_t retVal;
	if(tx)retVal=(slaveAdress << 1)& 0b11111110 ;
	else retVal=(slaveAdress << 1)| 0b00000001 ;
	return retVal;
}

void i2c0_write_data(uint8_t data)
{
	i2c_ptr[0]->D=data;
}

void i2c0_switch_to_rx(void)
{
	i2c0_set_RX();
}

bool i2c0_check_RXAK(void)
{
	return ((i2c_ptr[0]->S&I2C_S_RXAK_MASK)==0);
}
