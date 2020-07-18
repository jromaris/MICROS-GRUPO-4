/*
 * ftm.c
 *
 *  Created on: 16 oct. 2019
 *      Author: Manuel
 */
#include "ftm.h"
#include "MK64F12.h"
#include "gpio.h"
#include "board.h"

//Punteros a las estructuras de cada FTM
static FTM_Type * ftm_ptr[]= FTM_BASE_PTRS;

//Puntero al periférico SIM
static SIM_Type *  sim=SIM;

//Vectores de interrupción para el periférico FTM
static IRQn_Type  irqEnable[]=FTM_IRQS;

//Punteros a las estructuras de cada Puerto
static PORT_Type * ports[] = PORT_BASE_PTRS;

//Arreglo de los SIM de cada puerto
static uint32_t sim_port[] = {SIM_SCGC5_PORTA_MASK, SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK, SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK};

//Número de módulo de FTM a utilizar
int FTM_number;

//Módulo del contador de FTM
int FTM_mod=0;

//Frecuencia del clock de FTM después del prescaler
uint32_t FTM_clk_frec=0;

//Variable utilizada para separar la primer lectura de input capture
bool first_cnv=true;

//Primera medición del CnV
uint32_t cnv1=0;

//Segunda medición del CnV
uint32_t cnv2=0;

//Contador de overflows para el modo de input capture
uint32_t overflow_count=0;

//Cantidad de ticks para una medición de input capture
uint32_t frec_counter=0;

//Bool que avisa si la salida del Input Capture es válida para leer
bool frec_count_rdy =false;

//Cual fue el último valor lógico recibido por modulación FSK
int prev_logic_val=2;

//Variable temporal de escritura de Input Capture
uint32_t temp_cnv=0;

//
void input_capture_callback(void);

//Puntero a función utilizado para actualizar el valor del duty cycle de la señal PWM
void (*ftm_duty_callback)(void);

//Función que modifica los registros necesarios para utilizar el pin deseado
static void set_pin (pin_t pin);

int getMod(void){return FTM_mod;}

//La función devuelve true si se puede leer el frec counter
bool is_frec_count_rdy(void){return frec_count_rdy;}

//La función devuelve el frec counter
float get_frec_count(void){ frec_count_rdy=false; return (FTM_clk_frec)/(float)(frec_counter);}

