/*
 * FlashMemoryAccess.c
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#include "../Inc/FlashMemoryAccess.h"

uint32_t Flash_Write_Data (uint32_t startAddress, uint64_t *data, int numberOfWords)
{

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	int sofar=0;

	uint32_t address = 0;

	address = startAddress;


	  /* Unlock the Flash to enable the flash control register access *************/
	   HAL_FLASH_Unlock();
	   __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

	   /* Erase the user Flash area*/
	   if(address<FLASH_BASE || address>FLASH_END-FLASH_PAGE_SIZE) return 0;

	   //uint32_t StartPage = (StartAddress-FLASH_BASE)/FLASH_PAGE_SIZE;
	   /* Fill EraseInit structure*/
	   EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	   EraseInitStruct.Page = (address-FLASH_BASE)/FLASH_PAGE_SIZE;
	   EraseInitStruct.NbPages = 2;//(numberOfWords/FLASH_PAGE_SIZE) +1;
	   EraseInitStruct.Banks = FLASH_BANK_1;
	   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	   {
	     /*Error occurred while page erase.*/
		  return HAL_FLASH_GetError ();
	   }

	   /* Program the user Flash area word by word*/
	 while (sofar<numberOfWords)
	 {
		 if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data[sofar]) == HAL_OK)
		 {
			 address += 8;  // use StartPageAddress += 2 for half word and 8 for double word
			 sofar++;
		 }
		 else{
		     /* Error occurred while writing data in Flash memory*/
			 return HAL_FLASH_GetError ();
		 }
	 }

	   /* Lock the Flash to disable the flash control register access (recommended
	      to protect the FLASH memory against possible unwanted operation) *********/
	   HAL_FLASH_Lock();

	   return HAL_OK;
}

void Flash_Read_Data (uint32_t StartPageAddress, uint64_t *RxBuf, uint16_t numberofwords)
{
	while (1)
	{

		*RxBuf = *(__IO uint64_t *)StartPageAddress;
		StartPageAddress += 8;
		RxBuf++;
		numberofwords--;
		if (!(numberofwords)) break;
	}
}
