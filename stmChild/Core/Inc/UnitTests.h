/*
 * UnitTests.h
 *
 *  Created on: 21 Mar 2023
 *      Author: zurly
 */

#ifndef INC_UNITTESTS_H_
#define INC_UNITTESTS_H_

#include "Pressure/PressureSensing.h"
#include "Temperature/TemperatureSensing.h"
#include "RF/comms.h"
#include "Depth/DepthMeasuring.h"
#include "FlashMemoryAccess.h"

#define Vref 3.3
#define VOLTAGE_THRESHOLD 2.8

void pressureTest(I2C_HandleTypeDef* I2Chandle, uint8_t address);

void TemperatureTest(SPI_HandleTypeDef*	SPIhandle);

void depthTest(ADC_HandleTypeDef* adcHandle1, ADC_HandleTypeDef* adcHandle2, struct DepthData* depthData);

void SDcardTest();

void rfTest(UART_HandleTypeDef* handle);

void flashTest();

void batteryTest(ADC_HandleTypeDef* adcHandle1);

#endif /* INC_UNITTESTS_H_ */
