/*
 * Management.c
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#ifndef SRC_MANAGEMENT_C_
#define SRC_MANAGEMENT_C_

#include <ManagementChild.h>
#include "../Inc/FlashMemoryAccess.h"
#include "main.h"

void performDepthMeasurement(ADC_HandleTypeDef*  handle1, ADC_HandleTypeDef*  handle2, struct measurementData* data){

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_5;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	  sConfig.SingleDiff = ADC_SINGLE_ENDED;
	  sConfig.OffsetNumber = ADC_OFFSET_NONE;
	  sConfig.Offset = 0;
	  if (HAL_ADC_ConfigChannel(handle1, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	uint64_t supportVariables[numberOfDepthVariables];
	Flash_Read_Data(depthVariablesStartAddr, supportVariables, numberOfDepthVariables);

	//printf("\r\nDoing depth Measurement");

	struct DepthData depthData;

	depthData.pos = ((int)supportVariables[0]);
	depthData.output1 = ((int)supportVariables[1]);
	depthData.output2 = ((int)supportVariables[2]);
	depthData.max_value_1 = ((int)supportVariables[3]);
	depthData.min_value_1 = ((int)supportVariables[4]);
	depthData.max_value_2 = ((int)supportVariables[5]);
	depthData.min_value_2 = ((int)supportVariables[6]);

	//printf("\r\nstored data: %d, %d, %d, %d, %d, %d, %d", depthData.pos, depthData.output1, depthData.output2, depthData.max_value_1, depthData.min_value_1, depthData.max_value_2, depthData.min_value_2);
	HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_SET); // depth activation pin
	HAL_Delay(100);

	calculateDistance(handle1, handle2, &depthData);

	HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_RESET); // depth

	data->depthValue = 100*(((float)depthData.pos)/35);

	printf("\r\nCalculated distance: %d", data->depthValue);
	supportVariables[0] = (uint64_t)(depthData.pos);
	supportVariables[1] = (uint64_t)(depthData.output1);
	supportVariables[2] = (uint64_t)(depthData.output2);
	supportVariables[3] = (uint64_t)(depthData.max_value_1);
	supportVariables[4] = (uint64_t)(depthData.min_value_1);
	supportVariables[5] = (uint64_t)(depthData.max_value_2);
	supportVariables[6] = (uint64_t)(depthData.min_value_2);

	//printf("\r\new data: %d, %d, %d, %d, %d, %d, %d", depthData.pos, depthData.output1, depthData.output2, depthData.max_value_1, depthData.min_value_1, depthData.max_value_2, depthData.min_value_2);

	Flash_Write_Data(depthVariablesStartAddr, supportVariables, numberOfDepthVariables);

}

void performPressureMeasurement(I2C_HandleTypeDef* handle, struct measurementData* data){

	HAL_GPIO_WritePin(i2cGPIO, i2cGPIOPin, GPIO_PIN_SET); //pressure activation pin
	HAL_Delay(100);

	lps28dfw_info_t sensorInfo1;
	sensorInfo1.handle = handle;

	if(beginLPS28(&sensorInfo1, LPS28DFW_I2C_ADDRESS_DEFAULT) == LPS28DFW_OK)
	{

		lps28dfw_md_t modeConfig =
		{
			.fs  = fs_LPS28DFW_1260hPa,      // Full scale range
			.odr = odr_LPS28DFW_4Hz,         // Output data rate
			.avg = avg_LPS28DFW_512_AVG,      // Average filter
			.lpf = lpf_LPS28DFW_LPF_ODR_DIV_9 // Low-pass filter
		};

		setModeConfig(&sensorInfo1, &modeConfig);
		HAL_Delay(100);

		double runnningValue = 0;
		for(int i = 0;i<4;i++){
			getSensorData(&sensorInfo1);
			runnningValue += sensorInfo1.data.pressure.hpa;
			HAL_Delay(250);
		}
		runnningValue = runnningValue/4;

		data->undergroundTemperatureValue = (uint16_t)(sensorInfo1.data.heat.deg_c*100);

		data->waterPressureValue = (uint32_t)(runnningValue*100);

		printf("\r\n Measured pressure: %d", data->waterPressureValue);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	}
	else{

		printf("\r\nCould not connect to pressure sensor");

	}

	HAL_GPIO_WritePin(i2cGPIO, i2cGPIOPin, GPIO_PIN_RESET);

}

void performTemperatureMeasurement(SPI_HandleTypeDef* handle, struct measurementData* data){

	HAL_GPIO_WritePin(spiGPIO, spiGPIOPin, GPIO_PIN_SET); //temperature activation pin
	HAL_Delay(100);

	configureSensor(handle);

	double runnningValue = 0;
	for(int i = 0;i<4;i++){
		runnningValue += getTemperatureVal(handle);
		HAL_Delay(100);
	}
	runnningValue = runnningValue/4;

	data->surfaceTemperatureValue = (uint16_t)(runnningValue*100);

	printf("\r\nMeasured temperature: %f", getTemperatureVal(handle));

	HAL_GPIO_WritePin(spiGPIO, spiGPIOPin, GPIO_PIN_RESET);
}

void checkBattery(ADC_HandleTypeDef* adcHandle1, struct measurementData* data){

	printf("\r\nChecking battery");

	//HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET); //rf activation pin
	HAL_Delay(100);

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_6;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	  sConfig.SingleDiff = ADC_SINGLE_ENDED;
	  sConfig.OffsetNumber = ADC_OFFSET_NONE;
	  sConfig.Offset = 0;
	  if (HAL_ADC_ConfigChannel(adcHandle1, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	HAL_ADC_Start(adcHandle1);
	 HAL_ADC_PollForConversion(adcHandle1, 0xFFFF);
	 uint16_t ADC_value = HAL_ADC_GetValue(adcHandle1);
	 float voltage = ((ADC_value /4096.0) * Vref);
	 printf("ADC=%hu, Voltage= %.2f\r\n", ADC_value, voltage);
	 HAL_Delay(100);

	 /*when first battery run out, the back-up battery will passively turned on*/
	 /*higher voltage will be detected from the second battery*/
	 /*if the ADC pin detects a higher voltage turn on at second battery terminal*/
	 /*comm pin*/

	 //HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);

	if (voltage > VOLTAGE_THRESHOLD){
		 printf("First battery dead");
		 data->batteryStatus = 0;
	}
	 else{
		 printf("first battery ok");
	 }

	if(data->batteryStatus==0){

	}

}


