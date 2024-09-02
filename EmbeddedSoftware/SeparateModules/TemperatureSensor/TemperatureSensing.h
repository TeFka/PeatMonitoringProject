/*
 * temperatureSensing.h
 *
 *  Created on: Feb 10, 2023
 *      Author: Admin
 */

#ifndef INC_TEMPERATURE_TEMPERATURESENSING_H_
#define INC_TEMPERATURE_TEMPERATURESENSING_H_

#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "retarget.h"

#define TPSPI_CSGP GPIOB
#define TPSPI_CSPin GPIO_PIN_12

//addresses
#define TPtemperatureResultAddr 0x00
#define TPslewResultAddr        0x01
#define TPalertStatusAddr       0x02
#define TPconfigurationAddr     0x03
#define TPalertEnableAddr       0x04
#define TPtlowLimitqAddr        0x05
#define TPthighLimitAddr        0x06
#define TPhysteresisAddr        0x07
#define TPslewLimitAddr         0x08
#define TPuniqueID1Addr         0x09
#define TPuniqueID2Addr         0x0A
#define TPuniqueID3Addr         0x0B
#define TPdeviceIDAddr          0x0C

#define TPreadMask              0x01
#define TPwriteMask             0x00

#define TPdefaultConfigHigh     0x00
#define TPdefaultConfigLow     	0xA6

#define temperatureSensetivity 	0.03125;


void sendSPI(SPI_HandleTypeDef* handle, uint8_t controlVal, uint8_t registerAddr, uint8_t messageHigh, uint8_t messageLow);
int getSPI(SPI_HandleTypeDef* handle, uint8_t controlVal, uint8_t registerAddr);

int temp_getID(SPI_HandleTypeDef*);

void configureSensor(SPI_HandleTypeDef* handle);
double getTemperatureVal(SPI_HandleTypeDef* handle);

#endif /* INC_TEMPERATURE_TEMPERATURESENSING_H_ */
