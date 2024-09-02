/*
 * Management.c
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#ifndef SRC_MANAGEMENT_C_
#define SRC_MANAGEMENT_C_

#include <ManagementMother.h>
#include <FlashMemoryAccess.h>
#include <retarget.h>
#include <ctype.h>

//Function to add latest device data to SD card
void addDeviceDataToSD(struct CommunicationHandles* handles, struct deviceData* devices, struct timeData* time, int numberOfDevices){

	//Set up sd card structure
	struct SDinfo sdData;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;

	//Start SD card
	if(setupCard(&sdData)){

		//iterate through all device data
		for(int i = 0;i<numberOfDevices;i++){

			//check if device was  available
			if(devices[i].deviceStatus){
				char buf[50];

				char folderName[40];

				char fileName[100];

				sprintf(folderName, "Device_%d", devices[i].deviceID);

				//Add time data
				sprintf(buf, "%d::%d::%d %d/%d/%d\n", time->hours, time->minutes, time->seconds, time->day, time->month, time->year);
				sprintf(fileName, "%s/Time.txt", folderName);
				sendSD(&sdData, fileName, buf);

				//Add depth data
				sprintf(buf, "%f\n", ((float)devices[i].measurement.depthValue/10));
				sprintf(fileName, "%s/DepthChange(mm).txt", folderName);
				sendSD(&sdData, fileName, buf);

				//Add pressure data
				sprintf(buf, "%d\n", devices[i].measurement.waterPressureValue);
				sprintf(fileName, "%s/PressureValue(Pa).txt", folderName);
				sendSD(&sdData, fileName, buf);

				//Add underground temperature data
				sprintf(buf, "%f\n", ((float)devices[i].measurement.undergroundTemperatureValue/100));
				sprintf(fileName, "%s/UndergroundTemperature(deg).txt", folderName);
				sendSD(&sdData, fileName, buf);

				//Add surface temperature data
				sprintf(buf, "%f\n", ((float)devices[i].measurement.surfaceTemperatureValue/100));
				sprintf(fileName, "%s/SurfaceTemperature(deg).txt", folderName);
				sendSD(&sdData, fileName, buf);

				//Check if battery of the device became depleted
				if(devices[i].measurement.batteryStatus == 2){
					printf("\r\nadding low battery Device");

					//Add a low battery device to SD card
					addLowBatteryDeviceToSD(&sdData, devices[i].deviceID);

					//Set batttery status to zero
					devices[i].measurement.batteryStatus = 0;
				}

				//Add battery status data
				sprintf(buf, "%d\n", devices[i].measurement.batteryStatus);
				sprintf(fileName, "%s/BatteryStatus.txt", folderName);
				sendSD(&sdData, fileName, buf);
			}
			else{
				addBrokenDeviceToSD(&sdData, devices[i].deviceID);
			}
		}

		//Close SD card
		if(!stopCard(&sdData)){

			printf("\r\nSD stop error");

		}

	}
	else{

		printf("\r\nSD start error");

	}
}

//Function request  measurement data of sepcified device over RF
void requestRFData(struct CommunicationHandles* handles, struct deviceData* device, struct rfDataStorage* rfData){

	//Define message to send
	rfData->activeTxMessageSize = 0;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS1] = device->deviceID&0xFF;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = device->deviceHops;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	int requestSuccessful = false;

	//Perform 3 attempts to request data
	for(int t = 0;t<3;t++){

		//Check if request was successful
		if(!requestSuccessful){

			int listeningCounter = 50;

			//Send request for data from child device
			Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

			printf("\r\nScanning for %d", device->deviceID);

			//wait for specified amount of time
			for(int c = 0;c<listeningCounter;c++){

				//Handle received messages
				Comms_Handler(rfData);

				//iterate over potential messages list
				for(int i = 0; i<rfData->messageNum; i++){
					printf("\r\nmessage %d type: %x", i, rfData->activeRxMessage[i][MSG_TYPE_POS]);

					//Check if a "I am here" mesasge was received
					if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_IM_HERE){
						if(rfData->newDevices.currentNumOfDevices<DEVICE_LIMIT){
							printf("\r\nNew device found %d", rfData->fromID[i]);

							//Send back acknowledgement of devices
							acknowledgeDevice(rfData->fromID[i], handles, rfData);

							//Save new device data
							rfData->newDevices.newDevices[rfData->newDevices.numOfNewDevices] = rfData->fromID[i];
							rfData->newDevices.newDevicesHops[rfData->newDevices.numOfNewDevices] = rfData->activeRxMessage[i][MSG_NUM_HOPS_POS]+1;
							rfData->newDevices.numOfNewDevices++;
						}
					}

					//Check if data message was provided
					if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_SEND_DATA){
						//Check if request came from scanned device
						if(rfData->fromID[i] == device->deviceID){
							printf("got scan reply from: %d", device->deviceID);
							//Confirm that request was succeful
							requestSuccessful = true;
							device->deviceStatus = 1;
							printf("\r\ndecoding data");

							//retrieve data from device
							uint8_t allData[rfData->activeRxMessage[i][MSG_SIZE_POS]];
							for(int n = 0;n<rfData->activeRxMessage[i][MSG_SIZE_POS];n++){

									allData[n] = rfData->activeRxMessage[i][n+MSG_BODY_START];

							}

							//Decode the received data
							decodeMeasurementData(&(device->measurement), allData, rfData->activeRxMessage[i][MSG_SIZE_POS]);

							//Print received data
							printf("\r\nResulting measurement data: %d, %d, %d, %d, %d",
									device->measurement.depthValue,
									device->measurement.waterPressureValue,
									device->measurement.surfaceTemperatureValue,
									device->measurement.undergroundTemperatureValue,
									device->measurement.batteryStatus);

						}
					}
				}

				//Stop scanning for device if request successful
				if(requestSuccessful) break;

				HAL_Delay(200);

			}
		}
	}

	//Set device as lost if request was unsuccessful
	if(!requestSuccessful){
		device->deviceStatus = 0;
	}

}

//Function to perform a scan of all devices connected to mother device
void scanDeviceForData(struct rfDataStorage* rfData, struct CommunicationHandles* handles, struct timeData* time){

	printf("\r\nScanning devices");

	//get device number
	uint64_t deviceNum[1];
	Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);

	//get device data
	uint64_t deviceInfo[deviceNum[0]+1];
	Flash_Read_Data(deviceIDListStartAddress, deviceInfo,  deviceNum[0]);

	//get device IDs
	uint16_t deviceIds[deviceNum[0]+1];
	uint8_t deviceHops[deviceNum[0]+1];

	//Decompress device information
	deviceDataDecompression(deviceInfo, deviceNum[0], deviceIds, deviceHops);

	//retrieve battery statuses
	uint64_t deviceLowBatteryStatuses[(deviceNum[0]/64)+1];
	Flash_Read_Data(deviceLowBatteryStatusStartAddress, deviceLowBatteryStatuses, (deviceNum[0]/64)+1);

	uint64_t deviceSeparateLowBatteryStatuses[deviceNum[0]+1];

	//Placeholders for devices to remove
	uint16_t lostDevices[deviceNum[0]];
    uint8_t lostDevicesNum = 0;

	struct deviceData allDeviceData[deviceNum[0]];
	printf("Devices to scan: %d", (uint16_t)deviceNum[0]);
	//Retrieve separate battery statuses
	for(int i = 0;i< deviceNum[0];i++){

		//Add device IDs
		allDeviceData[i].deviceID = deviceIds[i];
		//Add device hops
		allDeviceData[i].deviceHops = deviceHops[i];
		//Add device battery statuses
		deviceSeparateLowBatteryStatuses[i] = (deviceLowBatteryStatuses[i/64]>>i)&(0x01);
		printf("\r\nDevice %ld low battery status: %ld", (uint32_t)allDeviceData[i].deviceID, (uint32_t)deviceSeparateLowBatteryStatuses[i]);

	}

	//Power on communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Iterate through all devices to scan
	for(int i = 0; i<deviceNum[0];i++){

		printf("\r\nrequesting from: %d", allDeviceData[i].deviceID);

		//Request data from a device
		requestRFData(handles, &(allDeviceData[i]), rfData);

		//Check if device could not be reached
		if(!allDeviceData[i].deviceStatus){
			printf("\r\ndevice %d not found", allDeviceData[i].deviceID);
			//Ste device as lost
			lostDevices[lostDevicesNum] = allDeviceData[i].deviceID;
			lostDevicesNum++;
		}

		//Check if battery of device is depleted
		if(allDeviceData[i].measurement.batteryStatus == 0){
			//Is it
			if(!deviceSeparateLowBatteryStatuses[i]){

				allDeviceData[i].measurement.batteryStatus = 2;
				deviceSeparateLowBatteryStatuses[i] = 0;
			}
		}
	}

	//Power off communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf activation pin

	printf("Adding data to SD card");
	//Power on SD card
	HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Add device data to SD card
	addDeviceDataToSD(handles, allDeviceData, time, deviceNum[0]);

	//Power off SD card
	HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_RESET);

	printf("Updating battery states");
	//Iterate over all device battery states
	for(int i = 0;i<deviceNum[0];i++){
		deviceLowBatteryStatuses[i/64] = deviceLowBatteryStatuses[i/64] | ((deviceSeparateLowBatteryStatuses[i]&0x01)<<i);
	}
	//Write latest battery statuses
	Flash_Write_Data(changingDeviceDataStart, deviceLowBatteryStatuses, (deviceNum[0]/64)+1);

	printf("Removing lsot devices");
	//Remove lost devices from flash memory
	removeDevices(lostDevices, lostDevicesNum);

	//Power on communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);

	HAL_Delay(1000);

	printf("Confirming Scan");
	//Send Scan confirmation to childe devices
	scanConfirmation(handles, rfData);

	//Power off communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf activation pin

}

//Function to confirm end of scan for all devices connected to mother device
void scanConfirmation(struct CommunicationHandles* handles, struct rfDataStorage* rfData){

	//MEssage to send as confirmation
	rfData->activeTxMessageSize = 3;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_DONE;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = 10;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	//Retrive latest system time
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);

	//Add system time to message content
	rfData->activeTxMessage[MSG_BODY_START] = sTime.Hours;
	rfData->activeTxMessage[MSG_BODY_START+1] = sTime.Minutes;
	rfData->activeTxMessage[MSG_BODY_START+2] = sTime.Seconds;

	//Set destination as zero to forward so that child device forward the message
	rfData->activeTxMessage[MSG_TO_ID_POS1] = 0;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = 0;

	//Send confirmation message
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
	HAL_Delay(2000);
	//Send confirmation message again in case previous could not be received
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
}

//Function to check for new devices to discover
void checkForNewDevices(struct CommunicationHandles* handles, struct rfDataStorage* rfData){

	//Power on communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Define how long to listen fro devices
	int listeningCounter = 50;

	printf("\r\nChecking for new devices");

	//Listeing for device
	for(int i = 0;i<listeningCounter;i++){

		//handling incoming messages
		Comms_Handler(rfData);

		//iterate over potential messages list
		for(int i = 0; i<rfData->messageNum; i++){

			//Check if "I am here" message was received
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_IM_HERE){
				printf("\r\nNew device found: %d", rfData->fromID[i]);

				//Send back aknoeledgemtn of devices
				acknowledgeDevice(rfData->fromID[i], handles, rfData);

				//Save new device data
				rfData->newDevices.newDevices[rfData->newDevices.numOfNewDevices] = rfData->fromID[i];
				rfData->newDevices.newDevicesHops[rfData->newDevices.numOfNewDevices] = rfData->activeRxMessage[i][MSG_NUM_HOPS_POS]+1;
				rfData->newDevices.numOfNewDevices++;

			}
		}
		HAL_Delay(100);

	}
	printf("\r\nCheck done");
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf activation pin

}

//Function to acknowledgment newly discoverred device
void acknowledgeDevice(int deviceNumber, struct CommunicationHandles* handles, struct rfDataStorage* rfData){

	//Create message for child device acknowledgment
	rfData->activeTxMessageSize = 3;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_YOUR_THERE;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	printf("Acknowledging: %d->%d and %d", deviceNumber, deviceNumber&0xFF, (deviceNumber>>8)&0xFF);
	rfData->activeTxMessage[MSG_TO_ID_POS1] = deviceNumber&0xFF;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = (deviceNumber>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = 10;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	//retrieve current system time
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);

	//add time in message content
	rfData->activeTxMessage[MSG_BODY_START] = sTime.Hours;
	rfData->activeTxMessage[MSG_BODY_START+1] = sTime.Minutes;
	rfData->activeTxMessage[MSG_BODY_START+2] = sTime.Seconds;

	//send acknowledgment to child device
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

}

//Function to remove devices from storage
void removeDevices(uint16_t deviceToRemove[], uint8_t numOfDeviceToRemove){

	//Check if any devices need to be removed
	if(numOfDeviceToRemove>0){

		//Retrive number of devices
		uint64_t deviceNum[1];
		Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);

		//create new array for device data
		uint64_t allDeviceData[deviceNum[0]+2];

		//Retrieve current devices in flash
		uint64_t deviceIds[deviceNum[0]+1];
		Flash_Read_Data(deviceIDListStartAddress, deviceIds, deviceNum[0]);

		//battery statuses
		uint64_t deviceLowBatteryStatuses[(deviceNum[0]/64)+1];
		Flash_Read_Data(deviceLowBatteryStatusStartAddress, deviceLowBatteryStatuses, (deviceNum[0]/64)+1);

		uint64_t deviceSeparateLowBatteryStatuses[deviceNum[0]+1];

		//Iterate through stored devices
		for(int i = 0;i<deviceNum[0]; i++){
			int deviceCanBeKept = 1;

			//Iterate through removable device
			for(int n = 0;n<numOfDeviceToRemove;n++){

				//Check if device IDs match
				if((deviceIds[i]&0xFF) == deviceToRemove[n]){

					deviceCanBeKept = 0;

				}

			}

			//check if device should be kept
			if(deviceCanBeKept){

				allDeviceData[i+1] = deviceIds[i];
				deviceSeparateLowBatteryStatuses[i] = (deviceLowBatteryStatuses[i/64]>>i)&(0x01);
			}
		}

		//Update new number of devices
		deviceNum[0]-=numOfDeviceToRemove;

		//Add number of devices to main array
		allDeviceData[0] = deviceNum[0];

		//Write new data to flash memory
		Flash_Write_Data(deviceDataStartAddr, allDeviceData, deviceNum[0]+1);

		for(int i = 0;i<deviceNum[0];i++){
			deviceLowBatteryStatuses[i/64] = deviceLowBatteryStatuses[0] | ((deviceSeparateLowBatteryStatuses[i]&0x01)<<i);
		}
		Flash_Write_Data(changingDeviceDataStart, deviceLowBatteryStatuses, (deviceNum[0]/64)+1);

	}

}

//Function to add new device to the storage
void addNewDevices(struct rfDataStorage* rfData){

	printf("\r\nnew devices to add: %d", rfData->newDevices.numOfNewDevices);
	//Check if device need to be added
	if(rfData->newDevices.numOfNewDevices>0){

		printf("\r\nAdding new devices");

		//Add new device to flash memory

		//Retrieve number of devices
		uint64_t deviceNum[1];
		Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);
		printf("old number of devices: %d", (uint16_t)deviceNum[0]);

		//Create array for all devices
		uint64_t allDeviceData[deviceNum[0]+rfData->newDevices.numOfNewDevices+1];

		//Get device info from flash memory
		uint64_t deviceInfo[deviceNum[0]+rfData->newDevices.numOfNewDevices];
		Flash_Read_Data(deviceIDListStartAddress, deviceInfo, deviceNum[0]);

		//Add new device data to the
		for(int i = 0;i<rfData->newDevices.numOfNewDevices; i++){
			//Add device ID
			deviceInfo[deviceNum[0]+i] = rfData->newDevices.newDevices[i];

			//Add device hops
			deviceInfo[deviceNum[0]+i] = deviceInfo[deviceNum[0]+i]|((uint64_t)rfData->newDevices.newDevicesHops[i]<<16);

		}

		//Add data device data to main array
		for(int i = 0;i<deviceNum[0];i++){

			allDeviceData[i+1] = deviceInfo[i];

		}

		/*for(int i = 0;i<rfData->newDevices.numOfNewDevices; i++){
			printf("\r\nAdding device %d to flash", rfData->newDevices.newDevices[i]);
			allDeviceData[deviceNum[0]+i+1] = rfData->newDevices.newDevices[i];
		}*/

		//Update overall device num
		deviceNum[0]+=rfData->newDevices.numOfNewDevices;

		printf("new number of devices: %d", (uint16_t)deviceNum[0]);
		//add new device number to flash array
		allDeviceData[0] = deviceNum[0];

		//write latest data to flash
		Flash_Write_Data(deviceDataStartAddr, allDeviceData, deviceNum[0]+1);

		//Power on SD card
		HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_SET);
		HAL_Delay(100);

		//Add new device to SD card

		//Declare SD card structure
		struct SDinfo config;

		FATFS fs; 	//Fatfs handle
		FIL fil; 		//File handle
		FRESULT fres;

		config.fs = &fs;
		config.fil = &fil;
		config.fres = &fres;

		//Reset SD card
		if(stopCard(&config)){
			printf("\r\ncard unmounted");
		}
		else{

			printf("\r\nfail to unmount card");
		}

		//Start SD card
		if(setupCard(&config)){
			printf("\r\ncard mounted");

			char buffer[50];

			//Add new storage for each device
			for(int i = 0;i<rfData->newDevices.numOfNewDevices; i++){
				printf("saving initial data for %d device", rfData->newDevices.newDevices[i]);
				sprintf(buffer, "Device_%d\n", (uint16_t)rfData->newDevices.newDevices[i]);

				//Add device to device list
				sendSD(&config, "AllDeviceInfo.txt", buffer);

				//Create a separate folder for device measurement data
				f_mkdir(buffer);
			}

			//Close SD card
			if(stopCard(&config)){
				printf("\r\ncard unmounted");
			}
			else{

				printf("\r\nfail to unmount card");
			}
		}
		else{

			printf("\r\nfail to mount card");
		}

		//Power off SD card
		HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_RESET);
	}
}

