/*
 * DataHandling.c
 *
 *  Created on: Mar 27, 2023
 *      Author: zurly
 */


#include "DataHandling.h"

//Function to encode measurement data into bytes
void encodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t* dataSize){

	//Long word to store all bits from measuremetn data
	uint64_t measurementValue = 0;//data->measurementTime;

	//encode saved depth value
	uint8_t depthSign = (data->depthValue>0);
	uint64_t depthValue = data->depthValue>0?data->depthValue:data->depthValue*-1;
	measurementValue = measurementValue|(((uint64_t)depthSign)<<depthValueSignShift);
	measurementValue = measurementValue|((depthValue)<<depthValueShift);

	//encode water pressure value
	measurementValue = measurementValue|(((uint64_t)data->waterPressureValue)<<waterPressureShift);

	//encode surface temperature value
	uint8_t temperatureSign = (data->surfaceTemperatureValue>0);
	uint64_t temperatureValue = data->surfaceTemperatureValue>0?data->surfaceTemperatureValue:data->surfaceTemperatureValue*-1;
	measurementValue = measurementValue|(((uint64_t)temperatureSign)<<surfaceTemperatureSignShift);
	measurementValue = measurementValue|((temperatureValue)<<surfaceTemperatureShift);

	//encode underground temperature value
	uint8_t undergroundTemperatureSign = (data->undergroundTemperatureValue>0);
	uint64_t undergroundTemperatureValue = data->undergroundTemperatureValue>0?data->undergroundTemperatureValue:data->undergroundTemperatureValue*-1;
	measurementValue = measurementValue|(((uint64_t)undergroundTemperatureSign)<<undergroundTemperatureSignShift);
	measurementValue = measurementValue|((undergroundTemperatureValue)<<undergroundTemperatureShift);

	//encode battery state value
	measurementValue =  measurementValue|(((uint64_t)data->batteryStatus)<<batteryStatusShift);

	//separate the long word into bytes and add them to byte array
	int iterator = 0;
	for(int i = 0;i<=overallDataBitSize;i+=8){
		dataArray[iterator] = (measurementValue>>(i))&0xFF;
		iterator++;
	}

	*dataSize = iterator;
}

//Function to decode measurement data from bytes
void decodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t dataSize){

	uint64_t measurementValue = 0;

	//add all bytes from array in to the long word
	for(int i = 0;i<dataSize;i++){
		measurementValue = measurementValue | (((uint64_t)dataArray[i])<<i*8);
	}

	//decode the depth value
	uint8_t depthSign = ((uint8_t)(measurementValue>>depthValueSignShift))&depthValueSignMask;
	data->depthValue = ((uint16_t)(measurementValue>>depthValueShift))&depthValueMask;
	if(!depthSign) data->depthValue = data->depthValue * -1;

	//decode the water pressure value
	data->waterPressureValue = ((uint32_t)(measurementValue>>waterPressureShift))&waterPressureMask;

	//decode surface temperature value
	uint8_t surfaceTemperatureSign = ((uint8_t)(measurementValue>>surfaceTemperatureSignShift))&surfaceTemperatureSignMask;
	data->surfaceTemperatureValue = ((uint16_t)(measurementValue>>surfaceTemperatureShift))&surfaceTemperatureMask;
	if(!surfaceTemperatureSign)	data->surfaceTemperatureValue = data->surfaceTemperatureValue * -1;

	//decode underground temperature value
	uint8_t undergroundTemperatureSign = ((uint8_t)(measurementValue>>undergroundTemperatureSignShift))&undergroundTemperatureSignMask;
	data->undergroundTemperatureValue = ((uint16_t)(measurementValue>>undergroundTemperatureShift))&undergroundTemperatureMask;
	if(!undergroundTemperatureSign)	data->undergroundTemperatureValue = data->undergroundTemperatureValue * -1;

	//decode battery state value
	data->batteryStatus = ((uint8_t)(measurementValue>>batteryStatusShift))&batteryStatusMask;

}

//Function to compress data from separate arrays into long word array
void deviceDataCompression(uint64_t largeWords[], uint16_t numberOfValues, uint16_t deviceIds[], uint8_t deviceHops[]){

	//iterate through large word array and add separate values
	for(int i = 0;i<numberOfValues;i++){

		//retrieve device id
		largeWords[i] = deviceIds[i];

		//retrieve device hops
		largeWords[i] = largeWords[i] | (((uint64_t)deviceHops[i])<<16);

	}

}

//Function to decompress data from long word array in to separate arrays
void deviceDataDecompression(uint64_t largeWords[], uint16_t numberOfValues, uint16_t deviceIds[], uint8_t deviceHops[]){

	//iterate through large word array and retrieve separete values
	for(int i = 0;i<numberOfValues;i++){

		//retrieve device ids
		deviceIds[i] = ((uint16_t)(largeWords[i]))&0xFF;
		//retrieve device hops
		deviceHops[i] = ((uint8_t)(largeWords[i]>>16))&0x0F;

	}

}
