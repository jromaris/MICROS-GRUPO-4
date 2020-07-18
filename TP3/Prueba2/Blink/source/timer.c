/***************************************************************************//**
  @file     timer.c
  @brief    Timer driver. Advance implementation
  @author   NicolÃ¡s Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "timer.h"

#include "SysTick.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TIMER_DEVELOPMENT_MODE    1

#define TIMER_ID_INTERNAL   0


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
	bool				enable;
	ttick_t             period;
	ttick_t             cnt;
	bool				pause;
    tim_callback_t      callback;
    uint8_t             mode        : 1;
} timer_t;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Periodic service
 */
static void timer_isr(void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static timer_t timers[TIMERS_MAX_CANT];
static tim_id_t timers_cant = TIMER_ID_INTERNAL; 	//cantidad timers


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void timerInit(void)
{
    static bool yaInit = false;
    if (yaInit)
        return;

    SysTick_Init(timer_isr); // init peripheral

    yaInit = true;
}


tim_id_t timerGetId(void)
{
#ifdef TIMER_DEVELOPMENT_MODE
    if (timers_cant > TIMERS_MAX_CANT)
    {
        return TIMER_INVALID_ID;
    }
    else
#endif // TIMER_DEVELOPMENT_MODE
    {
    	timers[timers_cant].enable=FALSE;		//todavia no esta habilitado
        timers_cant = timers_cant + 1;		//hay un timer mas
    	return (timers_cant - 1);			//devuelvo el id correspondiente
    }
}


void timerStart(tim_id_t id, ttick_t ticks, uint8_t mode, tim_callback_t callback)
{
#ifdef TIMER_DEVELOPMENT_MODE
    if ((id < TIMERS_MAX_CANT) && (mode < CANT_TIM_MODES))
#endif // TIMER_DEVELOPMENT_MODE
    {
    	//chequear que ticks !=0
    	timers[id].cnt = ticks;			//Este valor se actualizara y modificara
    	timers[id].period = ticks;		//Aqui se memorizara el valor inicial
    	timers[id].mode = mode;
    	timers[id].callback = callback;
    	timers[id].pause = FALSE;
    	timers[id].enable = TRUE;

        // ****** COMPLETAR ******
        // disable timer
        // configure timer
        // enable timer

    }
}


void timerFinish(tim_id_t id)
{
    // ****** COMPLETAR ******
	timers[id].enable=FALSE;
    // Apago el timer
    // y bajo el flag
}

void timerToggle(tim_id_t id){

	if(timers[id].enable == TRUE){
		timers[id].pause = !timers[id].pause;
	}
}


bool timerExpired(tim_id_t id)
{
    // ****** COMPLETAR ******
	//me fijo que haya expitrado y que este activado el timer
	if((timers[id].cnt==0) && (timers[id].enable)){
		return TRUE;
	}
	else
		return FALSE;
    // Verifico si expirÃ³ el timer
    // y bajo el flag
}

tim_id_t timersActive(void){
	int i;
	tim_id_t amount=0;
	for(i=0;i<TIMERS_MAX_CANT;i++){
		if(timers[i].enable) amount++;
	}
	return amount;
}

bool isSpecificTimerActive(tim_id_t num){
	return timers[num].enable;
}


void timerDelay(ttick_t ticks)
{
    timerStart(TIMER_ID_INTERNAL, ticks, TIM_MODE_SINGLESHOT, NULL);
    while (!timerExpired(TIMER_ID_INTERNAL))
    {
        // wait...
    }
}

uint32_t configTimerFreq(uint32_t ticks){
	uint32_t ms_return;
	//si pido una velocidad aceptable
	if(ticks > REAL_FREQ_SYSTICK_MS){
		 ms_return= REAL_FREQ_SYSTICK_MS*ticks;

	}
	//no puedo ir mas rapido q systick
	//si no cumple por default le pongo q se llame cada 1000 ms
	else{
		ms_return=1000*REAL_FREQ_SYSTICK_MS;
	}

	return ms_return;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void timer_isr(void)
{
    // ****** COMPLETAR ******
	int i;
	for (i=0;i<TIMERS_MAX_CANT;i++){
		if(timers[i].enable && timers[i].pause == FALSE){	//si esta habilitado y no esta en pausa
			timers[i].cnt--;			//decremento el timer
			if(timerExpired(i)){		//si termino
				timers[i].callback();		//ejecuo
				if(timers[i].mode==TIM_MODE_SINGLESHOT){		//si solo es una vez
					timers[i].enable=FALSE;							//deshabilito
				}
				else{											//si es periodico
					timers[i].cnt=timers[i].period;				//volvemos al valor inicial
				}
			}
		}
	}

    // decremento los timers activos
    // si hubo timeout!
    // 1) execute action: callback or set flag
    // 2) update state
}


/******************************************************************************/