//Setup the initial SDcard structure
void setupSDcard(){

	//Power on SD card
	HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Define SD card structure
	struct SDinfo config;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	config.fs = &fs;
	config.fil = &fil;
	config.fres = &fres;

	//Reser SD card
	if(stopCard(&config)){
		printf("\r\ncard unmounted");
	}
	else{

		printf("\r\nfail to unmount card");
	}

	//Open SDcard
	if(setupCard(&config)){
		printf("\r\ncard mounted");

		//Create initial files for device information storage
		*config.fres = f_open(config.fil, "AllDeviceInfo.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		*config.fres = f_close(config.fil);

		*config.fres = f_open(config.fil, "BrokenDevices.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		*config.fres = f_close(config.fil);

		*config.fres = f_open(config.fil, "LowBatteryDevices.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		*config.fres = f_close(config.fil);

		//Close SD card
		if(stopCard(&config)){
			printf("\r\ncard unmounted");
		}
		else{

			printf("\r\nfail to unmount card");
		}
	}
	else{

		printf("\r\nfail to mount card");
	}

	//Power off SD card
	HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_RESET);
}

//Function to add broken device information to SD card
void addBrokenDeviceToSD(struct SDinfo* config, uint16_t deviceID){

	char buf[10];
	printf("\r\nAddign broken device");

	//Send specified broken device ID to SD card
	sprintf(buf, "Device_%d\n", deviceID);
	sendSD(config, "BrokenDevices.txt", buf);

}

//function to add low battery device information to SD card
void addLowBatteryDeviceToSD(struct SDinfo* config, uint16_t deviceID){

	char buf[10];
	printf("\r\nAdding low battery device");

	//Send specified low battery device ID to SD card
	sprintf(buf, "Device_%d\n", deviceID);
	sendSD(config, "LowBatteryDevices.txt", buf);

}

//Function to get devices from file in SD card
void getDevices(struct SDinfo* config, char* listFile, uint16_t* data, uint8_t* num){

	char buf[1000];
	//printf("\r\ngetting SD data");
	getSD(config, listFile, buf);
	//printf("\r\nretrieved data: %s", buf);
	int iterator = 0;
	for(int i = 0; i<strlen(buf);i++){
		//printf("\r\nCurrent data: %c", buf[i]);
		if(buf[i]=='_'){
			int numberIterator = i+1;

			uint16_t gatheredNum = 0;
			//printf("\r\nnumber started");
			int gettingNumber = 1;
			while(gettingNumber){
				//printf("\r\nchecking data: %c", buf[numberIterator]);
				if(isdigit(buf[numberIterator])){
					//printf("\r\nadding number %c to %d", buf[i], gatheredNum);
					gatheredNum = gatheredNum*10 + (buf[numberIterator] - '0');

				}
				else{
					gettingNumber = 0;
				}
				numberIterator++;
			}

			data[iterator] = gatheredNum;
			iterator++;
		}

	}

	*num = iterator;

}

//Function to check for presence of a remote
void checkForRemote(struct rfDataStorage* rfData){

	//retrieve mother ID
	uint64_t managementBuf[managementDataNum];
	Flash_Read_Data(managementDataAddr, managementBuf, managementDataNum);

	//Initialize comms
	init_comms(rfData, DEVICE_PARENT, managementBuf[0], 0);

	//Power on communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Check ofr remote for 10s
	for(int i = 0;i<10;i++){
		printf("\r\nchecking for remote");
		//Handle incoming messages
		Comms_Handler(rfData);

		//iterate over potential messages list
		for(int i = 0; i<rfData->messageNum; i++){
			//add device
			printf("\r\nReceived message type: %x", rfData->activeRxMessage[i][MSG_TYPE_POS]);

			//Check if data was requested by remote
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_GIVE_STATUS_DATA){
				printf("\r\nsending data to remote");

				//Send data to remote
				sentDataToRemote(rfData, rfData->fromID[i]);
				break;
			}
		}
		HAL_Delay(1000);
	}

	//Power off communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);

}

//Function to send data to an an active remote
void sentDataToRemote(struct rfDataStorage* rfData, int remoteId){

	//Prepare SD card
	struct SDinfo sdData;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;

	uint16_t lowBatteryDevices[100];
	uint16_t brokenDevices[100];

	uint8_t lowBatteryNum = 0;
	uint8_t brokenNum = 0;

	//Power on SD card
	HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Open SD cardd
	if(setupCard(&sdData)){

		//Retrierve broken devices from SD card
		getDevices(&sdData, "BrokenDevices.txt",  brokenDevices, &brokenNum);

		//Retrieve low battery devices from SD card
		getDevices(&sdData, "LowBatteryDevices.txt",  lowBatteryDevices, &lowBatteryNum);

		//Stop SD card
		if(!stopCard(&sdData)){

			printf("\r\nSD stop error");

		}

	}
	else{

		printf("\r\nSD start error");

	}

	//Power off SD card
	HAL_GPIO_WritePin(SPIGPIO, SPIGPIOPin, GPIO_PIN_RESET);

	//Show retrieved devices
	/*printf("\r\nBroken devices: %d", brokenNum);
	for(int i = 0;i<brokenNum;i++){

		printf("\r\n%d", brokenDevices[i]);

	}
	printf("\r\nLow battery devices: %d", lowBatteryNum);
	for(int i = 0;i<lowBatteryNum;i++){

		printf("\r\n%d", lowBatteryDevices[i]);

	}*/

	//Message to send broken devices
	rfData->activeTxMessageSize = brokenNum;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_BROKEN_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS1] = remoteId&0xFF;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = (remoteId>>8)&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	int iterator = 0;

	//Power on communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Send all broken device in waves
	for(int i = 0;i<brokenNum;i++){

		rfData->activeTxMessage[iterator+MSG_BODY_START] = brokenDevices[i]&0xFF;
		rfData->activeTxMessage[iterator+1+MSG_BODY_START] = (brokenDevices[i]>>8)&0xFF;
		//Send data if 8 bytes of data were gathered
		if(iterator==8){
			rfData->activeTxMessageSize = iterator;
			rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
			Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
			iterator = 0;
			HAL_Delay(300);
		}
		else{
			iterator+=2;
		}
	}

	if(iterator>0){
		rfData->activeTxMessageSize = iterator;
		rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
		Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
		HAL_Delay(300);
	}

	//low battery devices
	rfData->activeTxMessageSize = lowBatteryNum;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_LOW_BATTERY_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS1] = remoteId&0xFF;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = (remoteId>>8)&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	iterator = 0;
	for(int i = 0;i<lowBatteryNum;i++){

		rfData->activeTxMessage[iterator+MSG_BODY_START] = lowBatteryDevices[i]&0xFF;
		rfData->activeTxMessage[iterator+1+MSG_BODY_START] = (lowBatteryDevices[i]>>8)&0xFF;
		if(iterator==8){
			rfData->activeTxMessageSize = iterator;
			rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
			Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
			HAL_Delay(300);
		}
		else{
		    iterator+=2;
		}
	}
	if(iterator>0){
		rfData->activeTxMessageSize = iterator;
		rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
		Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
		HAL_Delay(300);
	}
	HAL_Delay(3000);
	printf("\r\nSending DONE message");
	rfData->activeTxMessageSize = 0;
	//rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_REMOTE_DONE;
	rfData->activeTxMessage[MSG_TO_ID_POS1] = remoteId&0xFF;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = (remoteId>>8)&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = 0;

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
	HAL_Delay(3000);
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

	//Power off communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);
}

