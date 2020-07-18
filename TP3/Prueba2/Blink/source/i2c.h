/*
 * i2c.h
 *
 *  Created on: 17 sep. 2019
 *      Author: marcos
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*i2cIrqFun_t)(void);

void init_i2c0(void);
void i2c_read_reg(uint8_t slave_adress, uint8_t reg_adress, uint8_t* data, uint8_t dataLength,i2cIrqFun_t callback);
void i2c_write_reg(uint8_t slave_adress, uint8_t reg_adress, uint8_t* data, uint8_t dataLength, i2cIrqFun_t callback);


#endif /* I2C_H_ */
