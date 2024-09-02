# Combined STM32 projects<br>
### Covers combined projects, each involving several separate modules. The maind components are:<br>
* Code for Mother device - device meant for data storage.<br>
* Code for Child device - device meant for data grathering.<br>
* Code for remote connector - device meant for data retrieval.<br>
### The overall operation logic goes as follows:<br>
1. The child device gathers data using available sensors.<br>
2. The child device sends data to the mother device using RF communicaton, if the mother device is too far away, other device acts as a proxy.<br>
3. The mother devoce receives the data from child device and saves it into an SD card.<br>
4. The operation repeats itself at specific intervals and for a required amount of time.<br>
5. The user comes to the mother device and uses the remote device to communicate with mother device and the mother device transfers all stored data to the remote.<br>



