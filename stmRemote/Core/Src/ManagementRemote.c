/*
 * Management.c
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#ifndef SRC_MANAGEMENT_C_
#define SRC_MANAGEMENT_C_

#include <ManagementRemote.h>
#include <retarget.h>
#include <ctype.h>

void addDeviceData(struct CommunicationHandles* handles, struct deviceData* devices, int numberOfDevices){

	struct SDinfo sdData;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;

	if(setupCard(&sdData)){
		for(int i = 0;i<numberOfDevices;i++){

			//if(devices[i].deviceStatus==1){
				char buf[50];

				char folderName[40];

				char fileName[100];

				sprintf(folderName, "Device_%d", devices[i].deviceID);

				//for(int n = 0; n<NUM_READINGS_MAX;n++){
					sprintf(fileName, "%s/Time.txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", devices[i].measurement.depthValue);
					sprintf(fileName, "%s/DepthChange(cm_x_10).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", devices[i].measurement.waterPressureValue);
					sprintf(fileName, "%s/PressureValue(Pa).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", devices[i].measurement.undergroundTemperatureValue);
					sprintf(fileName, "%s/UndergroundTemperature(deg_x_100).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", devices[i].measurement.surfaceTemperatureValue);
					sprintf(fileName, "%s/SurfaceTemperature(deg_x_100).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", devices[i].measurement.batteryStatus);
					sprintf(fileName, "%s/BatteryStatus.txt", folderName);
					sendSD(&sdData, fileName, buf);

					if(!devices[i].measurement.batteryStatus){
						printf("\r\nadding low battery Device");
						addLowBatteryDevice(&sdData, devices[i].deviceID);

					}
					if(devices[i].measurement.batteryStatus == 2) devices[i].measurement.batteryStatus = 0;
					sprintf(buf, "%d\n", devices[i].measurement.batteryStatus);
					sprintf(fileName, "%s/BatteryStatus.txt", folderName);
					sendSD(&sdData, fileName, buf);
				//}
			/*}
			else if(devices[i].deviceStatus==0){
				printf("\r\nadding Broken Device");
				addBrokenDevice(&sdData, devices[i].deviceID);
			}*/
		}
		if(!stopCard(&sdData)){

			printf("\r\nSD stop error");

		}

	}
	else{

		printf("\r\nSD start error");

	}
}

void requestMeasureData(struct deviceData* device, int* deviceNum, struct rfDataStorage* rfData){

	rfData->activeTxMessageSize = 0;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_MEASUREMENT_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS] = 0;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+6);

	int requestSuccessful = false;
	while(!requestSuccessful){
		printf("Scanning");

		Comms_Handler(rfData);

		for(int i = 0; i<rfData->messageNum; i++){
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_REMOTE_DONE){
				printf("got data request reply from: %d", device->deviceID);
				requestSuccessful = true;

			}
			else if(rfData->activeRxMessage[i][MSG_TO_ID_POS] == rfData->myDevice.device_id){

				printf("decoding data");
				uint8_t allData[rfData->activeRxMessage[i][MSG_SIZE_POS]];
				for(int i = 0;i<rfData->activeRxMessage[i][MSG_SIZE_POS];i++){

						allData[i] = rfData->activeRxMessage[i][i+MSG_BODY_START];

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
		HAL_Delay(1000);

	}

}

int requestStatusData(struct statusData* statusInfo, struct rfDataStorage* rfData){

	rfData->activeTxMessageSize = 0;
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_GIVE_STATUS_DATA;
	rfData->activeTxMessage[MSG_TO_ID_POS] = 70;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	int requestSuccessful = false;

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+6);
	int listeningCounter = 200;
	while(!requestSuccessful){
		printf("\r\nScanning");
		Comms_Handler(rfData);

		for(int i = 0; i<rfData->messageNum; i++){
			//printf("last id: %d -> comparing to: %d", rfData->activeRxMessage[MSG_TO_ID_POS], rfData->myDevice.device_id);
			printf("last message type: %d", rfData->activeRxMessage[i][MSG_TYPE_POS]);
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_REMOTE_DONE){
				printf("\r\ngot data request reply from: %d", rfData->activeRxMessage[i][MSG_FROM_ID_POS]);
				requestSuccessful = true;

			}
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_GIVE_BROKEN_DATA){
				printf("\r\nbroken data received: %d->", rfData->activeRxMessage[i][MSG_SIZE_POS], statusInfo->brokenNum);
				for(int n = 0;n<rfData->activeRxMessage[i][MSG_SIZE_POS];n++){

					statusInfo->brokenDevices[statusInfo->brokenNum] = rfData->activeRxMessage[i][MSG_BODY_START+n];
					statusInfo->brokenNum++;
				}
				printf("\r\nbroken data received: %d->", rfData->activeRxMessage[i][MSG_SIZE_POS], statusInfo->brokenNum);

			}
			else if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_GIVE_LOW_BATTERY_DATA){
				printf("\r\nbattery data received: %d->", rfData->activeRxMessage[i][MSG_SIZE_POS], statusInfo->lowBatteryNum);
				for(int n = 0;n<rfData->activeRxMessage[i][MSG_SIZE_POS];n++){

					statusInfo->lowBatteryDevices[statusInfo->lowBatteryNum] = rfData->activeRxMessage[i][MSG_BODY_START+n];
					statusInfo->lowBatteryNum++;
				}
				printf("\r\nbattery data received: %d->", rfData->activeRxMessage[i][MSG_SIZE_POS], statusInfo->lowBatteryNum);
			}

			else{
				listeningCounter--;
				if(listeningCounter<=0){
					break;
				}
			}
		}
		HAL_Delay(100);

	}
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);
	return requestSuccessful;
}

