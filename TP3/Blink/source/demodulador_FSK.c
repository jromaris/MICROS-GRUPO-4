#include "demodulador_FSK.h"
#include "adc.h"
#include "pit.h"


#define SAMPLES_DELAY	5
//#define FILTER_ORDER 18
#define FILTER_TAP_NUM 19
#define FILTER_ORDER FILTER_TAP_NUM
#define PAST_SAMPLES SAMPLES_DELAY+FILTER_ORDER
//#define PAST_SAMPLES FILTER_ORDER
#define ADC_OFFSET 2048
#define PACKAGE_SIZE 10
#define BIT_SIZE 10
#define START_DELAY 3
#define CHAR_SIZE 8
#define COMP_OFFSET 1
#define READ_DELAY 3
#define OTRO_PARAM 0

uint16_t currentValue=0;
static int16_t inSignal[PAST_SAMPLES];
//static double multSignal[FILTER_ORDER];
static float outSignal[BIT_SIZE];

//static uint8_t bitCount=0;

static int realBitCount = 0;

static char uartChar;

static char* uartPtr;

demFSKfun_t uartFun;

uint8_t dataCount=0;

static uint8_t startCount=0;
bool dataReady=false;
bool idle=true;
bool package[PACKAGE_SIZE];
bool uartValue=true;



//static uint8_t samplesCount=0;
//static uint8_t samplesCount2=0;
//double testOut[100];

const float filter_coef[19] = {
   0.007848204437773,  0.01872786035622, 0.005205461788255, -0.01893692212548,
   -0.04676280227987, -0.03762359959866,  0.02968648215476,   0.1443518276064,
     0.2540226538991,   0.2994894420974,   0.2540226538991,   0.1443518276064,
    0.02968648215476, -0.03762359959866, -0.04676280227987, -0.01893692212548,
   0.005205461788255,  0.01872786035622, 0.007848204437773
};


bool calc_outSig(void);
void push_value (void);
void sampleProcess(void);
void outSignalPush(void);
void getUartChar(void);
//void mult_inSignal(void);

void init_dem_FSK(char* charPtr, demFSKfun_t callback)
{
	uartPtr=charPtr;
	uartFun=callback;

	//ACA HAY QUE INICIAR EL ADC
	adcIrqFun_t adcCallback=sampleProcess;

	pitIrqFun_t pitCallback=start_con;
	init_adc0(&currentValue, adcCallback);
	//pitInit((uint8_t)2);
	pitStart((uint8_t)2, 12000,pitCallback);

}

void sampleProcess(void)
{
	//push_value();
	//mult_inSignal();
	dataReady=true;

}

void push_value (void)
{
	uint8_t i=0;

	while(i<PAST_SAMPLES-1)
	{
		inSignal[i]=inSignal[i+1];
		i++;
	}
	inSignal[i]=currentValue-ADC_OFFSET;

	/*samplesCount++;
	if(samplesCount==1)
	{
		samplesCount=0;
		calc_outSig();
	}*/
}

bool calc_outSig(void)
{
	float retVal=0;
	uint8_t i=0;
	bool bitValue=0;


	/*while(i<=FILTER_ORDER)
	{
		retVal+=filter_coef[FILTER_ORDER-i]*(float)(inSignal[SAMPLES_DELAY+i-1]*inSignal[i]);
		i++;
	}*/
	while(i<FILTER_ORDER)
	{
		retVal+=filter_coef[i]*(float)(inSignal[i+SAMPLES_DELAY]*inSignal[i]);
		i++;
	}
	outSignal[realBitCount]=retVal;
	realBitCount = realBitCount+1;
	if(realBitCount>=BIT_SIZE )
	{
		realBitCount=0;
		uint8_t aux=0;
//		uint8_t j=0;
		/*if(!idle)j=READ_DELAY;

		while(j<BIT_SIZE-OTRO_PARAM)
		{
			aux+=(outSignal[j]>0);
			j++;
		}*/
		//aux=outSignal[3]+outSignal[5]+outSignal[7]+outSignal[4]+outSignal[6];

		//aux=outSignal[6]+outSignal[7];

		//aux=outSignal[5]+outSignal[6];

		aux=outSignal[4]+outSignal[5]+outSignal[6];
		//package[dataCount]=(outSignal[0]+outSignal[1])>0;
		//if((aux)<((BIT_SIZE/2)-2.5))
		if((aux)<COMP_OFFSET)
		{
			bitValue=1;
		}
		else
		{
			bitValue=0;
		}
		if(bitValue==0 && idle)
		{
			startCount=startCount+1;

			if(startCount>START_DELAY)
			{
				idle=false;
				startCount=0;
			}
			else
			{
				outSignalPush();
				realBitCount=BIT_SIZE-1;
			}
		}
		else if(bitValue==1 && idle)
		{
			startCount=0;
			outSignalPush();
			realBitCount=BIT_SIZE-1;

		}
		if(idle==0)
		{
			package[dataCount]=bitValue;
 			dataCount=dataCount+1;
			if(dataCount>PACKAGE_SIZE)
			{
				//dataReady=true;
				idle=true;
				getUartChar();
				dataCount=0;
				*uartPtr=uartChar;
				uartFun();
			}
		}

	}
	//return (retVal>0);
	return 1;

}

bool isDemDataReady(void)
{
	return dataReady;
}

void getDataPackage(bool* data)
{
	//dataReady=false;
	uint8_t i=0;
	while(i<PACKAGE_SIZE)
	{
		data[i]=package[i];
		i++;
	}
}

void outSignalPush(void)
{
	uint8_t i=0;

	while(i<BIT_SIZE-1)
	{
		outSignal[i]=outSignal[i+1];
		i++;
	}

}

void getUartChar(void)
{
	uint8_t i=0;
	uartChar=0;
	while(i<CHAR_SIZE)
	{
		uartChar|=((package[i+1])<<i);
		i++;
	}
}

/*void mult_inSignal(void)
{
	uint8_t i=0;

	while(i<FILTER_ORDER-1)
	{
		multSignal[i]=multSignal[i+1];
		i++;
	}
	multSignal[i]=(double)inSignal[SAMPLES_DELAY-1]*inSignal[0];
}*/

void processDemData(void)
{
	push_value();
	calc_outSig();
	dataReady=false;

}
