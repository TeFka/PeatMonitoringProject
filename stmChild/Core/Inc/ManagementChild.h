/*
 * Management.h
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#ifndef INC_MANAGEMENTCHILD_H_
#define INC_MANAGEMENTCHILD_H_

#include "stm32l4xx_hal.h"
#include <stdio.h>

//#include "Depth/PressureSensing.h"
#include "DataHandling.h"
#include "Pressure/PressureSensing.h"
#include "Temperature/TemperatureSensing.h"
#include "RF/comms.h"
#include "Depth/DepthMeasuring.h"
#include "FlashMemoryAccess.h"
#include "SDcard/SDcommunication.h"

#define Vref 3.3
#define VOLTAGE_THRESHOLD 2.8

#define idSetGPIO 		GPIOB
#define idSetGPIOPin 	GPIO_PIN_2 //D4
#define wakeUpGPIO 		GPIOB
#define wakeUpGPIOPin 	GPIO_PIN_2 //D5

//management data
#define managementDataNum				5
#define managementDataAddr				USABLE_FLASH_START
#define IDaddressAddr					managementDataAddr
#define motherIDaddressAddr				managementDataAddr+8
#define setupAddr						managementDataAddr+16
#define discoverAddr					managementDataAddr+24
#define lowBatteryModeAddr				managementDataAddr+32

//depth support variables
#define numberOfDepthVariables				7
#define depthVariablesStartAddr				USABLE_FLASH_START+FLASH_PAGE_SIZE*2
#define posAddr								depthVariablesStartAddr+8
#define output1Addr							depthVariablesStartAddr+16
#define output2Addr							depthVariablesStartAddr+24
#define max_value_1Addr						depthVariablesStartAddr+32
#define min_value_1Addr						depthVariablesStartAddr+40
#define max_value_2Addr						depthVariablesStartAddr+48
#define min_value_2Addr						depthVariablesStartAddr+56

//power pins
#define i2cGPIO GPIOB
#define i2cGPIOPin GPIO_PIN_7
#define adcGPIO i2cGPIO
#define adcGPIOPin i2cGPIOPin
#define spiGPIO i2cGPIO
#define spiGPIOPin i2cGPIOPin
#define uartGPIO GPIOB
#define uartGPIOPin GPIO_PIN_9
#define debugUARTGPIO GPIOB
#define debugUARTGPIOPin GPIO_PIN_8

#define NUM_READINGS_MAX 1
#define NEIGHBOUR_LIMIT 20

#define RTC_BKUP_DEFINE_CODE	0x327

extern uint8_t deviceOperating;

struct CommunicationHandles{

	I2C_HandleTypeDef*	I2Chandle;
	SPI_HandleTypeDef*	SPIhandle;
	ADC_HandleTypeDef*  ADChandle1;
	ADC_HandleTypeDef*  ADChandle2;
	UART_HandleTypeDef* UARThandle;
	RTC_HandleTypeDef* 	RTChandle;
};

/*
struct managementInfo{

	int* rfReceiveFlag;
	int numberOfMeasurementsStored;
	int activeMeasurement;
	struct measurementData measurements[NUM_READINGS_MAX];
	int neighbourDevices[NEIGHBOUR_LIMIT];
	int numNeighbourDevices;
	int lowBatteryMode;
};
*/

void performDepthMeasurement(ADC_HandleTypeDef*, ADC_HandleTypeDef*, struct measurementData* data);

void performPressureMeasurement(I2C_HandleTypeDef*, struct measurementData* data);

void performTemperatureMeasurement(SPI_HandleTypeDef*, struct measurementData* data);

void checkBattery(ADC_HandleTypeDef* adcHandle1, struct measurementData* data);

void setRfListening(struct measurementData* data, struct rfDataStorage* rfData, struct CommunicationHandles* handles);

int IDhandling();

void powerDownDevices();

void customRTCInit();

void synchronizationSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles);

void memorySetup();

int mainSetup(struct rfDataStorage* rfData, struct CommunicationHandles*);

void saveMeasurementData(struct measurementData* data, int index);
void extractMeasurementData(struct measurementData* data, int index);

void encodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t* dataSize);
void decodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t dataSize);

void addDeviceData(struct CommunicationHandles* handles, struct measurementData* data);

void setShutdownMode();

void mainOperation(struct rfDataStorage*, struct CommunicationHandles*);

#endif /* INC_MANAGEMENTCHILD_H_ */
