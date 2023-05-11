/*
 * DataChange.h
 *
 *  Created on: Mar 26, 2023
 *      Author: zurly
 */

#ifndef INC_DATACHANGE_H_
#define INC_DATACHANGE_H_

#include "stm32l4xx_hal.h"

//measurement data sizes
#define depthValueSignBitSize 				1
#define depthValueBitSize					12
#define waterPressureBitSize				17
#define surfaceTemperatureSignBitSize 		1
#define surfaceTemperatureBitSize 			12
#define undergroundTemperatureSignBitSize 	1
#define undergroundTemperatureBitSize 		12
#define batteryStatusBitSize 				1
#define overallDataBitSize					57

//measurement data shifts
#define depthValueSignShift 				0
#define depthValueShift 					1
#define waterPressureShift 					13
#define surfaceTemperatureSignShift 		30
#define surfaceTemperatureShift 			31
#define undergroundTemperatureSignShift 	43
#define undergroundTemperatureShift 		44
#define batteryStatusShift 					56

//measurement data masks
#define depthValueSignMask					0x01
#define depthValueMask 						0xFFF
#define waterPressureMask 					0x1FFFF
#define surfaceTemperatureSignMask 			0x01
#define surfaceTemperatureMask 				0x0FFF
#define undergroundTemperatureSignMask 		0x01
#define undergroundTemperatureMask 			0x0FFF
#define batteryStatusMask					0x01

#define bit8Comb(val1, val2)  ((uint16_t)val2<<8)|((uint16_t)val1)

//Structure to keep measurement data
struct measurementData{

	int depthValue;
	int waterPressureValue;
	int surfaceTemperatureValue;
	int undergroundTemperatureValue;
	int batteryStatus;

};

//Function to encode measurement data into bytes
void encodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t* dataSize);

//Function to decode measurement data from bytes
void decodeMeasurementData(struct measurementData* data, uint8_t dataArray[], uint8_t dataSize);

//Function to compress data from separate arrays into long word array
void deviceDataCompression(uint64_t largeWords[], uint16_t numberOfValues, uint16_t deviceIds[], uint8_t deviceHops[]);

//Function to decompress data from long word array in to separate arrays
void deviceDataDecompression(uint64_t largeWords[], uint16_t numberOfValues, uint16_t deviceIds[], uint8_t deviceHops[]);

#endif /* INC_DATACHANGE_H_ */