//Función que inicializa al módulo FTM
void FTM_init(FTM_params params)
{
	gpioMode(PIN_SCK,OUTPUT);
	gpioMode(PIN_MOSI,OUTPUT);

	gpioWrite(PIN_SCK,false);
	gpioWrite(PIN_MOSI,false);

	//Habilito los pines asociados a cada canal del módulo de FTM
	set_pin(PIN_FTM0_CH0);
	set_pin(PIN_FTM0_CH1);

	FTM_number=params.FTM_num;

	//Hago el Clock Gating apropiado
	switch(params.FTM_num)
	{
		case 0:
			sim->SCGC6 |= SIM_SCGC6_FTM0_MASK;
			break;
		case 1:
			sim->SCGC6 |= SIM_SCGC6_FTM1_MASK;
			break;
		case 2:
			sim->SCGC6 |= SIM_SCGC6_FTM2_MASK;
			break;
		case 3:
			sim->SCGC3 |= SIM_SCGC3_FTM3_MASK;
			break;
		default:
			sim->SCGC6 |= SIM_SCGC6_FTM0_MASK;
	}

	ftm_duty_callback=params.ftm_callback;

	//Deshabilito el periférico para la inicialización
	ftm_ptr[params.FTM_num]->SC =(ftm_ptr[params.FTM_num]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(0);

	//Deshabilito el Write Protection
	ftm_ptr[params.FTM_num]->MODE=(ftm_ptr[params.FTM_num]->MODE & ~FTM_MODE_WPDIS_MASK) | FTM_MODE_WPDIS(1);

	//Pongo el Módulo en advanced mode o Legacy compatible
	ftm_ptr[params.FTM_num]->MODE=(ftm_ptr[params.FTM_num]->MODE & ~FTM_MODE_FTMEN_MASK) | FTM_MODE_FTMEN(params.FTM_adv_func);

	//Elijo el prescaler del modulo
	ftm_ptr[params.FTM_num]->SC=(ftm_ptr[params.FTM_num]->SC & ~FTM_SC_PS_MASK) | FTM_SC_PS(params.FTM_prescaler);

	FTM_clk_frec=50000000/(1<<params.FTM_prescaler);

	//Elijo el módulo del contador
	ftm_ptr[params.FTM_num]->MOD = (ftm_ptr[params.FTM_num]->MOD & ~FTM_MOD_MOD_MASK) | FTM_MOD_MOD(params.FTM_clk_mod);

	FTM_mod=params.FTM_clk_mod;

	//Elijo el valor inicial del Contador
	ftm_ptr[params.FTM_num]->CNTIN = (ftm_ptr[params.FTM_num]->CNTIN & ~FTM_CNTIN_INIT_MASK) | FTM_CNTIN_INIT(params.FTM_init_cnt);

	//Habilito la interrupción de overflow
	ftm_ptr[params.FTM_num]->SC = (ftm_ptr[params.FTM_num]->SC & ~ FTM_SC_TOIE_MASK) | FTM_SC_TOIE_MASK;

	//Habilito la interrupción de ese periférico
	NVIC_EnableIRQ(irqEnable[params.FTM_num]);

	//Seteo la polaridad de todos los canales del FTM
	int i;
	for(i=0;i<8;i++)
	{
		ftm_ptr[params.FTM_num]->POL=(ftm_ptr[params.FTM_num]->POL & ~(FTM_POL_POL0_MASK+i)) | (FTM_POL_POL0(params.FTM_clk_POL[i]<<i));
	}

	FTM_Channel_Setup(0,PWM);
	FTM_Channel_Setup(1,INPUT_CAPTURE);

	//Elijo la fuente del clock
	ftm_ptr[params.FTM_num]->SC=(ftm_ptr[params.FTM_num]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(params.FTM_clk_select);

	DMA0_Config(&(ftm_ptr[FTM_number]->CONTROLS[1].CnV), &temp_cnv, input_capture_callback);

	DMA0_ConfigCounters(0, sizeof(temp_cnv), sizeof(temp_cnv));

	DMA0_EnableRequest(0);
}

//Función a llamar cuando ocurra una interrupción de FTM0
void FTM0_DriverIRQHandler(void)
{
	uint8_t overFlowFlag=0;
	uint8_t channel_0_flag=0;
	uint8_t channel_1_flag=0;

	//Me fijo si la interrupción es por overflow
	overFlowFlag=ftm_ptr[FTM_number]->SC & FTM_SC_TOF_MASK;

	//Me fijo si la interrupción es del canal 0
	channel_0_flag=ftm_ptr[FTM_number]->CONTROLS[0].CnSC & FTM_CnSC_CHF_MASK;

	//Me fijo si la interrupción es del canal 1
	channel_1_flag=ftm_ptr[FTM_number]->CONTROLS[1].CnSC & FTM_CnSC_CHF_MASK;

	//Si la interrupción es por overflow
	if(overFlowFlag)
	{

		//Aumento el contador de overflows
		overflow_count+=1;

		//Realizo la función encargada de modificar el duty Cycle
		ftm_duty_callback();

		//Bajo el flag de la IRQ de overflow
		ftm_ptr[FTM_number]->SC = (ftm_ptr[FTM_number]->SC & ~FTM_SC_TOF_MASK) | FTM_SC_TOF(0);

	}

	//Si la interrupción es por el canal 0 (en modo Edge-Aligned-PWM)
	if(channel_0_flag)
	{
		//Limpio el flag asociado al canal
		ftm_ptr[FTM_number]->CONTROLS[0].CnSC =(ftm_ptr[FTM_number]->CONTROLS[0].CnSC & ~FTM_CnSC_CHF_MASK) | FTM_CnSC_CHF(0);
	}

	//Si la interrupción es por el canal 1 (en modo Input Capture)
	if(channel_1_flag)
	{
		gpioWrite(PIN_MOSI,true);
		if(first_cnv)
		{
			cnv1=ftm_ptr[FTM_number]->CONTROLS[1].CnV;
			first_cnv=false;
		}
		else
		{
			cnv2=ftm_ptr[FTM_number]->CONTROLS[1].CnV;
			frec_counter=(FTM_mod+1)*overflow_count +cnv2 -cnv1;
			cnv1=cnv2;
			frec_count_rdy=true;
		}
		gpioWrite(PIN_MOSI,false);

		//Limpio el flag asociado al canal
		ftm_ptr[FTM_number]->CONTROLS[1].CnSC =(ftm_ptr[FTM_number]->CONTROLS[1].CnSC & ~FTM_CnSC_CHF_MASK) | FTM_CnSC_CHF(0);

		//Cada vez que me llega una interrupción limpio el contador de overflows
		overflow_count = 0;

	}
}

//Función que permite configurar el modo de funcionamiento de un canal de FTM
void FTM_Channel_Setup(int channel_num,mode__t chnl_mode)
{
	if(chnl_mode==OUTPUT_COMPARE)
	{
		//Configuraciones generales para el modo Output Compare
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_DECAPEN0_MASK<<((int)(8*(channel_num/2))))) | (FTM_COMBINE_DECAPEN0(0)<<((int)(8*(channel_num/2))));
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_COMBINE0_MASK<<((int)(8*(channel_num/2))))) | (FTM_COMBINE_COMBINE0(0)<<((int)(8*(channel_num/2))));
		ftm_ptr[FTM_number]->SC	= (ftm_ptr[FTM_number]->SC & ~FTM_SC_CPWMS_MASK) | (FTM_SC_CPWMS(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_MSA_MASK) | (FTM_CnSC_MSA(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_MSB_MASK) | (FTM_CnSC_MSB(1));

		//Configuro Toggle output on match
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC = (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_ELSA_MASK)	| (FTM_CnSC_ELSA(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC = (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_ELSB_MASK)	| (FTM_CnSC_ELSB(1));

		//Habilito la interrupción del canal
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_CHIE_MASK) | (FTM_CnSC_CHIE(1));
	}

	if(chnl_mode==PWM)
	{
		//Configuraciones generales para el modo Edge-Aligned PWM
		ftm_ptr[FTM_number]->QDCTRL=(ftm_ptr[FTM_number]->QDCTRL & ~FTM_QDCTRL_QUADEN_MASK) | FTM_QDCTRL_QUADEN(0);
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_DECAPEN0_MASK<<((int)(8*(channel_num/2))))) | (FTM_COMBINE_DECAPEN0(0)<<((int)(8*(channel_num/2))));
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_COMBINE0_MASK<<((int)(8*(channel_num/2))))) | (FTM_COMBINE_COMBINE0(0)<<((int)(8*(channel_num/2))));
		ftm_ptr[FTM_number]->SC	= (ftm_ptr[FTM_number]->SC & ~FTM_SC_CPWMS_MASK) | (FTM_SC_CPWMS(0));

		//Configuro Edge-Aligned-PWM
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_MSA_MASK) | (FTM_CnSC_MSA(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_MSB_MASK) | (FTM_CnSC_MSB(1));

		//Set output on Match-up
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC = (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_ELSA_MASK)	| (FTM_CnSC_ELSA(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC = (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_ELSB_MASK)	| (FTM_CnSC_ELSB(1));

		//Habilito la subida de valores a los registros MOD,CNTIN y CnV
		ftm_ptr[FTM_number]->PWMLOAD = (ftm_ptr[FTM_number]->PWMLOAD & ~FTM_PWMLOAD_LDOK_MASK ) | FTM_PWMLOAD_LDOK(1)	;

		//Incluyo al canal 0
		ftm_ptr[FTM_number]->PWMLOAD = (ftm_ptr[FTM_number]->PWMLOAD & ~(FTM_PWMLOAD_CH0SEL_MASK<<channel_num)) | (FTM_PWMLOAD_CH0SEL(1)<<channel_num);

		//Preparo para que la sincronización del PWM sea por software
		ftm_ptr[FTM_number]->MODE =	(ftm_ptr[FTM_number]->MODE  & ~FTM_MODE_PWMSYNC_MASK) | FTM_MODE_PWMSYNC(1);

		//Habilito la sincronización de PWM del canal
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_SYNCEN0_MASK<<((int)(8*(channel_num/2))))) | (FTM_COMBINE_SYNCEN0(1)<<((int)(8*(channel_num/2))));

		//Configuraciones para que la sincronización sea por Software
		ftm_ptr[FTM_number]->SYNCONF = (ftm_ptr[FTM_number]->SYNCONF & ~FTM_SYNCONF_SWWRBUF_MASK) | FTM_SYNCONF_SWWRBUF(1);
		ftm_ptr[FTM_number]->SYNCONF = (ftm_ptr[FTM_number]->SYNCONF & ~FTM_SYNCONF_SWRSTCNT_MASK) | FTM_SYNCONF_SWRSTCNT(1);
		ftm_ptr[FTM_number]->SYNCONF = (ftm_ptr[FTM_number]->SYNCONF & ~FTM_SYNCONF_SYNCMODE_MASK) | FTM_SYNCONF_SYNCMODE(1);
		ftm_ptr[FTM_number]->SYNCONF = (ftm_ptr[FTM_number]->SYNCONF & ~FTM_SYNCONF_CNTINC_MASK) | FTM_SYNCONF_CNTINC(1);
		//CHEQUEAR SI ESTO ANDA//

		//Fijo el valor inicial del canal
		update_PWM(10);

		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_DMA_MASK) | (FTM_CnSC_DMA(1));

		//Deshabilito la interrupción del canal
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_CHIE_MASK) | (FTM_CnSC_CHIE(1));
	}

	if(chnl_mode==INPUT_CAPTURE)
	{
		//Configuraciones generales para el modo Edge-Aligned PWM
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_DECAPEN0_MASK<<((int)(8*(channel_num/2))))) | (FTM_COMBINE_DECAPEN0(0)<<((int)(8*(channel_num/2))));
		ftm_ptr[FTM_number]->COMBINE = (ftm_ptr[FTM_number]->COMBINE & ~(FTM_COMBINE_COMBINE0_MASK)<<((int)(8*(channel_num/2)))) | (FTM_COMBINE_COMBINE0(0)<<((int)(8*(channel_num/2))));
		ftm_ptr[FTM_number]->SC	= (ftm_ptr[FTM_number]->SC & ~FTM_SC_CPWMS_MASK) | (FTM_SC_CPWMS(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_MSA_MASK) | (FTM_CnSC_MSA(0));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_MSB_MASK) | (FTM_CnSC_MSB(0));

		//Configuro para que capture en flanco ascendente
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC = (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_ELSA_MASK)	| (FTM_CnSC_ELSA(1));
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC = (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_ELSB_MASK)	| (FTM_CnSC_ELSB(1));

		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_DMA_MASK) | (FTM_CnSC_DMA(1));

		//Habilito la interrupción del canal
		ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC= (ftm_ptr[FTM_number]->CONTROLS[channel_num].CnSC & ~FTM_CnSC_CHIE_MASK) | (FTM_CnSC_CHIE(1));
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
	ports[pinPort]->PCR[pinBit]= PORT_PCR_MUX(4) | \
			PORT_PCR_IRQC(0) | \
			PORT_PCR_PS(0)| \
			PORT_PCR_PE(0);
}

