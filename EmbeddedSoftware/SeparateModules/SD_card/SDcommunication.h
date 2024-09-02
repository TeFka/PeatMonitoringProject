/*
 * SDcommunication.h
 *
 *  Created on: Feb 12, 2023
 *      Author: Admin
 */

#ifndef INC_SDCARD_SDCOMMUNICATION_SDCOMMUNICATION_H_
#define INC_SDCARD_SDCOMMUNICATION_SDCOMMUNICATION_H_

#include "stm32l4xx_hal.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>

struct SDinfo{

	FATFS* fs; 	//Fatfs handle
	FIL* fil; 		//File handle
	FRESULT* fres;

};

#define SD_CS_PORT      GPIOA
#define SD_CS_PIN     	GPIO_PIN_10

void process_SD_card(struct SDinfo* config);

void getChildIDs();

int setupCard(struct SDinfo* sdData);
int stopCard(struct SDinfo* sdData);

void sendSD(struct SDinfo* config, char* file, char* data);
void getSD(struct SDinfo* config, char* file, char* data);

#endif /* INC_SDCARD_SDCOMMUNICATION_SDCOMMUNICATION_H_ */
