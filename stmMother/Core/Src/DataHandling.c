/*
 * DataHandling.c
 *
 *  Created on: Mar 27, 2023
 *      Author: zurly
 */


#include "DataHandling.h"

void encodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t* dataSize){

	uint64_t measurementValue = 0;//data->measurementTime;

	uint8_t depthSign = (data->depthValue>0);
	uint64_t depthValue = data->depthValue>0?data->depthValue:data->depthValue*-1;
	measurementValue = measurementValue|(((uint64_t)depthSign)<<depthValueSignShift);
	measurementValue = measurementValue|((depthValue)<<depthValueShift);

	measurementValue = measurementValue|(((uint64_t)data->waterPressureValue)<<waterPressureShift);

	uint8_t temperatureSign = (data->surfaceTemperatureValue>0);
	uint64_t temperatureValue = data->surfaceTemperatureValue>0?data->surfaceTemperatureValue:data->surfaceTemperatureValue*-1;
	measurementValue = measurementValue|(((uint64_t)temperatureSign)<<surfaceTemperatureSignShift);
	measurementValue = measurementValue|((temperatureValue)<<surfaceTemperatureShift);

	uint8_t undergroundTemperatureSign = (data->undergroundTemperatureValue>0);
	uint64_t undergroundTemperatureValue = data->undergroundTemperatureValue>0?data->undergroundTemperatureValue:data->undergroundTemperatureValue*-1;
	measurementValue = measurementValue|(((uint64_t)undergroundTemperatureSign)<<undergroundTemperatureSignShift);
	measurementValue = measurementValue|((undergroundTemperatureValue)<<undergroundTemperatureShift);

	measurementValue =  measurementValue|(((uint64_t)data->batteryStatus)<<batteryStatusShift);

	int iterator = 0;
	for(int i = 0;i<=overallDataBitSize;i+=8){
		dataArray[iterator] = (measurementValue>>(i))&0xFF;
		iterator++;
	}

	*dataSize = iterator;
}

void decodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t dataSize){

	uint64_t measurementValue = 0;

	for(int i = 0;i<dataSize;i++){
		measurementValue = measurementValue | (((uint64_t)dataArray[i])<<i*8);
	}

	uint8_t depthSign = ((uint8_t)(measurementValue>>depthValueSignShift))&depthValueSignMask;
	data->depthValue = ((uint16_t)(measurementValue>>depthValueShift))&depthValueMask;
	if(!depthSign) data->depthValue = data->depthValue * -1;

	data->waterPressureValue = ((uint32_t)(measurementValue>>waterPressureShift))&waterPressureMask;

	uint8_t surfaceTemperatureSign = ((uint8_t)(measurementValue>>surfaceTemperatureSignShift))&surfaceTemperatureSignMask;
	data->surfaceTemperatureValue = ((uint16_t)(measurementValue>>surfaceTemperatureShift))&surfaceTemperatureMask;
	if(!surfaceTemperatureSign)	data->surfaceTemperatureValue = data->surfaceTemperatureValue * -1;

	uint8_t undergroundTemperatureSign = ((uint8_t)(measurementValue>>undergroundTemperatureSignShift))&undergroundTemperatureSignMask;
	data->undergroundTemperatureValue = ((uint16_t)(measurementValue>>undergroundTemperatureShift))&undergroundTemperatureMask;
	if(!undergroundTemperatureSign)	data->undergroundTemperatureValue = data->undergroundTemperatureValue * -1;

	data->batteryStatus = ((uint8_t)(measurementValue>>batteryStatusShift))&batteryStatusMask;

}
