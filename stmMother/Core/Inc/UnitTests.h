/*
 * UnitTests.h
 *
 *  Created on: 21 Mar 2023
 *      Author: zurly
 */

#ifndef INC_UNITTESTS_H_
#define INC_UNITTESTS_H_

#include "RF/comms.h"
#include "SDcard/SDcommunication.h"
#include "FlashMemoryAccess.h"

#define Vref 3.3
#define VOLTAGE_THRESHOLD 2.8

void SDcardTest();

void rfTest(UART_HandleTypeDef* handle);

void flashTest();

void batteryTest(ADC_HandleTypeDef* adcHandle1);

#endif /* INC_UNITTESTS_H_ */
