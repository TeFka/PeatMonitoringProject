# Code for separate modules<br>
### Covers code for separate moduels that would then be used together, the modules include<br>
1. Depth measurement logic - logic for using analogue Hall magnet sensors for surface elevation measurements. The sensors would provide measurements from magnets on the elevation sensor and this way determine the current surface movement. The modeul code includes calibrartion values that are needed for accurate measurements.<br>
2. Pressure sensor interaction module - the code includes C wrapper for STM32 that allows to communicate with LPS28DFW pressure sensor using I2C. Covers all relevant operations an dinteractions with registers.<br>
3. Temperature sensor interaction module - the code includes C wrapper for STM32 that allows to communicate with TMP126-Q1 temperature sensor using I2C. Covers all relevant operations an dinteractions with registers.<br>
4. SD card interacttion module - the code allows to interact with an SD card using basic SPI commands. It is meant to work with FATFS support and can be used directly with a basic SD card adapter.<br>
5. STM32 flash memory inteaction module - the code allows to easilty interact with the STM32 flash memory and store data at specific locations.<br>
6. RF networking logic module - the code allows to automatically handle RF communication logic of the device system. The main feature is the ability for the device to act as a data proxy for another device and this way allowing devives to communicate with further devices. When device sends data, it also sends parameters that allow proxy devices to handle data correctly, such as target device or the number of bounces the data had. The module allows to create the communication network that the system needs.<br>