void setRfListening(struct measurementData* data, struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET); //rf activation pin
	HAL_Delay(100);

	uint64_t managementBuf[managementDataNum];
	Flash_Read_Data(managementDataAddr, managementBuf, managementDataNum);

	init_comms(rfData, DEVICE_CHILD, managementBuf[0], managementBuf[3]);

	rfData->activeTxMessage[MSG_TO_ID_POS] = managementBuf[1];

	printf("\r\nmeasurement data to send: %d, %d, %d, %d", data->depthValue, data->waterPressureValue, data->surfaceTemperatureValue, data->undergroundTemperatureValue, data->batteryStatus);
	uint8_t encodedDataSize = 0;
	uint8_t encodedData[20];
	encodeMeasurementData(data, encodedData, &encodedDataSize);

	for(int i = 0;i<encodedDataSize;i++){

		rfData->activeTxMessage[MSG_BODY_START+i] = encodedData[i];

	}

	rfData->activeTxMessageSize = encodedDataSize;

	int listening = 1;
	int listeningCounter = 100;
	while(listening){

		Comms_Handler(rfData);

		for(int i = 0; i<rfData->messageNum; i++){
			printf("\r\nmessage %d type: %x", i, rfData->activeRxMessage[i][MSG_TYPE_POS]);

			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_DONE){

				printf("scan done, received time: %d %d %d", rfData->activeRxMessage[i][MSG_BODY_START], rfData->activeRxMessage[i][MSG_BODY_START+1], rfData->activeRxMessage[i][MSG_BODY_START+2]);

				RTC_TimeTypeDef sTime;

				sTime.Hours = rfData->activeRxMessage[i][MSG_BODY_START];
				sTime.Minutes = rfData->activeRxMessage[i][MSG_BODY_START+1];
				sTime.Seconds = rfData->activeRxMessage[i][MSG_BODY_START+2];

				if (HAL_RTC_SetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN) != HAL_OK)
				{
					//printf("Could not set time");
					Error_Handler();
				}

				HAL_Delay(1000);
				Send(rfData->activeRxMessage, rfData->activeRxMessage[i][MSG_SIZE_POS]+MSG_BODY_START);

				listening = 0;
				break;
			}
			if (rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_GIVE_DATA)
			{
				Send_Data(rfData);
			}

		}

		if(listeningCounter<=0){

			listening = 0;

		}
		else{
			listeningCounter--;
			HAL_Delay(1000);
		}

	}

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);
}