//Function to setup memory for operation
void memorySetup(){

	//printf("setting up memory");
	uint64_t setupData[managementDataNum];
	Flash_Read_Data(managementDataAddr, setupData, managementDataNum);
	setupData[1] = 1;

	//printf("writting memory");
	Flash_Write_Data(managementDataAddr, setupData, managementDataNum);

	//Refersh number of devices
	uint64_t deviceNum[] = {0};
	Flash_Write_Data(numberOfDevicesAddress, deviceNum, 1);

	//Refresh all changing data
	Flash_Erase_Data(changingDeviceDataStart, 1);
}

//Function to setup device for operation
int mainSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	printf("\r\nSetting up");
	//Check if user wants to reset device IDs
	if(IDhandling()){
		return 0;
	}

	//Check if user wants to use remote
	if(remoteAccessHandling(rfData)){
		printf("\r\nHandling remote");
		//go to shutdown mode
		setShutdownMode();

	}

	//Retrieve setup bit from flash memory
	uint64_t setupBit[1];
	Flash_Read_Data(setupAddr, setupBit, 1);

	//Check if setup should be performed
	if(setupBit[0]!=1){
		printf("\r\nperforming setup");

		//Add initila data to SD card
		setupSDcard();

		//Set memory values for setup
		memorySetup();
		printf("\r\nSetup done");
		setShutdownMode(handles->RTChandle);

	}

	return 1;

}

