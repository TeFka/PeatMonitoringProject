/*
 * FlashMemoryAccess.c
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#include "../Inc/FlashMemoryAccess.h"

//Function to erase data from defined pages
uint32_t Flash_Erase_Data (uint32_t startAddress, uint16_t pages){

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;

	//Starting address holder
	uint32_t address = 0;
	address = startAddress;

	/* Unlock the Flash to enable the flash control register access *************/
   HAL_FLASH_Unlock();
   __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

   /* Erase the user Flash area*/
   if(address<FLASH_BASE || address>FLASH_END-FLASH_PAGE_SIZE) return 0;

   //uint32_t StartPage = (StartAddress-FLASH_BASE)/FLASH_PAGE_SIZE;
   /* Fill EraseInit structure*/
   EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; //tyep of erasure
   EraseInitStruct.Page = (address-FLASH_BASE)/FLASH_PAGE_SIZE; //number of pages to erase
   EraseInitStruct.NbPages = pages; //number of page to erase
   EraseInitStruct.Banks = FLASH_BANK_1; //active memory bank

   //erase the relevant pages that will be used
   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
   {
	 /*Error occurred while page erase.*/
	   HAL_FLASH_Lock();
	  return HAL_FLASH_GetError ();
   }
   /* Lock the Flash to disable the flash control register access (recommended
		  to protect the FLASH memory against possible unwanted operation) *********/
   HAL_FLASH_Lock();

   return HAL_OK;

}

//Function to write data at specified address
uint32_t Flash_Write_Data (uint32_t startAddress, uint64_t *data, int numberOfWords)
{

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	int sofar=0;

	//Starting address holder
	uint32_t address = 0;
	address = startAddress;


	  /* Unlock the Flash to enable the flash control register access *************/
	   HAL_FLASH_Unlock();
	   __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

	   /* Erase the user Flash area*/
	   if(address<FLASH_BASE || address>FLASH_END-FLASH_PAGE_SIZE) return 0;

	   //uint32_t StartPage = (StartAddress-FLASH_BASE)/FLASH_PAGE_SIZE;
	   /* Fill EraseInit structure*/
	   EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; //tyep of erasure
	   EraseInitStruct.Page = (address-FLASH_BASE)/FLASH_PAGE_SIZE; //number of pages to erase
	   EraseInitStruct.NbPages = ((numberOfWords*8)/FLASH_PAGE_SIZE)+1; //number of page to erase
	   EraseInitStruct.Banks = FLASH_BANK_1; //active memory bank

	   //erase the relevant pages that will be used
	   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	   {
		 /*Error occurred while page erase.*/
		   HAL_FLASH_Lock();
		  return HAL_FLASH_GetError ();
	   }

	   /* Program the user Flash area word by word*/
	 while (sofar<numberOfWords)
	 {
		 //program a double word to the page one at a time
		 if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data[sofar]) == HAL_OK)
		 {
			 address += 8;  // use StartPageAddress += 2 for half word and 8 for double word
			 sofar++;
		 }
		 else{
		     /* Error occurred while writing data in Flash memory*/
			 HAL_FLASH_Lock();
			 return HAL_FLASH_GetError ();
		 }
	 }

	   /* Lock the Flash to disable the flash control register access (recommended
	      to protect the FLASH memory against possible unwanted operation) *********/
	   HAL_FLASH_Lock();

	   return HAL_OK;
}

//Function to read data from specified address
void Flash_Read_Data (uint32_t StartPageAddress, uint64_t *RxBuf, uint16_t numberofwords)
{
	//iterate over memory location
	while (1)
	{
		//add value from address to array
		*RxBuf = *(__IO uint64_t *)StartPageAddress;
		//increment active address
		StartPageAddress += 8;
		//increment active array element
		RxBuf++;
		//decrement number of words left to read
		numberofwords--;
		//stop reading if no more words are needed
		if (!(numberofwords)) break;
	}
}