void powerDownDevices(){

	HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_RESET); //depth
	HAL_GPIO_WritePin(i2cGPIO, i2cGPIOPin, GPIO_PIN_RESET); //pressure
	HAL_GPIO_WritePin(spiGPIO, spiGPIOPin, GPIO_PIN_RESET); //temperature
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf

}


void synchronizationSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	//printf("\r\nSynchro Setup");

	RTC_TimeTypeDef sTime;

	uint64_t managementBuf[managementDataNum];
	Flash_Read_Data(managementDataAddr, managementBuf, managementDataNum);

	init_comms(rfData, DEVICE_CHILD, managementBuf[0], 0);

	rfData->activeTxMessage[MSG_TO_ID_POS] = managementBuf[1];
	rfData->count = 0;
	rfData->num_fwds = 0;

	int listening = 1;

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET); //rf activation pin
	HAL_Delay(100);

	while(listening){
		printf("\r\nsignaling from id: %d", rfData->myDevice.device_id);
		Comms_Handler(rfData);

		for(int i = 0; i<rfData->messageNum; i++){
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_YOUR_THERE)
			{
				if(rfData->myDevice.device_id == rfData->activeRxMessage[i][MSG_TO_ID_POS])
				{
					rfData->myDevice.device_is_discovered = true;
					sTime.Hours = rfData->activeRxMessage[i][MSG_BODY_START];
					sTime.Minutes = rfData->activeRxMessage[i][MSG_BODY_START+1];
					sTime.Seconds = rfData->activeRxMessage[i][MSG_BODY_START+2];

					printf("Device discovered, time set to: %d::%d::%d", sTime.Hours, sTime.Minutes, sTime.Seconds);

					if (HAL_RTC_SetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN) != HAL_OK)
					{
						printf("Error setting time");
						Error_Handler();
					}

					if(!managementBuf[3]){
						managementBuf[1] = rfData->activeRxMessage[i][MSG_FROM_ID_POS];
						managementBuf[3] = 1; //discovered

					}

					listening = false;
					break;
				}

			}
			printf("message %d type: %x", i, rfData->activeRxMessage[i][MSG_TYPE_POS]);
		}
		HAL_Delay(100);
	}

	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);

	Flash_Write_Data(managementDataAddr, managementBuf, managementDataNum);

}



void memorySetup(){

	uint64_t setupData[managementDataNum];
	Flash_Read_Data(managementDataAddr, setupData, managementDataNum);

	setupData[2] = 1;

	Flash_Write_Data(managementDataAddr, setupData, managementDataNum);

	//uint64_t supportVariables[numberOfMeasurementVariables];
	//supportVariables[0] = 0;
	//Flash_Write_Data(measurementVariablesStartAddr, supportVariables, 2);

	uint64_t depthVariables[numberOfDepthVariables] = {0,0,0,0,0,0,0};
	Flash_Write_Data(depthVariablesStartAddr, depthVariables, numberOfDepthVariables);

	//uint64_t supportVariablesRx[2];
	//Flash_Read_Data(measurementVariablesStartAddr, supportVariablesRx, 2);

}

int mainSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	if(IDhandling()){
		return 0;
	}

	powerDownDevices();

	uint64_t setupBit[1];
	Flash_Read_Data(setupAddr, setupBit, 1);

	if(setupBit[0]!=1){

		printf("\r\nMain setup\n");

		synchronizationSetup(rfData, handles);

		printf("\r\nSynchro Done\n");
		memorySetup();

		printf("\r\nSetup Done\n");
		setShutdownMode(handles);
	}
	return 1;
}