int mainSetupRemote(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	printf("\r\nSetting up");
	if(IDhandling()){
		return 0;
	}
	uint64_t rxBuf[1] = {0};
	Flash_Read_Data(managementDataAddr, rxBuf, 1);
	printf("\r\nRetrieved ID: %lx from %lx", (uint32_t)rxBuf[0], managementDataAddr);
	init_comms(rfData, DEVICE_PARENT, rxBuf[0], 1);

	return 1;

}

int IDhandling(){

	printf("\r\nChecking ID set pin : %d ", HAL_GPIO_ReadPin(statusGPIO, statusGPIOPin));

	if(HAL_GPIO_ReadPin(statusGPIO, statusGPIOPin)== GPIO_PIN_SET)
	{
		uint64_t managementBuf[managementDataNum];
		uint64_t rxBuf[1];
		uint32_t buf;

		printf("\r\nSet device ID");

		scanf("%ld", &buf);
		printf("\r\nData to write: %ld", (uint32_t)buf);
		managementBuf[0] = buf;

		if(Flash_Write_Data(managementDataAddr, managementBuf, managementDataNum) == HAL_OK){

			Flash_Read_Data(managementDataAddr, rxBuf, 1);
			printf("\r\nID set to %ld", (uint32_t)rxBuf[0]);
		}
		return 1;
	}

	return 0;
}

void mainOperationRemote(struct rfDataStorage* rfData, struct CommunicationHandles* handles)
{
	printf("\r\nRemote Operation");
	if(HAL_GPIO_ReadPin(statusGPIO, statusGPIOPin)== GPIO_PIN_SET)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		struct statusData statusInfo;
		statusInfo.brokenNum = 0;
		statusInfo.lowBatteryNum = 0;
		if(requestStatusData(&statusInfo, rfData)){

			printf("\r\nBroken Devices:");
			for(int i = 0;i<statusInfo.brokenNum;i++){

				printf("\r\n%d", statusInfo.brokenDevices[i]);

			}

			printf("Low Battery Devices:");
			for(int i = 0;i<statusInfo.lowBatteryNum;i++){

				printf("\r\n%d", statusInfo.lowBatteryDevices[i]);

			}
		}
		else{
			printf("\r\nNo reply received;");
		}
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	}
	else if(HAL_GPIO_ReadPin(dataGPIO, dataGPIOPin)== GPIO_PIN_SET)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
		int deviceNum = 0;
		struct deviceData deviceBuf[100];
		requestMeasureData(deviceBuf, &deviceNum, rfData);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
	}

}

#endif /* SRC_MANAGEMENT_C_ */
