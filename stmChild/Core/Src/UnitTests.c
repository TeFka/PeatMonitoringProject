/*
 * UnitTests.c
 *
 *  Created on: 21 Mar 2023
 *      Author: zurly
 */

#include <UnitTests.h>

void pressureTest(I2C_HandleTypeDef* I2Chandle, uint8_t address){

	HAL_StatusTypeDef result;

	/*for (int i=1; i<128; i++)
	{
	  result = HAL_I2C_IsDeviceReady(I2Chandle, (uint16_t)(i<<1), 2, 2);
	  if (result != HAL_OK) // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
	  {
		  printf("."); // No ACK received at that address
	  }
	  if (result == HAL_OK)
	  {
		  printf("0x%X", i); // Received an ACK at that address
	  }
	}*/

	lps28dfw_info_t sensorInfo1;
	sensorInfo1.handle = I2Chandle;
	printf("looking for %x", address);
	if(beginLPS28(&sensorInfo1, address) != LPS28DFW_OK)
	{
		printf("\r\nCannot connect to pressure sensor");
		HAL_Delay(1000);
		return;
	}
	lps28dfw_md_t modeConfig =
	{
		.fs  = fs_LPS28DFW_1260hPa,      // Full scale range
		.odr = odr_LPS28DFW_4Hz,         // Output data rate
		.avg = avg_LPS28DFW_512_AVG,      // Average filter
		.lpf = lpf_LPS28DFW_LPF_ODR_DIV_9 // Low-pass filter
	};

	setModeConfig(&sensorInfo1, &modeConfig);
	HAL_Delay(100);
	getSensorData(&sensorInfo1);
	double runnningValue = 0;
	for(int i = 0;i<4;i++){
		getSensorData(&sensorInfo1);
		runnningValue += sensorInfo1.data.pressure.hpa;
		HAL_Delay(250);
	}
	runnningValue = runnningValue/4;

	uint32_t temperatureValue = (uint32_t)(sensorInfo1.data.heat.deg_c*100);

	uint32_t pressureValue = (uint32_t)(sensorInfo1.data.pressure.hpa*100);

	printf("\r\nMeasure pressure: %ld", pressureValue);
	printf("\r\nMeasure temperature: %ld", temperatureValue);
}

void TemperatureTest(SPI_HandleTypeDef*	SPIhandle){

	configureSensor(SPIhandle);

	double runnningValue = 0;
	for(int i = 0;i<4;i++){
		runnningValue += getTemperatureVal(SPIhandle);
		HAL_Delay(100);
	}
	runnningValue = runnningValue/4;

	uint16_t temperatureValue = (uint16_t)(runnningValue*100);

	printf("\r\nMeasured temperature: %f", getTemperatureVal(SPIhandle));

}

void SDcardTest(){

	printf("Performing SD card test");

	FATFS fs;
	  FIL fil;
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_Delay(100);
	  if(f_mount(&fs, "/", 0)==FR_OK){
		  HAL_Delay(100);
		  printf("\r\nSD card mounted");
		  if(f_open(&fil, "write.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ)==FR_OK){

			  printf("\r\nFile opend");

		  }
		  else{
			  printf("\r\nCould not open File");
		  }
		  HAL_Delay(100);
		  f_lseek(&fil, f_size(&fil));
		  HAL_Delay(100);
		  f_puts("Hello World\n", &fil);
		  HAL_Delay(100);
		  if(f_close(&fil)==FR_OK){

			  printf("\r\nFile closed");

		  }
		  else{
			  printf("\r\nCould not close File");
		  }
		  if(f_mount(NULL, "/", 1)==FR_OK){

			  printf("\r\nSD card closed");

		  }
		  else{
			  printf("\r\nFailed to close");
		  }
	  }
	  else{
		  printf("\r\nSD card error");
	  }

	/*struct SDinfo sdData;

	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;

	char buf[50];

	if(setupCard(&sdData)){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
		printf("Card mounted successfully");
		for(int i = 0;i<5;i++){

			char folderName[40];
			sprintf(folderName, "Device_%d", i);

			sprintf(buf, "Device_%ld\n", i);
			sendSD(&sdData, "AllDeviceInfo.txt", buf);
			f_mkdir(buf);

			for(int n = 0;n<10;n++){

					char folderName[40];

					char fileName[100];

					sprintf(folderName, "Device_%d", i);

					sprintf(buf, "%d\n", n);
					sprintf(fileName, "%s/Time.txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", n*2);
					sprintf(fileName, "%s/DepthChange(mm_divided_by_10).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", n*5);
					sprintf(fileName, "%s/PressureValue(Pa).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", n*10);
					sprintf(fileName, "%s/Temperature(deg).txt", folderName);
					sendSD(&sdData, fileName, buf);

					sprintf(buf, "%d\n", n*20);
					sprintf(fileName, "%s/BatteryStatus.txt", folderName);
					sendSD(&sdData, fileName, buf);


			}
		}
		if(!stopCard(&sdData)){

			printf("SD stop error");

		}
		else{
			printf("Card unmounted successfully");
		}

	}
	else{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
		printf("SD start error");

	}
*/
	printf("SD test done");
}
void depthTest(ADC_HandleTypeDef* adcHandle1, ADC_HandleTypeDef* adcHandle2, struct DepthData* depthData){

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_5;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	  sConfig.SingleDiff = ADC_SINGLE_ENDED;
	  sConfig.OffsetNumber = ADC_OFFSET_NONE;
	  sConfig.Offset = 0;
	  if (HAL_ADC_ConfigChannel(adcHandle1, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	calculateDistance(adcHandle1, adcHandle2, depthData);

	float depthValue = 100*(((float)depthData->pos)/35);

	printf("\r\nCalculated distance: %f", depthValue);

}

void rfTest(UART_HandleTypeDef* handle){

	uint8_t rxBuf[20];

	HAL_UART_Transmit(handle, "AT", 2, 1000);

	HAL_UART_Receive(handle, rxBuf, 5, 1000);

	printf("\r\nReceived RF response: %s", rxBuf);
}

void flashTest(){

	uint64_t testData[] = {10,11,55,256};
	 uint64_t testDataToRead[4];
	  Flash_Write_Data(USABLE_FLASH_START, testData, 4);

	  Flash_Read_Data(USABLE_FLASH_START, testDataToRead, 4);

	  for(int i = 0;i<4;i++){

		  printf("\r\nData at %d: %ld", i, (uint32_t)testDataToRead[i]);

	  }
}

void batteryTest(ADC_HandleTypeDef* adcHandle1){

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
	 //HAL_ADC_Stop(adcHandle1);
	 /*when first battery run out, the back-up battery will passively turned on*/
	 /*higher voltage will be detected from the second battery*/
	 /*if the ADC pin detects a higher voltage turn on at second battery terminal*/
	 /*comm pin

	 if (voltage > VOLTAGE_THRESHOLD){
		/*second battery is turned on
		/*save first battery state 0
		printf("First battery state 0\r\n");
		/*send first battery run out warning
		printf("Back-up battery turned on\r\n\n");
	 }
	 else{
		printf("First battery running\r\n\n");
	 }*/

}


