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

//Function to perform depth measurement
void performDepthMeasurement(ADC_HandleTypeDef*  handle1, ADC_HandleTypeDef*  handle2, struct measurementData* data){

	//Set up the ADC channel for depth measuring
	ADC_ChannelConfTypeDef sConfig = {0};
	//set all config parameters
	sConfig.Channel = ADC_CHANNEL_5;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	  sConfig.SingleDiff = ADC_SINGLE_ENDED;
	  sConfig.OffsetNumber = ADC_OFFSET_NONE;
	  sConfig.Offset = 0;

	  //Set active config channel
	  if (HAL_ADC_ConfigChannel(handle1, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	//Retrieve depth measurement calibration values from flash memory
	uint64_t supportVariables[numberOfDepthVariables];
	Flash_Read_Data(depthVariablesStartAddr, supportVariables, numberOfDepthVariables);

	//Define all depth measuring data and calibration values
	struct DepthData depthData;
	depthData.pos = ((int)supportVariables[0]);
	depthData.output1 = ((int)supportVariables[1]);
	depthData.output2 = ((int)supportVariables[2]);
	depthData.max_value_1 = ((int)supportVariables[3]);
	depthData.min_value_1 = ((int)supportVariables[4]);
	depthData.max_value_2 = ((int)supportVariables[5]);
	depthData.min_value_2 = ((int)supportVariables[6]);
	//printf("\r\nstored data: %d, %d, %d, %d, %d, %d, %d", depthData.pos, depthData.output1, depthData.output2, depthData.max_value_1, depthData.min_value_1, depthData.max_value_2, depthData.min_value_2);

	//Power on depth measuring device
	HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Perform depth measurement
	calculateDistance(handle1, handle2, &depthData);

	//Power of depth measuring device
	HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_RESET); // depth

	//Retrieve actual depth value
	data->depthValue = 100*(((float)depthData.pos)/50);

	printf("\r\nCalculated distance: %d", data->depthValue);

	//Save latest calibration values
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

//Function to perform pressure measurement
void performPressureMeasurement(I2C_HandleTypeDef* handle, struct measurementData* data){

	//Power on pressure measuring device
	HAL_GPIO_WritePin(i2cGPIO, i2cGPIOPin, GPIO_PIN_SET); //pressure activation pin
	HAL_Delay(100);

	//Define pressure sensor
	lps28dfw_info_t sensorInfo1;
	sensorInfo1.handle = handle;

	//Connect to pressure sensor
	if(beginLPS28(&sensorInfo1, LPS28DFW_I2C_ADDRESS_DEFAULT) == LPS28DFW_OK)
	{

		//Configure pressure sensor for best performance
		lps28dfw_md_t modeConfig =
		{
			.fs  = fs_LPS28DFW_1260hPa,      // Full scale range
			.odr = odr_LPS28DFW_4Hz,         // Output data rate
			.avg = avg_LPS28DFW_512_AVG,      // Average filter
			.lpf = lpf_LPS28DFW_LPF_ODR_DIV_9 // Low-pass filter
		};
		setModeConfig(&sensorInfo1, &modeConfig);

		//Delay for pressure sensor to configure
		HAL_Delay(100);

		//Retrieve 4 values and get their average
		double runnningPressureValue = 0;
		double runnningTemperatureValue = 0;
		for(int i = 0;i<4;i++){
			//get sensor value
			getSensorData(&sensorInfo1);
			//add pressure value to running value
			runnningPressureValue += sensorInfo1.data.pressure.hpa;
			//add temperature value to running value
			runnningTemperatureValue += sensorInfo1.data.heat.deg_c;

			//Have a delay to measure a new value
			HAL_Delay(300);
		}

		//Retrieve average value
		runnningPressureValue = runnningPressureValue/4;
		runnningTemperatureValue = runnningTemperatureValue/4;

		//Add pressure data to measurement structure
		data->waterPressureValue = (uint32_t)(runnningPressureValue*100);
		data->undergroundTemperatureValue = (uint16_t)(runnningTemperatureValue*100);

		printf("\r\n Measured pressure: %d", data->waterPressureValue);

	}
	else{

		printf("\r\nCould not connect to pressure sensor");

	}

	//Power off pressure measuring device
	HAL_GPIO_WritePin(i2cGPIO, i2cGPIOPin, GPIO_PIN_RESET);

}

//Function to perform temperature measurement
void performTemperatureMeasurement(SPI_HandleTypeDef* handle, struct measurementData* data){

	//Power on temperature measuring device
	HAL_GPIO_WritePin(spiGPIO, spiGPIOPin, GPIO_PIN_SET); //temperature activation pin
	HAL_Delay(100);

	//Configure temperature sensor for best performance
	configureSensor(handle);

	//Retrieve 4 values and get their average
	double runnningValue = 0;
	for(int i = 0;i<4;i++){
		//add pressure value to running value
		runnningValue += getTemperatureVal(handle);

		//Have a delay to measure a new value
		HAL_Delay(300);
	}

	//Retrieve average value
	runnningValue = runnningValue/4;

	//Add temperature data to measurement structure
	data->surfaceTemperatureValue = (uint16_t)(runnningValue*100);

	printf("\r\nMeasured temperature: %f", getTemperatureVal(handle));

	//Power off temperature measuring device
	HAL_GPIO_WritePin(spiGPIO, spiGPIOPin, GPIO_PIN_RESET);
}

//Function to check battery state
void checkBattery(ADC_HandleTypeDef* adcHandle1, struct measurementData* data){

	printf("\r\nChecking battery");

	//Set up the ADC channel for battery checking
	ADC_ChannelConfTypeDef sConfig = {0};

	//set all config parameters
	sConfig.Channel = ADC_CHANNEL_6;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	  sConfig.SingleDiff = ADC_SINGLE_ENDED;
	  sConfig.OffsetNumber = ADC_OFFSET_NONE;
	  sConfig.Offset = 0;
	  //Set active config channel
	  if (HAL_ADC_ConfigChannel(adcHandle1, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	 //Power on battery connection device
	 HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_SET);
	 HAL_Delay(100);

	 //Retrive ADC data from battery channel
	 HAL_ADC_Start(adcHandle1);
	 HAL_ADC_PollForConversion(adcHandle1, 0xFFFF);
	 uint16_t ADC_value = HAL_ADC_GetValue(adcHandle1);

	 //Calculate battery voltage based on ADC value
	 float voltage = ((ADC_value /4096.0) * Vref);
	 printf("ADC=%hu, Voltage= %.2f\r\n", ADC_value, voltage);
	 HAL_Delay(100);

	 /*when first battery run out, the back-up battery will passively turned on*/
	 /*higher voltage will be detected from the second battery*/
	 /*if the ADC pin detects a higher voltage turn on at second battery terminal*/
	 /*comm pin*/

	//Confirm if voltage specifies that first battery is depleted
	if (voltage > VOLTAGE_THRESHOLD){
		 printf("First battery depleted");
		 data->batteryStatus = 0;
	}
	 else{
		 printf("first battery ok");
	 }

	//Power off battery connection device
	 HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_RESET);
	 HAL_Delay(100);

}

//Function listen to listen for RF requests
void setRfListening(struct measurementData* data, struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	//Power on battery communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET); //rf activation pin
	HAL_Delay(100);

	//Read management data from flash memory
	uint64_t managementBuf[managementDataNum];
	Flash_Read_Data(managementDataAddr, managementBuf, managementDataNum);

	//Initialize communications with retrieved device ID and discover flag
	init_comms(rfData, DEVICE_CHILD, managementBuf[0], managementBuf[3]);


	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_SEND_DATA;
	//Specify child device as sender
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0xFF;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = (rfData->myDevice.device_id>>8)&0xFF;
	//Specify mother as received
	rfData->activeTxMessage[MSG_TO_ID_POS1] = managementBuf[1]&0xFF;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = (managementBuf[1]>>8)&0xFF;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = managementBuf[4];

	printf("\r\nmeasurement data to send: %d, %d, %d, %d, %d", data->depthValue, data->waterPressureValue, data->surfaceTemperatureValue, data->undergroundTemperatureValue, data->batteryStatus);

	//Encode data from measurement structure into bytes
	uint8_t encodedDataSize = 0;
	uint8_t encodedData[20];
	encodeMeasurementData(data, encodedData, &encodedDataSize);

	//Add bytes into RF message
	for(int i = 0;i<encodedDataSize;i++){

		rfData->activeTxMessage[MSG_BODY_START+i] = encodedData[i];

	}
	//Specify overall measuremetn data size in bytes
	rfData->activeTxMessageSize = encodedDataSize;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	//DEfine how long child device should be listening to mother device
	int listening = 1;
	int listeningCounter = 500;

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);
	//Listen for mother device
	while(listening){

		//Handle received messages
		Comms_Handler(rfData);
		printf("\r\nListening of mother device");
		//iterate over potential messages list
		for(int i = 0; i<rfData->messageNum; i++){
			printf("\r\nmessage %d type: %x", i, rfData->activeRxMessage[i][MSG_TYPE_POS]);

			//If the "Done" message was received
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_DONE){

				printf("scan done, received time: %d %d %d", rfData->activeRxMessage[i][MSG_BODY_START], rfData->activeRxMessage[i][MSG_BODY_START+1], rfData->activeRxMessage[i][MSG_BODY_START+2]);

				//Define time received by mother device
				sTime.Hours = rfData->activeRxMessage[i][MSG_BODY_START];
				sTime.Minutes = rfData->activeRxMessage[i][MSG_BODY_START+1];
				sTime.Seconds = rfData->activeRxMessage[i][MSG_BODY_START+2];

				//Set latest time received by mother device to synchronize the devices
				if (HAL_RTC_SetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN) != HAL_OK)
				{
					//printf("Could not set time");
					Error_Handler();
				}

				//Wait a delay
				HAL_Delay(1000);

				//Forward the message further in addition to the default forwarding
				Send(rfData->activeRxMessage[i], rfData->activeRxMessage[i][MSG_SIZE_POS]+MSG_BODY_START);

				listening = 0;
				break;
			}
			//If data is requested by mother device
			if (rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_GIVE_DATA)
			{
				if(rfData->toID[rfData->messageNum] == rfData->myDevice.device_id){
					//Send predefined data to mother deivce
					Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);
				}
			}

		}

		//Stop listening if counter is done
		if(listeningCounter<=0){

			listening = 0;

		}
		else{
			//Update counter
			listeningCounter--;
			HAL_Delay(200);
		}

	}

	//Power off battery communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);
}

