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

//management data
#define managementDataNum				2
#define managementDataAddr				USABLE_FLASH_START
#define IDaddressAddr					managementDataAddr
#define setupAddr						managementDataAddr+8

//changing device data
#define changingDeviceDataStart					USABLE_FLASH_START+FLASH_PAGE_SIZE*2
#define deviceLowBatteryStatusStartAddress1		changingDeviceDataStart

//device data variables
#define deviceDataStartAddr						USABLE_FLASH_START+FLASH_PAGE_SIZE*4
#define numberOfDevicesAddress					deviceDataStartAddr
#define deviceIDListStartAddress				deviceDataStartAddr+8

#define DEVICE_LIMIT	50

//id set
#define idSetGPIO 		GPIOB
#define idSetGPIOPin 	GPIO_PIN_2 //D4
#define setupDoneGPIO 		GPIOB
#define setupDoneGPIOPin 	GPIO_PIN_2 //D4
#define remoteGPIO 		GPIOB
#define remoteGPIOPin 	GPIO_PIN_10 //D5

//power pins
#define depthGPIO GPIOB
#define depthGPIOPin GPIO_PIN_5
#define pressureGPIO GPIOB
#define pressureGPIOPin GPIO_PIN_7
#define SDcardGPIO GPIOB
#define SDcardGPIOPin pressureGPIOPin//GPIO_PIN_9
#define uartGPIO GPIOB
#define uartGPIOPin GPIO_PIN_9
#define debugUARTGPIO GPIOB
#define debugUARTGPIOPin GPIO_PIN_8

#define NUM_READINGS_MAX	1

#define RTC_BKUP_DEFINE_CODE	0x327

struct CommunicationHandles{

	SPI_HandleTypeDef*	SPIhandle;
	RTC_HandleTypeDef* 	RTChandle;

};

struct timeData{

	int seconds;
	int minutes;
	int hours;
	int day;
	int month;
	int year;

};

struct deviceData{

	int deviceID;
	int deviceStatus;
	struct measurementData measurement;

};

void extractMeasurementData(struct deviceData* device, uint64_t data);

void synchroniationSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles);

void setupConfirmation(struct CommunicationHandles* handles, struct rfDataStorage* rfData);

void requestRFData(struct CommunicationHandles* handles, struct deviceData* device, struct rfDataStorage* rfData);

int remoteAccessHandling(struct rfDataStorage* rfData);
void checkForRemote(struct rfDataStorage* rfData);
void sentDataToRemote(struct rfDataStorage* rfData, int remoteId);

void addDeviceData(struct CommunicationHandles* handles, struct deviceData* devices, struct timeData*, int numberOfDevices);
void addBrokenDevice(struct SDinfo* config, uint16_t deviceID);
void addLowBatteryDevice(struct SDinfo* config, uint16_t deviceID);

void checkForNewDevices(struct CommunicationHandles* handles, struct rfDataStorage* rfData);
void acknowledgeDevice(int deviceNumber, struct CommunicationHandles* handles, struct rfDataStorage* rfData);
void addNewDevices(struct rfDataStorage* rfData);
void removeDevices(uint8_t deviceToRemove[], uint8_t numOfDeviceToRemove);

void storeDeviceInitialData(uint64_t deviceIds[], uint8_t deviceNumber);
void scanDeviceForData(struct rfDataStorage* rfData, struct CommunicationHandles* handles, struct timeData*);
void scanConfirmation(struct CommunicationHandles* handles, struct rfDataStorage* rfData);

void setShutdownModeMother();

int IDhandling();

void setTimerMother(RTC_HandleTypeDef* handle, uint32_t sleepTime);

void decodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t dataSize);

int mainSetupMother(struct rfDataStorage*, struct CommunicationHandles*);
void mainOperationMother(struct rfDataStorage* rfData, struct CommunicationHandles* handles);

#endif /* INC_MANAGEMENTMOTHER_H_ */