//Function to handle communication with a remote
int remoteAccessHandling(struct rfDataStorage* rfData){

	int remoteListening = 0;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

	//Check if user wants to lsiten for remote
	if(HAL_GPIO_ReadPin(remoteGPIO, remoteGPIOPin)== GPIO_PIN_SET)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		remoteListening = 1;

		//Check for remote signals
		checkForRemote(rfData);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
	return remoteListening;

}

//Function to handle device ID changes
int IDhandling(){

	printf("\r\nChecking ID set pin : %d ", HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin));

	if(HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin)== GPIO_PIN_SET)
	{
		uint64_t managementBuf[managementDataNum];
		uint64_t rxBuf[2];
		uint32_t buf;

		//Request user to set mother device ID
		printf("\r\nSet device ID");
		scanf("%ld", &buf);
		printf("\r\nData to write: %ld", (uint32_t)buf);
		managementBuf[0] = 70;

		//Set other configuration flags
		managementBuf[1] = 0;

		//Write configuration data to flash memory
		if(Flash_Write_Data(managementDataAddr, managementBuf, managementDataNum) == HAL_OK){

			Flash_Read_Data(managementDataAddr, rxBuf, managementDataNum);
			printf("\r\nID set to %ld", (uint32_t)rxBuf[0]);
			printf("\r\nSetup bit set to: %ld", (uint32_t)rxBuf[1]);
		}

		//Refresh number of active devices
		uint64_t deviceBuf[1] = {0};
		Flash_Write_Data(numberOfDevicesAddress, deviceBuf, 1);
		Flash_Read_Data(numberOfDevicesAddress, rxBuf, 1);
		printf("\r\nSet device num to %ld", (uint32_t)rxBuf[0]);
		return 1;
	}

	return 0;
}

