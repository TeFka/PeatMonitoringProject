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
#define deviceLowBatteryStatusStartAddress		changingDeviceDataStart


//device data variables
#define deviceDataStartAddr						USABLE_FLASH_START+FLASH_PAGE_SIZE*4
#define numberOfDevicesAddress					deviceDataStartAddr
#define deviceIDListStartAddress				deviceDataStartAddr+8

#define DEVICE_LIMIT 100

//Configuration button
#define idSetGPIO 		GPIOB
#define idSetGPIOPin 	GPIO_PIN_2 //D4

//Remote communication button
#define remoteGPIO 		GPIOB
#define remoteGPIOPin 	GPIO_PIN_10 //D5

//Device power pins
#define adcGPIO GPIOB
#define adcGPIOPin GPIO_PIN_5
#define I2CGPIO GPIOB
#define I2CGPIOPin GPIO_PIN_7
#define SPIGPIO I2CGPIO
#define SPIGPIOPin I2CGPIOPin//GPIO_PIN_9
#define uartGPIO GPIOB
#define uartGPIOPin GPIO_PIN_9
#define debugUARTGPIO GPIOB
#define debugUARTGPIOPin GPIO_PIN_8

//Defined value of RTC backup
#define RTC_BKUP_DEFINE_CODE	0x327

//All peripheral handles
struct CommunicationHandles{

	SPI_HandleTypeDef*	SPIhandle;
	RTC_HandleTypeDef* 	RTChandle;

};

//Structure to hold time data
struct timeData{

	int seconds;
	int minutes;
	int hours;
	int day;
	int month;
	int year;

};

//Structure to hold child device data
struct deviceData{

	uint16_t deviceID;
	uint16_t deviceStatus;
	uint16_t deviceHops;
	struct measurementData measurement;

};

//Function request  measurement data of sepcified device over RF
void requestRFData(struct CommunicationHandles* handles, struct deviceData* device, struct rfDataStorage* rfData);

//Function to handle communication with a remote
int remoteAccessHandling(struct rfDataStorage* rfData);

//Function to check for presence of a remote
void checkForRemote(struct rfDataStorage* rfData);

//Function to send data to an an active remote
void sentDataToRemote(struct rfDataStorage* rfData, int remoteId);

//Function to add latest device data to SD card
void addDeviceDataToSD(struct CommunicationHandles* handles, struct deviceData* devices, struct timeData*, int numberOfDevices);

//Function to add broken device information to SD card
void addBrokenDeviceToSD(struct SDinfo* config, uint16_t deviceID);

//function to add low battery device information to SD card
void addLowBatteryDeviceToSD(struct SDinfo* config, uint16_t deviceID);

//Function to check for new devices to discover
void checkForNewDevices(struct CommunicationHandles* handles, struct rfDataStorage* rfData);

//Function to aknowledge newly discoverred device
void acknowledgeDevice(int deviceNumber, struct CommunicationHandles* handles, struct rfDataStorage* rfData);

//Function to get devices from file in SD card
void getDevices(struct SDinfo* config, char* listFile, uint16_t* data, uint8_t* num);

//Function to add new device to the storage
void addNewDevices(struct rfDataStorage* rfData);

//Function to remove devices from storage
void removeDevices(uint16_t deviceToRemove[], uint8_t numOfDeviceToRemove);

//Setup the initial SDcard structure
void setupSDcard();

//Function to perform a scan of all devices connected to mother device
void scanDeviceForData(struct rfDataStorage* rfData, struct CommunicationHandles* handles, struct timeData*);

//Function to confirm end of scan for all devices connected to device
void scanConfirmation(struct CommunicationHandles* handles, struct rfDataStorage* rfData);

//Function to to let device go into shutdown mode
void setShutdownMode();

//Function to handle device ID changes
int IDhandling();

//Function to setup device for operation
int mainSetup(struct rfDataStorage*, struct CommunicationHandles*);

//Function to perform main device operation
void mainOperationMother(struct rfDataStorage* rfData, struct CommunicationHandles* handles);

#endif /* INC_MANAGEMENTMOTHER_H_ */