//Función que actualiza el duty del canal de PWM
void update_PWM(int duty)
{
	//Habilito el cargar el próximo valor de CnV
	ftm_ptr[FTM_number]->PWMLOAD = (ftm_ptr[FTM_number]->PWMLOAD & ~FTM_PWMLOAD_LDOK_MASK)| ~FTM_PWMLOAD_LDOK(1);
	//Actualizo el valor del Cnv dato el duty deseado

	ftm_ptr[FTM_number]->CONTROLS[0].CnV = duty;

	//ftm_ptr[FTM_number]->CONTROLS[0].CnV = (uint32_t)(FTM_mod*(float)(duty)/100);

	//Sincronización del CnV por software
	ftm_ptr[FTM_number]->SYNC = (ftm_ptr[FTM_number]->SYNC & ~FTM_SYNC_SWSYNC_MASK)	| FTM_SYNC_SWSYNC(1);
}

void input_capture_callback(void)
{
	gpioWrite(PIN_MOSI,true);
	if(first_cnv)
	{
		cnv1=temp_cnv;
		first_cnv=false;
	}
	else
	{
		cnv2=temp_cnv;
		frec_counter=(FTM_mod+1)*overflow_count +cnv2 -cnv1;
		cnv1=cnv2;
		frec_count_rdy=true;
	}
		gpioWrite(PIN_MOSI,false);

	//Cada vez que me llega una interrupción limpio el contador de overflows
	overflow_count = 0;
}
