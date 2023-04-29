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

void addDeviceData(struct CommunicationHandles* handles, struct deviceData* devices, struct timeData* time, int numberOfDevices){

	struct SDinfo sdData;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;

	if(setupCard(&sdData)){

		for(int i = 0;i<numberOfDevices;i++){

			if(devices[i].deviceStatus){
				char buf[50];

				char folderName[40];

				char fileName[100];

				sprintf(folderName, "Device_%d", devices[i].deviceID);

				for(int n = 0; n<NUM_READINGS_MAX;n++){

					sprintf(buf, "%d::%d::%d %d/%d/%d\n", time->hours, time->minutes, time->seconds, time->day, time->month, time->year);
					sprintf(fileName, "%s/Time.txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%f\n", ((float)devices[i].measurement.depthValue/10));
					sprintf(fileName, "%s/DepthChange(mm).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", devices[i].measurement.waterPressureValue);
					sprintf(fileName, "%s/PressureValue(Pa).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%f\n", ((float)devices[i].measurement.undergroundTemperatureValue/100));
					sprintf(fileName, "%s/UndergroundTemperature(deg).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%f\n", ((float)devices[i].measurement.surfaceTemperatureValue/100));
					sprintf(fileName, "%s/SurfaceTemperature(deg).txt", folderName);
					sendSD(&sdData, fileName, buf);

					if(devices[i].measurement.batteryStatus == 2){
						printf("\r\nadding low battery Device");
						addLowBatteryDevice(&sdData, devices[i].deviceID);
						devices[i].measurement.batteryStatus = 0;
					}

					sprintf(buf, "%d\n", devices[i].measurement.batteryStatus);
					sprintf(fileName, "%s/BatteryStatus.txt", folderName);
					sendSD(&sdData, fileName, buf);

				}
			}
			else{
				addBrokenDevice(&sdData, devices[i].deviceID);
			}
		}

		if(!stopCard(&sdData)){

			printf("\r\nSD stop error");

		}

	}
	else{

		printf("\r\nSD start error");

	}
}

void requestRFData(struct CommunicationHandles* handles, struct deviceData* device, struct rfDataStorage* rfData){

	//Define message to send
	rfData->activeTxMessageSize = 0;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS] = device->deviceID;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	int requestSuccessful = false;
	for(int t = 0;t<3;t++){

		if(!requestSuccessful){

			int listeningCounter = 20;

			Send(rfData->activeTxMessage, rfData->activeTxMessageSize+6);

			printf("\r\nScanning for %d", device->deviceID);

			for(int c = 0;c<listeningCounter;c++){

				Comms_Handler(rfData);

				for(int i = 0; i<rfData->messageNum; i++){
					printf("\r\nmessage %d type: %x", i, rfData->activeRxMessage[i][MSG_TYPE_POS]);

					if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_IM_HERE){
						printf("\r\New device found", rfData->activeRxMessage[i][MSG_FROM_ID_POS]);
						acknowledgeDevice(rfData->activeRxMessage[i][MSG_FROM_ID_POS], handles, rfData);
						rfData->newDevices.newDevices[rfData->newDevices.numOfNewDevices] = rfData->activeRxMessage[i][MSG_FROM_ID_POS];
						rfData->newDevices.numOfNewDevices++;

					}

					if(rfData->activeRxMessage[i][MSG_FROM_ID_POS] == device->deviceID){
						printf("got scan reply from: %d", device->deviceID);
						requestSuccessful = true;
						device->deviceStatus = 1;
						printf("\r\ndecoding data");
						uint8_t allData[rfData->activeRxMessage[i][MSG_SIZE_POS]];
						for(int n = 0;n<rfData->activeRxMessage[i][MSG_SIZE_POS];n++){

								allData[n] = rfData->activeRxMessage[i][n+MSG_BODY_START];

						}
						decodeMeasurementData(&(device->measurement), allData, rfData->activeRxMessage[i][MSG_SIZE_POS]);

						printf("\r\nResulting measurement data: %d, %d, %d, %d, %d",
								device->measurement.depthValue,
								device->measurement.waterPressureValue,
								device->measurement.surfaceTemperatureValue,
								device->measurement.undergroundTemperatureValue,
								device->measurement.batteryStatus);

					}
				}

				if(requestSuccessful) break;

				HAL_Delay(100);

			}
		}
	}

	if(!requestSuccessful){
		device->deviceStatus = 0;
	}

}

