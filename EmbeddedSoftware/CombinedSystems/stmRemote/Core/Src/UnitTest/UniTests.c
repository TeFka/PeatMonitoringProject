/*
 * UniTests.c
 *
 *  Created on: Mar 17, 2023
 *      Author: zurly
 */

#include "UnitTest/UnitTests.h"

void SDcardTest(){

	printf("Performing SD card test");
	struct SDinfo sdData;

	FATFS fs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;

	sdData.fs = &fs;
	sdData.fil = &fil;
	sdData.fres = &fres;

	char buf[50];

	if(setupCard(&sdData)){
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

		printf("SD start error");

	}

	printf("SD test done");
}
