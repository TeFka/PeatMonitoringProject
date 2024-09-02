/*
 * DepthMeasuring.h
 *
 *  Created on: Mar 7, 2023
 *      Author: zurly
 */

#ifndef INC_DEPTH_DEPTHMEASURING_H_
#define INC_DEPTH_DEPTHMEASURING_H_

#include "stm32l4xx_hal.h"

//depth data that has to be preserved
struct DepthData{

	int pos;
	//kept data
	int output1;
	int output2;
	int max_value_1;
	int min_value_1;
	int max_value_2;
	int min_value_2;

};

//function to retrieve distance value
void calculateDistance(ADC_HandleTypeDef* adcHandle1, ADC_HandleTypeDef* adcHandle2, struct DepthData* depthData);

#endif /* INC_DEPTH_DEPTHMEASURING_H_ */