void scanDeviceForData(struct rfDataStorage* rfData, struct CommunicationHandles* handles, struct timeData* time){

	printf("\r\nScanning devices");

	//get device number
	uint64_t deviceNum[1];
	Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);

	//get device IDs
	uint64_t deviceIds[deviceNum[0]+1];
	Flash_Read_Data(deviceIDListStartAddress, deviceIds,  deviceNum[0]);
	//retrieve battery statuses
	uint64_t deviceLowBatteryStatuses[1];
	Flash_Read_Data(deviceLowBatteryStatusStartAddress1, deviceLowBatteryStatuses, 1);
	uint64_t deviceSeparateLowBatteryStatuses[deviceNum[0]];
	struct deviceData allDeviceData[deviceNum[0]];
	uint8_t lostDevices[deviceNum[0]];
	uint8_t lostDevicesNum = 0;
	printf("\r\nDevices to scan: %ld", (uint32_t)deviceNum[0]);
	for(int i = 0;i< deviceNum[0];i++){

		allDeviceData[i].deviceID = deviceIds[i];
		deviceSeparateLowBatteryStatuses[i] = (deviceLowBatteryStatuses[0]>>i)&(0x01);
		printf("\r\nDevice %ld low battery status: %ld", (uint32_t)allDeviceData[i].deviceID, (uint32_t)deviceSeparateLowBatteryStatuses[i]);

	}

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);
	for(int i = 0; i<deviceNum[0];i++){

		printf("\r\nrequesting from: %d", allDeviceData[i].deviceID);
		requestRFData(handles, &(allDeviceData[i]), rfData);

		if(!allDeviceData[i].deviceStatus){
			printf("\r\ndevice %d not found", allDeviceData[i].deviceID);
			lostDevices[lostDevicesNum] = allDeviceData[i].deviceID;
			lostDevicesNum++;
		}

		if(allDeviceData[i].measurement.batteryStatus == 0){
			if(!deviceSeparateLowBatteryStatuses[i]){

				allDeviceData[i].measurement.batteryStatus = 2;
				deviceSeparateLowBatteryStatuses[i] = 1;
			}
		}
	}

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf activation pin

	HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);
	addDeviceData(handles, allDeviceData, time, deviceNum[0]);

	HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_RESET);

	removeDevices(lostDevices, lostDevicesNum);

	for(int i = 0;i<deviceNum[0];i++){
		deviceLowBatteryStatuses[0] = deviceLowBatteryStatuses[0] | ((deviceSeparateLowBatteryStatuses[i]&0x01)<<i);
	}
	Flash_Write_Data(changingDeviceDataStart, deviceLowBatteryStatuses, 1);

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);

	HAL_Delay(1000);
	scanConfirmation(handles, rfData);

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf activation pin

}

void scanConfirmation(struct CommunicationHandles* handles, struct rfDataStorage* rfData){

	printf("Confirming Scan");
	uint64_t deviceNum[1];
	Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);

	uint64_t deviceIds[deviceNum[0]+1];
	Flash_Read_Data(deviceIDListStartAddress, deviceIds, deviceNum[0]);
	rfData->activeTxMessageSize = 3;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_DONE;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	printf("\r\nconfirming scan for %ld devices", (uint32_t)deviceNum[0]);
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);

	rfData->activeTxMessage[MSG_BODY_START] = sTime.Hours;
	rfData->activeTxMessage[MSG_BODY_START+1] = sTime.Minutes;
	rfData->activeTxMessage[MSG_BODY_START+2] = sTime.Seconds;

	rfData->activeTxMessage[MSG_TO_ID_POS] = 0;

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+6);
	HAL_Delay(2000);
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+6);
}

