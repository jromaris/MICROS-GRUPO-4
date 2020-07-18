#ifndef DMA_H_
#define DMA_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void DMA0_Config(uint16_t *source_add, uint16_t* dest_add, void(*funcallback)(void));

void DMA1_Config(uint16_t *source_add, uint16_t* dest_add, void(*funcallback)(void));

void DMA0_ConfigCounters(uint8_t channel, uint32_t source_full_size, uint32_t source_unit_size);

void DMA0_ConfigSourceAddress(uint8_t channel, uint32_t *source_add);

void DMA0_ConfigDestAddress(uint8_t channel, uint32_t *dest_add);

void DMA0_EnableRequest(uint8_t channel);

void DMA0_DisableRequest(uint8_t channel);

#endif /* DMA_H_ */
