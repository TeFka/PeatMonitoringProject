/*
 * LCDMeniu.c
 *
 *  Created on: Mar 2, 2023
 *      Author: zurly
 */

#include "LCD/LCDMeniu.h"


void LCDmeniuInit(I2C_HandleTypeDef* handle){

	lcd16x2_i2c_init(handle);

}

void handleControl(struct LCDmeniu* theMeniu){

	if(!theMeniu->changeActive){

		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)== GPIO_PIN_SET)
		{

			nextScreen(theMeniu);
			theMeniu->changeActive = 1;

		}

		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET)
		{

			nextScreen(theMeniu);
			theMeniu->changeActive = 1;

		}
	}
	else{

		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET &&
		   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET){

			theMeniu->changeActive = 0;
		}

	}

}

void updatePageData(struct meniuPage* thePage, char* newData, int dataSize){

	for(int i = 0;i<dataSize;i++){

		thePage->shownData[i] = newData[i];

	}
	for(int i = dataSize;i<16;i++){

		thePage->shownData[i] = '#';
	}

}

void nextScreen(struct LCDmeniu* theMeniu){

	if(theMeniu->shownPage->nextPageExists){
		theMeniu->shownPage = theMeniu->shownPage->nextPage;
	}

}

void previousScreen(struct LCDmeniu* theMeniu){

	if(theMeniu->shownPage->previousPageExists){
		theMeniu->shownPage = theMeniu->shownPage->nextPage;
	}

}

void showMeniu(struct LCDmeniu* theMeniu){

	char firstHalf[8];
	char secondHalf[8];

	for(int i = 0;i<8;i++){

		firstHalf[i] = theMeniu->shownPage->shownData[i];
		secondHalf[i] = theMeniu->shownPage->shownData[8+i];

	}

	lcd16x2_i2c_clear();
	HAL_Delay(20);
	lcd16x2_i2c_setCursor(0,0);
	HAL_Delay(20);
	lcd16x2_i2c_printf(firstHalf);
	HAL_Delay(20);
	lcd16x2_i2c_setCursor(0,1);
	HAL_Delay(20);
	lcd16x2_i2c_printf(secondHalf);
}
