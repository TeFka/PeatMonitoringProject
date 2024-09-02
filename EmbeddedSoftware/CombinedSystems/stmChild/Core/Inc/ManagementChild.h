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

//Voltage definitions
#define Vref 3.3
#define VOLTAGE_THRESHOLD 2.8

//Button specifications
#define idSetGPIO 		GPIOB
#define idSetGPIOPin 	GPIO_PIN_2 //D4
//#define wakeUpGPIO 		GPIOB
//#define wakeUpGPIOPin 	GPIO_PIN_2 //D5

//management data
#define managementDataNum				5
#define managementDataAddr				USABLE_FLASH_START
#define IDaddressAddr					managementDataAddr
#define motherIDaddressAddr				managementDataAddr+8
#define setupAddr						managementDataAddr+16
#define discoverAddr					managementDataAddr+24
#define hopsAddr						managementDataAddr+32

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

//Defined value of RTC backup
#define RTC_BKUP_DEFINE_CODE	0x327

extern uint8_t deviceOperating;

//All peripheral handles
struct CommunicationHandles{

	I2C_HandleTypeDef*	I2Chandle;
	SPI_HandleTypeDef*	SPIhandle;
	ADC_HandleTypeDef*  ADChandle1;
	ADC_HandleTypeDef*  ADChandle2;
	UART_HandleTypeDef* UARThandle;
	RTC_HandleTypeDef* 	RTChandle;
};

//Function to perform depth measurement
void performDepthMeasurement(ADC_HandleTypeDef*, ADC_HandleTypeDef*, struct measurementData* data);

//Function to perform pressure measurement
void performPressureMeasurement(I2C_HandleTypeDef*, struct measurementData* data);

//Function to perform temperature measurement
void performTemperatureMeasurement(SPI_HandleTypeDef*, struct measurementData* data);

//Function to check battery state
void checkBattery(ADC_HandleTypeDef* adcHandle1, struct measurementData* data);

//Function listen to listen for RF requests
void setRfListening(struct measurementData* data, struct rfDataStorage* rfData, struct CommunicationHandles* handles);

//Function to handle device ID changes
int IDhandling();

//Fucntion to power down all externally connected devices
void powerDownDevices();

//Function to perform child device setup and synchronization with mother device
void synchronizationSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles);

//Function to setup memory for operation
void memorySetup();

//Function to setup device for operation
int mainSetup(struct rfDataStorage* rfData, struct CommunicationHandles*);

//Function to to let device go into shutdown mode
void setShutdownMode();

//Function to perform main device operation
void mainOperation(struct rfDataStorage*, struct CommunicationHandles*);

#endif /* INC_MANAGEMENTCHILD_H_ */