//Fucntion to power down all externally connected devices
void powerDownDevices(){

	//Power down all available peripherals
	HAL_GPIO_WritePin(adcGPIO, adcGPIOPin, GPIO_PIN_RESET); //depth
	HAL_GPIO_WritePin(i2cGPIO, i2cGPIOPin, GPIO_PIN_RESET); //pressure
	HAL_GPIO_WritePin(spiGPIO, spiGPIOPin, GPIO_PIN_RESET); //temperature
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET); //rf

}

//Function to perform child device setup and synchronization with mother device
void synchronizationSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	//Systemtile holder
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);

	//Read device data from
	uint64_t managementBuf[managementDataNum];
	Flash_Read_Data(managementDataAddr, managementBuf, managementDataNum);

	//Initialize communications with retrieved device ID and discover flag
	init_comms(rfData, DEVICE_CHILD, managementBuf[0], 0);

	//Specify child device as sender
	//rfData->activeTxMessage[MSG_FROM_ID_POS1] = managementBuf[0]&0xFF;
	//rfData->activeTxMessage[MSG_FROM_ID_POS2] = (managementBuf[0]>>8)&0xFF;

	printf("sent ID: %d->%d and %d", (uint16_t)managementBuf[1], rfData->activeTxMessage[MSG_FROM_ID_POS2], rfData->activeTxMessage[MSG_FROM_ID_POS1]);

	int listening = 1;

	//Power on battery communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_SET); //rf activation pin
	HAL_Delay(100);

	while(listening){
		printf("\r\nsignaling from id: %d", rfData->myDevice.device_id);

		//Handle received messages
		Comms_Handler(rfData);

		//iterate over potential messages list
		for(int i = 0; i<rfData->messageNum; i++){

			printf("message %d type: %x", i, rfData->activeRxMessage[i][MSG_TYPE_POS]);
			//If "You There" was received from mother device
			if(rfData->activeRxMessage[i][MSG_TYPE_POS] == MSG_TYPE_YOUR_THERE)
			{
				printf("Comparing IDs: %d and %d", rfData->myDevice.device_id, rfData->toID[i]);
				if(rfData->myDevice.device_id == rfData->toID[i])
				{
					//Set device as discovered
					rfData->myDevice.device_is_discovered = true;

					//Set time value received from mother device
					sTime.Hours = rfData->activeRxMessage[i][MSG_BODY_START];
					sTime.Minutes = rfData->activeRxMessage[i][MSG_BODY_START+1];
					sTime.Seconds = rfData->activeRxMessage[i][MSG_BODY_START+2];

					printf("Device discovered, time set to: %d::%d::%d", sTime.Hours, sTime.Minutes, sTime.Seconds);

					//Set time received by mother device to synchronize the devices
					if (HAL_RTC_SetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN) != HAL_OK)
					{
						printf("Error setting time");
						Error_Handler();
					}

					//Set configuration flags for device discovery
					if(!managementBuf[3]){
						managementBuf[1] = rfData->fromID[i]; //Found mother device ID
						managementBuf[3] = 1; //discovered flag
						//specified required number of mesh hops to reach mother device
						managementBuf[4] = rfData->activeTxMessage[MSG_NUM_HOPS_POS];

					}

					listening = false;
					break;
				}

			}

		}
		HAL_Delay(100);
	}

	//Power off battery communication device
	HAL_GPIO_WritePin(uartGPIO, uartGPIOPin, GPIO_PIN_RESET);

	//Write updated device data to flash memory
	Flash_Write_Data(managementDataAddr, managementBuf, managementDataNum);

}

