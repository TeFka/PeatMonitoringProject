/*
 * DepthMeasuring.c
 *
 *  Created on: Mar 7, 2023
 *      Author: zurly
 */

#include "Depth/DepthMeasuring.h"


void calculateDistance(ADC_HandleTypeDef* adcHandle1, ADC_HandleTypeDef* adcHandle2, struct DepthData* depthData){

	int outPrev1 = 0;
	int outPrev2 = 0;
	float div1 = 0;
	float div2 = 0;
    HAL_ADC_Start(adcHandle1);
    HAL_ADC_PollForConversion(adcHandle1, HAL_MAX_DELAY);

	float raw_value1 = (float)HAL_ADC_GetValue(adcHandle1);
    HAL_ADC_Stop(adcHandle1);

	HAL_ADC_Start(adcHandle2);
    HAL_ADC_PollForConversion(adcHandle2, HAL_MAX_DELAY);
    float raw_value2 = (float)HAL_ADC_GetValue(adcHandle2);
    HAL_ADC_Stop(adcHandle2);

    printf("\r\nRaw values: %f, %f", raw_value1, raw_value2);

    if (raw_value1 > depthData->max_value_1 ) depthData->max_value_1 = raw_value1 ;
    if (raw_value2 > depthData->max_value_2 ) depthData->max_value_2 = raw_value2 ;
    if (raw_value1 < depthData->min_value_1 ) depthData->min_value_1 = raw_value1 ;
    if (raw_value2 < depthData->min_value_2 ) depthData->min_value_2 = raw_value2 ;

    div1 = depthData->max_value_1 / 29;
    div2 = depthData->max_value_2 / 29;
    outPrev1 = depthData->output1;
    outPrev2 = depthData->output2;
    depthData->output1 = (int) raw_value1 / div1;
	depthData->output2 = (int) raw_value2 / div2;

    if (depthData->output1 == outPrev1 && depthData->output2 == outPrev2)
	{
    	return;
	}
    if ( depthData->output2 > depthData->output1 && depthData->output2 < 29 && depthData->output1 < 2){

		if (depthData->output2 > outPrev2) depthData->pos = depthData->pos + (depthData->output2 - outPrev2);
		if (depthData->output2 < outPrev2) depthData->pos = depthData->pos - (outPrev2 - depthData->output2);
		return;
	}
    if (depthData->output2 > depthData->output1 && depthData->output1 > 1 && depthData->output2 > 18 )
	{

		if (depthData->output1 > outPrev1 ) depthData->pos = depthData->pos + (depthData->output1 - outPrev1);
		if (depthData->output1 < outPrev1 ) depthData->pos = depthData->pos - (outPrev1 - depthData->output1);
		return;
	}
    if (depthData->output1 > depthData->output2 && depthData->output2 > 1 && depthData->output1 > 25)
	{

		if (depthData->output2 > outPrev2) depthData->pos = depthData->pos - (depthData->output2 - outPrev2);
		if (depthData->output2 < outPrev2) depthData->pos = depthData->pos + (outPrev2 - depthData->output2);
		return;
	}
    if (depthData->output1 > depthData->output2 && depthData->output1 < 29 && depthData->output2 < 2)
	{

		if (depthData->output1 > outPrev1) depthData->pos = depthData->pos - (depthData->output1 - outPrev1);
		if (depthData->output1 < outPrev1) depthData->pos = depthData->pos + (outPrev1 - depthData->output1);
		return;
	}

    return;
}
