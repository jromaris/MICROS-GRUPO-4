/*
 * ftm.h
 *
 *  Created on: 16 oct. 2019
 *      Author: Manuel
 */

#ifndef FTM_H_
#define FTM_H_

#define FREC_1 1200
#define FREC_0 2400
#define TOLERANCE 0.2

#include <stdbool.h>
#include <stdint.h>
#include "dma.h"

typedef struct FTM_params {
   int FTM_num;
   bool FTM_adv_func;
   int FTM_prescaler;
   char FTM_clk_select;
   int FTM_clk_mod;
   int FTM_init_cnt;
   bool FTM_clk_POL[8];
   void (*ftm_callback)(void);
} FTM_params;

typedef enum {OUTPUT_COMPARE,INPUT_CAPTURE,PWM} mode__t;

//Init del módulo de FTM
void FTM_init(FTM_params params);

//Función que permite setearle un modo a un canal de FTM (OUTPUT_COMPARE,INPUT_CAPTURE o PWM)
void FTM_Channel_Setup(int channel_num,mode__t chnl_mode);

//Función que actualiza el duty del canal de PWM
void update_PWM(int duty);

//Función que devuelve true si input capture devuelve algo válido
bool is_frec_count_rdy(void);

//Getter  de la cantidad de ticks entre dos flancos detectados por input Capture
float get_frec_count(void);

//Getter del Módulo del FTM
int getMod(void);

#endif /* FTM_H_ */