//Function to to let device go into shutdown mode
void setShutdownMode(){

	//Suspend system tick to prevent device from waking up beforehand
	printf("\r\nEntering Shutdown Mode");
	HAL_SuspendTick();
	HAL_PWREx_EnterSHUTDOWNMode();

}

//Function to perform main device operation
void mainOperation(struct rfDataStorage* rfData, struct CommunicationHandles* handles)
{
	//Retrieve current system time
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);
	printf("\r\nOperating at %02d::%02d::%02d",  sTime.Hours, sTime.Minutes, sTime.Seconds);

	//Save system time of when mother device woke up
	struct timeData timeInfo;
	timeInfo.seconds = sTime.Seconds;
	timeInfo.minutes = sTime.Minutes;
	timeInfo.hours = sTime.Hours;
	timeInfo.day = sDate.Date;
	timeInfo.month = sDate.Month;
	timeInfo.year = sDate.Year;

	//Create new device holders
	struct newDevicesStorage newDevices;
	newDevices.numOfNewDevices = 0;
	rfData->newDevices = newDevices;

	//get device number
	uint64_t deviceNum[1];
	Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);
	newDevices.currentNumOfDevices =  deviceNum[0];

	//Wait for child devices to wake up
	HAL_Delay(5000);

	//Retrieve dedvice ID
	uint64_t idBuf[1];
	Flash_Read_Data(IDaddressAddr, idBuf, 1);

	//Initialize comms
	init_comms(rfData, DEVICE_PARENT, idBuf[0], 0);

	//Check for new devices
	if(newDevices.currentNumOfDevices<DEVICE_LIMIT){
		checkForNewDevices(handles, rfData);
	}

	//Scan for measurement data in aknowledged devices
	scanDeviceForData(rfData, handles, &timeInfo);

	//Add newly discovered devices to storage
	addNewDevices(rfData);

	//Go to shutdown mode
	setShutdownMode();
}

#endif /* SRC_MANAGEMENT_C_ */
