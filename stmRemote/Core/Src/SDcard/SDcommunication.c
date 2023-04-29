/*
 * SDcommunication.c
 *
 *  Created on: Feb 12, 2023
 *      Author: Admin
 */

#include <SDcard/SDcommunication.h>

void process_SD_card(struct SDinfo* config)
{

  FRESULT     fres;                 //Result after operations
  char        buf[100];
  do
  {
    //Mount the SD Card
    fres = f_mount(config->fs, "", 1);    //1=mount now
    if (fres != FR_OK)
    {
      printf("No SD Card found : (%i)\r\n", fres);
      break;
    }
    printf("SD Card Mounted Successfully!!!\r\n");
    //Read the SD Card Total size and Free Size
    FATFS *pfs;
    DWORD fre_clust;
    uint32_t totalSpace, freeSpace;
    f_getfree("", &fre_clust, &pfs);
    totalSpace = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
    freeSpace = (uint32_t)(fre_clust * pfs->csize * 0.5);
    printf("TotalSpace : %lu bytes, FreeSpace = %lu bytes\n", totalSpace, freeSpace);
    //Open the file
    fres = f_open(config->fil, "Info.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if(fres != FR_OK)
    {
      printf("File creation/open Error : (%i)\r\n", fres);
      break;
    }
    printf("Writing data!!!\r\n");
    //write the data
    f_puts("Peat Monitoring Data", config->fil);
    //close your file
    f_close(config->fil);
    //Open the file
    fres = f_open(config->fil, "Info.txt", FA_READ);
    if(fres != FR_OK)
    {
      printf("File opening Error : (%i)\r\n", fres);
      break;
    }
    //read the data
    f_gets(buf, sizeof(buf), config->fil);
    printf("Read Data : %s\n", buf);
    //close your file
    f_close(config->fil);
    printf("Closing File!!!\r\n");
#if 0
    //Delete the file.
    fres = f_unlink("Info.txt");
    if (fres != FR_OK)
    {
      printf("Cannot able to delete the file\n");
    }
#endif
  } while(0);
  //We're done, so de-mount the drive
  f_mount(NULL, "", 0);
  printf("SD Card Unmounted Successfully!!!\r\n");
}

void sendSD(struct SDinfo* config, char* file, char* data){

	  HAL_Delay(50);
	  *config->fres = f_open(config->fil, file, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	  if(*config->fres == FR_OK){

		  printf("\r\nFile %s opened", file);

	  }
	  else{

		  printf("\r\ncould not open %s file", file);

	  }
	  *config->fres = f_lseek(config->fil, f_size(config->fil));
	  f_puts(data, config->fil);
	  *config->fres = f_close(config->fil);
	  if(*config->fres == FR_OK){

		  printf("\r\nFile closed");

	  }
	  else{

		  printf("\r\ncould not close file");

	  }
}

int setupCard(struct SDinfo* sdData){


	FRESULT fresult = f_mount(sdData->fs, "/", 1);
	HAL_Delay(100);
	return (fresult == FR_OK);

}

int stopCard(struct SDinfo* sdData){

	FRESULT fresult = f_mount(NULL, "/", 1);

	return (fresult == FR_OK);

}

void getSD(struct SDinfo* config, char* file, char* buffer){

	UINT br;
	  if(f_open(config->fil, file, FA_READ) != FR_OK){

		  printf("\r\ncould not open %s file for reading", file);
	  }
	  // Reads line by line until the end
	  if(f_read (config->fil, buffer, f_size(config->fil), &br) != FR_OK){

		  printf("\r\ncould not read %s file", file);
	  }

	  printf("\r\nread dat: %s", buffer);

	  f_close(config->fil);

}
