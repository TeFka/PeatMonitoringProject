/*
 * LCDMeniu.h
 *
 *  Created on: Mar 2, 2023
 *      Author: zurly
 */

#ifndef INC_LCD_LCDMENIU_H_
#define INC_LCD_LCDMENIU_H_

#include "LCD/lcd16x2_i2c.h"

 struct meniuPage{

	 char 		shownData[16];
	 struct meniuPage* previousPage;
	 struct meniuPage* nextPage;
	 int nextPageExists;
	 int previousPageExists;

};

struct LCDmeniu{

	struct meniuPage*  mainPage;
	struct meniuPage*  shownPage;
	struct meniuPage   allPages[20];

	int changeActive;
};

struct extensionPage{

	 struct meniuPage* theMeniuPage;


 };

void LCDmeniuInit(I2C_HandleTypeDef* handle);

void handleControl(struct LCDmeniu* theMeniu);

void updatePageData(struct meniuPage* thePage, char* newData, int dataSize);

void nextScreen(struct LCDmeniu* theMeniu);
void previousScreen(struct LCDmeniu* theMeniu);

void showMeniu(struct LCDmeniu* theMeniu);

#endif /* INC_LCD_LCDMENIU_H_ */
