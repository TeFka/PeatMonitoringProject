/*
 * TemperatureSensing.c
 *
 *  Created on: Feb 10, 2023
 *      Author: Admin
 */

#include <Temperature/TemperatureSensing.h>

void sendSPI(SPI_HandleTypeDef* handle, uint8_t controlVal, uint8_t registerAddr, uint8_t messageHigh, uint8_t messageLow){

  uint8_t spi_buf1[2];
  uint8_t spi_buf2[2];

  // Bytes to write to EEPROM
  spi_buf1[0] = registerAddr;
  spi_buf1[1] = registerAddr;
  spi_buf2[0] = messageHigh;
  spi_buf2[1] = messageLow;

  HAL_GPIO_WritePin(TPSPI_CSGP, TPSPI_CSPin, GPIO_PIN_RESET);
  HAL_Delay(50);
  if (HAL_SPI_Transmit(handle, (uint8_t *)spi_buf1, 2, 100) != HAL_OK){

	  printf("SPI transmit1 error");

  }

  if (HAL_SPI_Transmit(handle, (uint8_t *)spi_buf2, 2, 100) != HAL_OK){

	  printf("SPI transmit2 error");

  }
  HAL_GPIO_WritePin(TPSPI_CSGP, TPSPI_CSPin, GPIO_PIN_SET);
  HAL_Delay(50);

}

int getSPI(SPI_HandleTypeDef* handle, uint8_t controlVal, uint8_t registerAddr){

  uint8_t spi_buf[2];
  uint8_t spi_recv_buf[2];
  uint16_t receivedHighByte = 0;
  uint16_t receivedLowByte = 0;
  uint16_t receivedData = 0;

  spi_buf[0] = controlVal;
  spi_buf[1] = registerAddr;
  HAL_GPIO_WritePin(TPSPI_CSGP, TPSPI_CSPin, GPIO_PIN_RESET);
  HAL_Delay(50);
  if (HAL_SPI_Transmit(handle, (uint8_t *)spi_buf, 2, 100) != HAL_OK){

	  printf("SPI transmitReceive error");

  }
  if (HAL_SPI_Receive(handle, (uint8_t *)spi_recv_buf, 2, 100) != HAL_OK){

	  printf("SPI receive error");

  }
  HAL_GPIO_WritePin(TPSPI_CSGP, TPSPI_CSPin, GPIO_PIN_SET);
  HAL_Delay(50);

  receivedHighByte = spi_recv_buf[0];
  receivedLowByte = spi_recv_buf[1];

  receivedData = (receivedHighByte<<8)|receivedLowByte;

  return receivedData;
}

void configureSensor(SPI_HandleTypeDef* handle){

  sendSPI(handle, TPwriteMask, TPconfigurationAddr, TPdefaultConfigHigh, TPdefaultConfigLow);

}

int temp_getID(SPI_HandleTypeDef* handle){

	return getSPI(handle, TPreadMask, TPuniqueID1Addr);

}

double getTemperatureVal(SPI_HandleTypeDef* handle){

  int receivedValue = 0;

  receivedValue = getSPI(handle, TPreadMask, TPtemperatureResultAddr);

  receivedValue = receivedValue >> 2;

  return (double)receivedValue*temperatureSensetivity;
}
