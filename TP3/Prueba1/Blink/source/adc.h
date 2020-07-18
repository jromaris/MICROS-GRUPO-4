
#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*adcIrqFun_t)(void);

void start_con(void);
void init_adc0(uint16_t* ptrValue, adcIrqFun_t callback);


#endif /* ADC_H_ */