//Function to setup memory for operation
void memorySetup(){

	//Retrive management data from flash memory
	uint64_t setupData[managementDataNum];
	Flash_Read_Data(managementDataAddr, setupData, managementDataNum);

	//Set setup flag, implying that setup is complete
	setupData[2] = 1;

	//Write refreshed setup data
	Flash_Write_Data(managementDataAddr, setupData, managementDataNum);

	//Refresh all depth calibration values
	uint64_t depthVariables[numberOfDepthVariables] = {0,0,0,0,0,0,0};
	Flash_Write_Data(depthVariablesStartAddr, depthVariables, numberOfDepthVariables);

}

//Function to setup device for operation
int mainSetup(struct rfDataStorage* rfData, struct CommunicationHandles* handles){

	//Check if user wants to reset device IDs
	if(IDhandling()){
		return 0;
	}

	//Power dewn all devices
	powerDownDevices();

	//Retrieve setup bit from flash memory
	uint64_t setupBit[1];
	Flash_Read_Data(setupAddr, setupBit, 1);

	//Check if setup should be performed
	if(setupBit[0]!=1){

		printf("\r\nMain setup\n");

		//Perform child device synchronization with mother device
		synchronizationSetup(rfData, handles);

		//Set memory values for setup
		printf("\r\nSynchro Done\n");
		memorySetup();

		//Go into shutdown mode and start normal operation
		printf("\r\nSetup Done\n");
		setShutdownMode(handles);
	}
	return 1;
}