void checkForNewDevices(struct CommunicationHandles* handles, struct rfDataStorage* rfData){
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);
	int listeningCounter = 50;

	printf("\r\Checking for new devices");
	for(int i = 0;i<listeningCounter;i++){

		Comms_Handler(rfData);
		for(int i = 0; i<rfData->messageNum; i++){
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_IM_HERE){
				printf("\r\New device found", rfData->activeRxMessage[i][MSG_FROM_ID_POS]);
				acknowledgeDevice(rfData->activeRxMessage[i][MSG_FROM_ID_POS], handles, rfData);
				rfData->newDevices.newDevices[rfData->newDevices.numOfNewDevices] = rfData->activeRxMessage[i][MSG_FROM_ID_POS];
				rfData->newDevices.numOfNewDevices++;

			}
		}
		HAL_Delay(100);

	}
	printf("\r\Check done");
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf activation pin

}

void acknowledgeDevice(int deviceNumber, struct CommunicationHandles* handles, struct rfDataStorage* rfData){

	rfData->activeTxMessageSize = 3;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_YOUR_THERE;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_TO_ID_POS] = deviceNumber;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);

	rfData->activeTxMessage[MSG_BODY_START] = sTime.Hours;
	rfData->activeTxMessage[MSG_BODY_START+1] = sTime.Minutes;
	rfData->activeTxMessage[MSG_BODY_START+2] = sTime.Seconds;

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+6);

}

void removeDevices(uint8_t deviceToRemove[], uint8_t numOfDeviceToRemove){

	if(numOfDeviceToRemove>0){

		uint64_t deviceNum[1];
		Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);

		uint64_t allDeviceData[deviceNum[0]+2];

		uint64_t deviceIds[deviceNum[0]+1];
		Flash_Read_Data(deviceIDListStartAddress, deviceIds, deviceNum[0]);

		for(int i = 0;i<deviceNum[0]; i++){
			int deviceCanBeKept = 1;
			for(int n = 0;n<numOfDeviceToRemove;n++){

				if(deviceIds[i] == deviceToRemove[n]){

					deviceCanBeKept = 0;

				}

			}

			if(deviceCanBeKept){

				allDeviceData[i+1] = deviceIds[i];

			}
		}

		deviceNum[0]-=numOfDeviceToRemove;

		allDeviceData[0] = deviceNum[0];

		Flash_Write_Data(deviceDataStartAddr, allDeviceData, deviceNum[0]+1);

	}

}

