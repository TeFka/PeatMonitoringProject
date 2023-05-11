/*
 * FlashMemoryAccess.h
 *
 *  Created on: Feb 4, 2023
 *      Author: Admin
 */

#include "stm32l4xx_hal.h"


#ifndef INC_FLASHMEMORYACCESS_H_
#define INC_FLASHMEMORYACCESS_H_

#include <stdio.h>
#include <string.h>
#include "retarget.h"

#define USABLE_FLASH_START 	FLASH_BASE+FLASH_PAGE_SIZE*50
#define FLASH_PAGE_NUM 		64

uint32_t Flash_Erase_Data(uint32_t startAddress, uint16_t pages);
uint32_t Flash_Write_Data (uint32_t, uint64_t*, int);
void Flash_Read_Data (uint32_t, uint64_t*, uint16_t);

#endif /* INC_FLASHMEMORYACCESS_H_ */