//Function to handle device ID changes
int IDhandling(){

	printf("\r\nChecking ID set pin : %d ", HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin));

	//Check if user pressed cinfiguration button
	if(HAL_GPIO_ReadPin(idSetGPIO, idSetGPIOPin) == GPIO_PIN_SET)
	{
		uint64_t managementBuf[managementDataNum] = {15, 70, 0, 0, 0};
		uint64_t rxBuf[2];
		int buf;

		//Request user to set child device ID
		printf("\r\nSet device ID");
		setvbuf( stdin, NULL, _IONBF, 0 );
		scanf("%d", &buf);
		printf("\r\nData to write: %ld", (uint32_t)buf);
		managementBuf[0] = 15;//buf;

		//Set other configuration flags
		managementBuf[1] = 0; //setup flag
		managementBuf[2] = 0; //setup flag
		managementBuf[3] = 0; //device discovery flag
		managementBuf[4] = 0; //device mesh hops value

		//Write configuration data to flash memory
		if(Flash_Write_Data(IDaddressAddr, managementBuf, managementDataNum) == HAL_OK){

			//Read written ID values for confirmation
			Flash_Read_Data(IDaddressAddr, rxBuf, 2);
			printf("\r\nID set to %ld", (uint32_t)managementBuf[0]);

		}

		printf("\r\nSetup bit set to zero");

		return 1;
	}

	return 0;
}

//Function to to let device go into shutdown mode
void setShutdownMode(){

	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WRFBUSY);

	//Suspend system tick to prevent device from waking up beforehand
	powerDownDevices();
	printf("\r\nEntering Shutdown Mode");
	HAL_SuspendTick();
	HAL_PWREx_EnterSHUTDOWNMode();

}

//Function to perform main device operation
void mainOperation(struct rfDataStorage* rfData, struct CommunicationHandles* handles)
{
	printf("\r\nMain Operation");

	//Define measurement data structure
	struct measurementData data;
	data.depthValue = -46;
	data.waterPressureValue = 65700;
	data.surfaceTemperatureValue = -5;
	data.undergroundTemperatureValue = 7;
	data.batteryStatus = 1;

	//Show current time
	/*RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(handles->RTChandle, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(handles->RTChandle, &sDate, RTC_FORMAT_BIN);
	printf("\r\nOperating at %02d::%02d::%02d",  sTime.Hours, sTime.Minutes, sTime.Seconds);*/

	//Perform depth measuring
	performDepthMeasurement(handles->ADChandle1, handles->ADChandle2, &data);
	//Perform pressure measuring
	performPressureMeasurement(handles->I2Chandle, &data);
	//Perform temperature measuring
	performTemperatureMeasurement(handles->SPIhandle, &data);
	//Perform battery checking
	checkBattery(handles->ADChandle1, &data);
	checkBattery(handles->ADChandle1, &data);

	//Start listening for requests from mother device
	setRfListening(&data, rfData, handles);

	//Go into shutdown mode
	setShutdownMode();

}

#endif /* SRC_MANAGEMENT_C_ */