void addNewDevices(struct rfDataStorage* rfData){

	if(rfData->newDevices.numOfNewDevices>0){

		printf("\r\nAdding new devices");
		uint64_t deviceNum[1];
		Flash_Read_Data(numberOfDevicesAddress, deviceNum, 1);

		uint64_t allDeviceData[deviceNum[0]+rfData->newDevices.numOfNewDevices+1];

		uint64_t deviceIds[deviceNum[0]+1];

		Flash_Read_Data(deviceIDListStartAddress, deviceIds, deviceNum[0]);
		for(int i = 0;i<rfData->newDevices.numOfNewDevices; i++){
			deviceIds[deviceNum[0]+i] = rfData->newDevices.newDevices[i];
		}

		for(int i = 0;i<deviceNum[0];i++){

			allDeviceData[i+1] = deviceIds[i];

		}

		for(int i = 0;i<rfData->newDevices.numOfNewDevices; i++){
			printf("\r\nAdding device %d to flash", rfData->newDevices.newDevices[i]);
			allDeviceData[deviceNum[0]+i+1] = rfData->newDevices.newDevices[i];
		}

		deviceNum[0]+=rfData->newDevices.numOfNewDevices;

		allDeviceData[0] = deviceNum[0];

		Flash_Write_Data(deviceDataStartAddr, allDeviceData, deviceNum[0]+1);

		HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_SET);
		HAL_Delay(100);

		struct SDinfo config;

			FATFS fs; 	//Fatfs handle
			FIL fil; 		//File handle
			FRESULT fres;

			config.fs = &fs;
			config.fil = &fil;
			config.fres = &fres;

			if(stopCard(&config)){
				printf("\r\ncard unmounted");
			}
			else{

				printf("\r\nfail to unmount card");
			}

			if(setupCard(&config)){
				printf("\r\ncard mounted");

				char buffer[50];


				for(int i = 0;i<rfData->newDevices.numOfNewDevices; i++){
					printf("saving initial data for %d device", rfData->newDevices.newDevices[i]);
					sprintf(buffer, "Device_%d\n", (uint16_t)rfData->newDevices.newDevices[i]);
					sendSD(&config, "AllDeviceInfo.txt", buffer);
					f_mkdir(buffer);
				}

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

		HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_RESET);
	}
}

