/*
 * Management.h
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#ifndef INC_MANAGEMENTMOTHER_H_
#define INC_MANAGEMENTMOTHER_H_

#include "stm32l4xx_hal.h"

#include "DataHandling.h"
#include "RF/comms.h"
#include "SDcard/SDcommunication.h"
#include "FlashMemoryAccess.h"

//management data
#define managementDataNum				1
#define managementDataAddr				USABLE_FLASH_START
#define IDaddressAddr					managementDataAddr

//id set
#define statusGPIO 		GPIOB
#define statusGPIOPin 	GPIO_PIN_2 //D4
#define dataGPIO 		GPIOB
#define dataGPIOPin 	GPIO_PIN_10 //D5

//power GPIO
//power pins
#define spiGPIO GPIOB
#define spiGPIOPin GPIO_PIN_7//GPIO_PIN_9
#define uartGPIO GPIOB
#define uartGPIOPin GPIO_PIN_9
#define debugUARTGPIO GPIOB
#define debugUARTGPIOPin GPIO_PIN_8


struct CommunicationHandles{

	UART_HandleTypeDef* UARThandle;

};


struct deviceData{

	int deviceID;
	int deviceStatus;
	struct measurementData measurement;

};

struct statusData{

	uint8_t brokenDevices[100];
	uint8_t lowBatteryDevices[100];
	uint8_t brokenNum;
	uint8_t lowBatteryNum;

};

void addDeviceData(struct CommunicationHandles* handles, struct deviceData* devices, int numberOfDevices);

void requestMeasureData(struct deviceData* device, int* deviceNum, struct rfDataStorage* rfData);

int requestStatusData(struct statusData* statusInfo, struct rfDataStorage* rfData);

int IDhandling();

void decodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t dataSize);

int mainSetupRemote(struct rfDataStorage*, struct CommunicationHandles*);

void mainOperationRemote(struct rfDataStorage* rfData, struct CommunicationHandles* handles);

#endif /* INC_MANAGEMENTMOTHER_H_ */
