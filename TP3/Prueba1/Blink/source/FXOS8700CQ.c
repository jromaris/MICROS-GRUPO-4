/*
 * FXOS8700CQ.c
 *
 *  Created on: 1 oct. 2019
 *      Author: marcos
 *
 *
 */

#include "FXOS8700CQ.h"

//PRIVATE VAR
static bool tx_end=false;
static bool rx_end=false;
static SRAWDATA* mypAccelData;
static SRAWDATA* mypMagnData;
uint8_t data_buffer[FXOS8700CQ_READ_LEN];
//PRIVATE FUN
void com_end(void);
void rx_callback(void);

void FXOS8700CQ_init(void)
{
	// write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into
	// standby
	// [7-1] = 0000 000
	// [0]: active=0

	uint8_t databyte = 0x00;
	tx_end=false;
	i2cIrqFun_t callback=com_end;

	/*i2c_read_reg(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_WHOAMI, &databyte, 1,callback);
	while(tx_end==false){};

	if(databyte==FXOS8700CQ_WHOAMI_VAL)
	{
		char whoami;
		whoami++;
	}
	*/
	databyte = 0x00;
	tx_end=false;
	i2c_write_reg(  FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_CTRL_REG1,
					&databyte,
					(uint8_t) 1,
					callback);
	while(tx_end==false){};

	/*uint8_t datatest = 0x00;
	tx_end=false;
	i2c_read_reg(  FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_CTRL_REG1,
					&datatest,
					(uint8_t) 1,
					callback);
	while(tx_end==false){};

	int test2;
	test2++;*/
	// write 0001 1111 = 0x1F to magnetometer control register 1
	// [7]: m_acal=0: auto calibration disabled
	// [6]: m_rst=0: no one-shot magnetic reset
	// [5]: m_ost=0: no one-shot magnetic measurement
	// [4-2]: m_os=111=7: 8x oversampling (for 200Hz) to reduce	magnetometer noise
	// [1-0]: m_hms=11=3: select hybrid mode with accel and	magnetometer active
	//databyte = 0x1F;
	databyte = 0x9F;
	tx_end=false;
	i2c_write_reg(  FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_M_CTRL_REG1,
					&databyte,
					(uint8_t) 1,
					callback);
	while(tx_end==false){};

	// write 0010 0000 = 0x20 to magnetometer control register 2
	// [7]: reserved
	// [6]: reserved
	// [5]: hyb_autoinc_mode=1 to map the magnetometer registers to	follow the
	// accelerometer registers
	// [4]: m_maxmin_dis=0 to retain default min/max latching even	though not used
	// [3]: m_maxmin_dis_ths=0
	// [2]: m_maxmin_rst=0
	// [1-0]: m_rst_cnt=00 to enable magnetic reset each cycle
	databyte = 0x20;
	tx_end=false;
	i2c_write_reg(  FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_M_CTRL_REG2,
					&databyte,
					(uint8_t) 1,
					callback);
	while(!tx_end){};

	// write 0000 0001= 0x01 to XYZ_DATA_CFG register
	// [7]: reserved
	// [6]: reserved
	// [5]: reserved
	// [4]: hpf_out=0
	// [3]: reserved
	// [2]: reserved
	// [1-0]: fs=01 for accelerometer range of +/-4g range with 	0.488mg/LSB
	databyte = 0x01;
	tx_end=false;
	i2c_write_reg(  FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_XYZ_DATA_CFG,
					&databyte,
					(uint8_t) 1,
					callback);
	while(!tx_end){};



	// write 0000 1101 = 0x0D to accelerometer control register 1
	// [7-6]: aslp_rate=00
	// [5-3]: dr=001 for 200Hz data rate (when in hybrid mode)
	// [2]: lnoise=1 for low noise mode
	// [1]: f_read=0 for normal 16 bit reads
	// [0]: active=1 to take the part out of standby and enable	sampling

	databyte = 0x0D;
	tx_end=false;
	i2c_write_reg(  FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_CTRL_REG1,
					&databyte,
					(uint8_t) 1,
					callback);
	while(!tx_end){};

	return;

}

void ReadAccelMagnData(SRAWDATA *pAccelData, SRAWDATA *pMagnData)
{
	mypAccelData=pAccelData;
	mypMagnData=pMagnData;

	rx_end=false;

	i2cIrqFun_t callback=rx_callback;
	i2c_read_reg( 	FXOS8700CQ_SLAVE_ADDR,
					FXOS8700CQ_STATUS,
					data_buffer,
					FXOS8700CQ_READ_LEN,
					callback);
}

void com_end(void)
{
	tx_end=true;
}

void rx_callback(void)
{
	// copy the 14 bit accelerometer byte data into 16 bit words
	mypAccelData->x = 0;
	mypAccelData->y = 0;
	mypAccelData->z = 0;
	// copy the magnetometer byte data into 16 bit words
	mypMagnData->x = 0;
	mypMagnData->y = 0;
	mypMagnData->z = 0;

	mypAccelData->x = (int16_t)(((data_buffer[1] << 8) | data_buffer[2]))>> 2;
	mypAccelData->y = (int16_t)(((data_buffer[3] << 8) | data_buffer[4]))>> 2;
	mypAccelData->z = (int16_t)(((data_buffer[5] << 8) | data_buffer[6]))>> 2;
	// copy the magnetometer byte data into 16 bit words
	mypMagnData->x = (int16_t)(data_buffer[7] << 8) | data_buffer[8];
	mypMagnData->y = (int16_t)(data_buffer[9] << 8) | data_buffer[10];
	mypMagnData->z = (int16_t)(data_buffer[11] << 8) | data_buffer[12];
	rx_end=true;
}

bool isDataReady(void)
{
	return rx_end;
}