void setupSDcard(){
	HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	struct SDinfo config;

		FATFS fs; 	//Fatfs handle
		FIL fil; 		//File handle
		FRESULT fres;

		config.fs = &fs;
		config.fil = &fil;
		config.fres = &fres;

		if(stopCard(&config)){
			printf("\r\ncard unmounted");
		}
		else{

			printf("\r\nfail to unmount card");
		}

		if(setupCard(&config)){
			printf("\r\ncard mounted");

			char buffer[50];

			*config.fres = f_open(config.fil, "BrokenDevices.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			*config.fres = f_close(config.fil);

			*config.fres = f_open(config.fil, "LowBatteryDevices.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			*config.fres = f_close(config.fil);


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

	HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_RESET);
}
void storeDeviceInitialData(uint64_t deviceIds[], uint8_t deviceNumber){

	struct SDinfo config;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	config.fs = &fs;
	config.fil = &fil;
	config.fres = &fres;

	if(stopCard(&config)){
		printf("\r\ncard unmounted");
	}
	else{

		printf("\r\nfail to unmount card");
	}

	if(setupCard(&config)){
		printf("\r\ncard mounted");

		char buffer[50];
		printf("\r\nsaving initial data for %d device", deviceNumber);

		for(int i = 0;i<deviceNumber;i++){

			sprintf(buffer, "Device_%ld\n", (uint32_t)deviceIds[i]);
			sendSD(&config, "AllDeviceInfo.txt", buffer);
			f_mkdir(buffer);
		}

		*config.fres = f_open(config.fil, "BrokenDevices.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		*config.fres = f_close(config.fil);

		*config.fres = f_open(config.fil, "LowBatteryDevices.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		*config.fres = f_close(config.fil);


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
}

void addBrokenDevice(struct SDinfo* config, uint16_t deviceID){

	char buf[10];
	printf("\r\nAddign broken device");
	sprintf(buf, "Device_%d\n", deviceID);
	sendSD(config, "BrokenDevices.txt", buf);

}

void addLowBatteryDevice(struct SDinfo* config, uint16_t deviceID){

	char buf[10];
	printf("\r\nAdding low battery device");
	sprintf(buf, "Device_%d\n", deviceID);
	sendSD(config, "LowBatteryDevices.txt", buf);

}

void getDevices(struct SDinfo* config, char* listFile, uint16_t* data, uint8_t* num){

	char buf[1000];
	printf("\r\ngetting SD data");
	getSD(config, listFile, buf);
	printf("\r\nretrieved data: %s", buf);
	int iterator = 0;
	for(int i = 0; i<strlen(buf);i++){
		printf("\r\nCurrent data: %c", buf[i]);
		if(buf[i]=='_'){
			int numberIterator = i+1;

			uint16_t gatheredNum = 0;
			printf("\r\nnumber started");
			int gettingNumber = 1;
			while(gettingNumber){
				printf("\r\nchecking data: %c", buf[numberIterator]);
				if(isdigit(buf[numberIterator])){
					printf("\r\nadding number %c to %d", buf[i], gatheredNum);
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

void checkForRemote(struct rfDataStorage* rfData){

	uint64_t managementBuf[managementDataNum];
	Flash_Read_Data(managementDataAddr, managementBuf, managementDataNum);

	init_comms(rfData, DEVICE_PARENT, managementBuf[0], 0);

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	for(int i = 0;i<10;i++){

		Comms_Handler(rfData);
		for(int i = 0; i<rfData->messageNum; i++){
			//add device
			printf("\r\nReceived message type: %x", rfData->activeRxMessage[i][MSG_TYPE_POS]);
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_GIVE_STATUS_DATA){
				printf("\r\nsending data to remote");
				sentDataToRemote(rfData, rfData->activeRxMessage[i][MSG_FROM_ID_POS]);
				break;
			}
		}
		HAL_Delay(1000);
	}

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);

}

void sentDataToRemote(struct rfDataStorage* rfData, int remoteId){

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

	HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	if(setupCard(&sdData)){

		getDevices(&sdData, "BrokenDevices.txt",  brokenDevices, &brokenNum);
		getDevices(&sdData, "LowBatteryDevices.txt",  lowBatteryDevices, &lowBatteryNum);
		if(!stopCard(&sdData)){

			printf("\r\nSD stop error");

		}

	}
	else{

		printf("\r\nSD start error");

	}

	HAL_GPIO_WritePin(SDcardGPIO, SDcardGPIOPin, GPIO_PIN_RESET);

	printf("\r\nbroken devices: %d", brokenNum);
	for(int i = 0;i<brokenNum;i++){

		printf("\r\n%d", brokenDevices[i]);

	}
	printf("\r\nlow battery devices: %d", lowBatteryNum);
	for(int i = 0;i<lowBatteryNum;i++){

		printf("\r\n%d", lowBatteryDevices[i]);

	}

	//broken devices
	rfData->activeTxMessageSize = brokenNum;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_BROKEN_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS] = remoteId;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	int iterator = 0;

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	for(int i = 0;i<brokenNum;i++){

		rfData->activeTxMessage[iterator+MSG_BODY_START] = brokenDevices[i];
		if(iterator==8){
			rfData->activeTxMessageSize = iterator;
			rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
			Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
			iterator = 0;
			HAL_Delay(300);
		}
		else{
			iterator++;
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
	rfData->activeTxMessage[MSG_TO_ID_POS] = remoteId;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	iterator = 0;
	for(int i = 0;i<lowBatteryNum;i++){

		rfData->activeTxMessage[iterator+MSG_BODY_START] = lowBatteryDevices[i];
		if(iterator==8){
			rfData->activeTxMessageSize = iterator;
			rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
			Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
			HAL_Delay(300);
		}
		else{
		    iterator++;
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
	rfData->activeTxMessage[MSG_TO_ID_POS] = remoteId;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
	HAL_Delay(3000);
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);
}

void memorySetup(){

	//printf("setting up memory");
	uint64_t setupData[managementDataNum];
	Flash_Read_Data(managementDataAddr, setupData, managementDataNum);
	//printf("read memory");
	setupData[1] = 1;

	//printf("writting memory");
	Flash_Write_Data(managementDataAddr, setupData, managementDataNum);
	//printf("memory setup done");

	uint64_t deviceNum[] = {0};
	Flash_Write_Data(numberOfDevicesAddress, deviceNum, 1);

	uint64_t statusData[1];
	statusData[0] = 0;
	printf("\r\nsetting up statuses");
	Flash_Write_Data(changingDeviceDataStart, statusData, 1);
}

int mainSetupMother(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	printf("\r\nSetting up");
	if(IDhandling()){
		return 0;
	}

	if(remoteAccessHandling(rfData)){
		printf("\r\nhandling remote");
		setShutdownModeMother();

	}

	uint64_t setupBit[1];
	Flash_Read_Data(setupAddr, setupBit, 1);

	if(setupBit[0]!=1){
		printf("\r\nperforming setup");
		//synchroniationSetup(rfData, handles);

		//uint64_t deviceNumber;
		//Flash_Read_Data(numberOfDevicesAddress, &deviceNumber, 1);

		//uint64_t deviceIds[deviceNumber+1];
		//Flash_Read_Data(deviceIDListStartAddress, deviceIds, deviceNumber);

		setupSDcard();

		memorySetup();
		printf("\r\nSetup done");
		setShutdownModeMother(handles->RTChandle);

	}

	return 1;

}

int remoteAccessHandling(struct rfDataStorage* rfData){

	int remoteListening = 0;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
	if(HAL_GPIO_ReadPin(remoteGPIO, remoteGPIOPin)== GPIO_PIN_SET)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		remoteListening = 1;
		printf("\r\nchecking remote");
		checkForRemote(rfData);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	}

	return remoteListening;

}

int IDhandling(){

	printf("\r\nChecking ID set pin : %d ", HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin));

	if(HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin)== GPIO_PIN_SET)
	{
		uint64_t managementBuf[managementDataNum];
		uint64_t rxBuf[2];
		uint32_t buf;

		printf("\r\nSet device ID");

		scanf("%ld", &buf);
		printf("\r\nData to write: %ld", (uint32_t)buf);
		managementBuf[0] = 70;

		managementBuf[1] = 0;

		if(Flash_Write_Data(managementDataAddr, managementBuf, managementDataNum) == HAL_OK){

			Flash_Read_Data(managementDataAddr, rxBuf, managementDataNum);
			printf("\r\nID set to %ld", (uint32_t)rxBuf[0]);
			printf("\r\nSetup bit set to: %ld", (uint32_t)rxBuf[1]);
		}
		uint64_t deviceBuf[1] = {0};
		Flash_Write_Data(numberOfDevicesAddress, deviceBuf, 1);
		Flash_Read_Data(numberOfDevicesAddress, rxBuf, 1);
		printf("\r\nSet device num to %ld", (uint32_t)rxBuf[0]);
		return 1;
	}

	return 0;
}

void setShutdownModeMother(){

	HAL_SuspendTick();
	printf("\r\nEntering Shutdown Mode");
	HAL_PWREx_EnterSHUTDOWNMode();

}

void mainOperationMother(struct rfDataStorage* rfData, struct CommunicationHandles* handles)
{

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);
	printf("\r\nOperating at %02d::%02d::%02d",  sTime.Hours, sTime.Minutes, sTime.Seconds);

	struct timeData timeInfo;
	timeInfo.seconds = sTime.Seconds;
	timeInfo.minutes = sTime.Minutes;
	timeInfo.hours = sTime.Hours;
	timeInfo.day = sDate.Date;
	timeInfo.month = sDate.Month;
	timeInfo.year = sDate.Year;

	struct newDevicesStorage newDevices;
	newDevices.numOfNewDevices = 0;
	rfData->newDevices = newDevices;
	HAL_Delay(5000);

	uint64_t idBuf[1];
	Flash_Read_Data(IDaddressAddr, idBuf, 1);

	init_comms(rfData, DEVICE_PARENT, idBuf[0], 0);

	rfData->count = 0;
	rfData->num_fwds = 0;

	checkForNewDevices(handles, rfData);
	scanDeviceForData(rfData, handles, &timeInfo);
	addNewDevices(rfData);

	setShutdownModeMother();
}

#endif /* SRC_MANAGEMENT_C_ */