int IDhandling(){

	printf("\r\nChecking ID set pin : %d ", HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin));

	if(HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin) == GPIO_PIN_SET)
	{
		uint64_t managementBuf[managementDataNum] = {15, 70, 0, 0, 0};
		uint64_t rxBuf[2];
		int buf;

		printf("\r\nSet device ID");

		setvbuf( stdin, NULL, _IONBF, 0 );
		scanf("%d", &buf);
		printf("\r\nData to write: %ld", (uint32_t)buf);
		managementBuf[0] = 15;//buf;

		printf("\r\nSet mother device ID");

		scanf("%d", &buf);
		printf("\r\nMother Data to write: %ld", (uint32_t)buf);
		managementBuf[1] = 70;//;

		for(int i = 2;i<managementDataNum;i++){

			managementBuf[i] = 0;

		}

		if(Flash_Write_Data(IDaddressAddr, managementBuf, managementDataNum) == HAL_OK){

			Flash_Read_Data(IDaddressAddr, rxBuf, 2);
			printf("\r\nID set to %ld", (uint32_t)managementBuf[0]);
			printf("\r\nMother ID set to %ld", (uint32_t)managementBuf[1]);

		}

		printf("\r\nSetup bit set to zero");

		return 1;
	}

	return 0;
}

void setShutdownMode(){

	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WRFBUSY);
	HAL_SuspendTick();
	powerDownDevices();
	printf("\r\nEntering Shutdown Mode");
	HAL_PWREx_EnterSHUTDOWNMode();

}

void addDeviceData(struct CommunicationHandles* handles, struct measurementData* data){

	struct SDinfo sdData;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;
	HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
	HAL_Delay(10);
	if(setupCard(&sdData)){

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		char buf[50];

		char fileName[100];

		RTC_TimeTypeDef sTime;
		RTC_DateTypeDef sDate;
		HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);

		sprintf(buf, "%d::%d::%d %d/%d/%d\n", sTime.Hours, sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month, sDate.Year);
		sprintf(fileName, "%s", "Time.txt");
		sendSD(&sdData, fileName, buf);
		HAL_Delay(100);
		sprintf(buf, "%d\n", data->depthValue);
		sprintf(fileName, "%s", "DepthChange(cm_x_100).txt");
		sendSD(&sdData, fileName, buf);
		HAL_Delay(100);
		sprintf(buf, "%d\n", data->waterPressureValue);
		sprintf(fileName, "%s", "PressureValue(Pa).txt");
		sendSD(&sdData, fileName, buf);
		HAL_Delay(100);
		sprintf(buf, "%d\n", data->surfaceTemperatureValue);
		sprintf(fileName, "%s", "Temperature(deg_x_100).txt");
		sendSD(&sdData, fileName, buf);
		sprintf(buf, "%d\n", data->undergroundTemperatureValue);
		sprintf(fileName, "%s", "UndergroundTemperature(deg_x_100).txt");
		sendSD(&sdData, fileName, buf);
		HAL_Delay(100);
		sprintf(buf, "%d\n", data->batteryStatus);
		sprintf(fileName, "%s", "BatteryStatus.txt");
		sendSD(&sdData, fileName, buf);
		HAL_Delay(100);
		if(!stopCard(&sdData)){

			printf("SD stop error");

		}

	}
	else{

		printf("SD start error");

	}
	HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
}

void mainOperation(struct rfDataStorage* rfData, struct CommunicationHandles* handles)
{
	printf("\r\nMain Operation");
	struct measurementData data;
	data.depthValue = -46;
	data.waterPressureValue = 65700;
	data.surfaceTemperatureValue = -5;
	data.undergroundTemperatureValue = 7;
	data.batteryStatus = 1;

	/*HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(TPSPI_CSGP, TPSPI_CSPin, GPIO_PIN_SET);

	HAL_Delay(10);
	struct SDinfo sdData;
	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;
	if(setupCard(&sdData)){
		if(!stopCard(&sdData)){

			printf("SD stop error");

		}

	}
	else{

		printf("SD start error");

	}

	HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);*/
	HAL_Delay(10);

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);
	printf("\r\nOperating at %02d::%02d::%02d",  sTime.Hours, sTime.Minutes, sTime.Seconds);

	performDepthMeasurement(handles->ADChandle1, handles->ADChandle2, &data);
	performPressureMeasurement(handles->I2Chandle, &data);
	performTemperatureMeasurement(handles->SPIhandle, &data);
	checkBattery(handles->ADChandle1, &data);
	checkBattery(handles->ADChandle1, &data);

	//HAL_Delay(4000);
	//addDeviceData(handles, &data);
	setRfListening(&data, rfData, handles);

	setShutdownMode();

}

#endif /* SRC_MANAGEMENT_C_ */
