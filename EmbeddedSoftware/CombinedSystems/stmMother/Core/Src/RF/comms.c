/*
 * comms.cpp
 *
 *  Created on: 27 Dec 2022
 *      Author: James Bodenham
 */

#include <RF/comms.h>

//uint8_t devices;
//device rfData->myDevice;

//Initialization of communications
void init_comms(struct rfDataStorage* rfData, uint8_t deviceType, int id, int discovered)
{
	//Create a fifo queuq
	create_queue(rfData->RXFIFO);

	//Set device data
	rfData->myDevice.device_id = id;
	rfData->myDevice.device_type = deviceType;
	rfData->myDevice.device_is_discovered = discovered;

	//Set initila forwarding data
	rfData->count = 0;
	rfData->num_fwds = 0;

	//Set initial fifo data
	for(int i = 0;i<MAX_NUM_FWDS;i++){

		rfData->FWDFIFO[i].from_id = 0;
		rfData->FWDFIFO[i].to_id = 0;
		rfData->FWDFIFO[i].type = 0;

	}

}

//The function to handle all communication traffic
void Comms_Handler(struct rfDataStorage* rfData)
{
	//If device is child
	if (rfData->myDevice.device_type == DEVICE_CHILD)					//Child Handler
	{
		//data holder
		uint8_t data = 0;

		//Refresh number of active messages
		rfData->messageNum = 0;

		//Iterate through fifo
		while((is_empty(rfData->RXFIFO)) == false)					//until the FIFO is empty
		{
			//Dequeue first data point
			data = dequeue(rfData->RXFIFO);					//read from FIFO
			if (data == MSG_PREAMBLE)					//is beginning of a rfData->activeMessage?
			{

				//Set preamble message
				rfData->activeRxMessage[rfData->messageNum][MSG_PREAMBLE_POS] = MSG_PREAMBLE;
				for(int i = 1;i<MSG_BODY_START;i++)
				{
					//Add control data to rx message
					rfData->activeRxMessage[rfData->messageNum][i] = dequeue(rfData->RXFIFO);

				}

				//assign destination ID
				rfData->toID[rfData->messageNum] = bit8Comb(rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS1], rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS2]);

				//assign sender ID
				rfData->fromID[rfData->messageNum] = bit8Comb(rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS1], rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS2]);

				//Retrieve message based on specified size
				for(int i = 0;i<rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS];i++)
				{
					data = dequeue(rfData->RXFIFO);
					rfData->activeRxMessage[rfData->messageNum][MSG_BODY_START+i] = data;

				}

				//Handle message
				Message_Handler(rfData);

				int newMessage = 1;
				//Check if the received message was not received before during this fifo handling
				if(rfData->messageNum>0){
					for(int i = 1;i<=rfData->messageNum;i++){
						//check message type, destination ID and sender ID for comparison
						if(rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_TYPE_POS]&&
						   rfData->fromID[rfData->messageNum] == rfData->fromID[rfData->messageNum-i]&&
						   rfData->toID[rfData->messageNum] == rfData->toID[rfData->messageNum-i]){
								newMessage = 0;
							}
					}
				}
				//Increment number of messages if new message was received
				if(newMessage){

					rfData->messageNum++;

				}

			}
		}

		//Perform discovery
		if(rfData->myDevice.device_is_discovered == false)			//if not discovered and theres no rfData->activeMessages
		{
			//Im_Here(rfData);
			Discovery_Handler(rfData);						//go to discovery handler
		}
	}
	else if (rfData->myDevice.device_type == DEVICE_PARENT)			//Parent Handler
	{
		//data holder
		uint8_t data = 0;

		//Refresh number of active messages
		rfData->messageNum = 0;

		//Iterate through fifo
		while((is_empty(rfData->RXFIFO)) == false)					//until the FIFO is empty
		{
			//Dequeue first data point
			data = dequeue(rfData->RXFIFO);					//read from FIFO
			if (data == MSG_PREAMBLE)					//is beginning of a rfData->activeMessage?
			{
				//Set preamble message
				rfData->activeRxMessage[rfData->messageNum][MSG_PREAMBLE_POS] = MSG_PREAMBLE;
				for(int i = 1;i<MSG_BODY_START;i++)
				{
					//Add control data to rx message
					rfData->activeRxMessage[rfData->messageNum][i] = dequeue(rfData->RXFIFO);

				}

				//assign destination ID
				rfData->toID[rfData->messageNum] = bit8Comb(rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS1], rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS2]);

				//assign sender ID
				rfData->fromID[rfData->messageNum] = bit8Comb(rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS1], rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS2]);

				//Retrieve message based on specified size
				for(int i = 0;i<rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS];i++)
				{
					data = dequeue(rfData->RXFIFO);
					rfData->activeRxMessage[rfData->messageNum][MSG_BODY_START+i] = data;

				}

				int newMessage = 1;
				//Check if the received message was not received before during this fifo handling
				if(rfData->messageNum>0){
					for(int i = 1;i<=rfData->messageNum;i++){
						//check message type, destination ID and sender ID for comparison
						if(rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_TYPE_POS]&&
						   rfData->fromID[rfData->messageNum] == rfData->fromID[rfData->messageNum-i]&&
						   rfData->toID[rfData->messageNum] == rfData->toID[rfData->messageNum-i]){
								newMessage = 0;
							}
					}
				}
				//Increment number of messages if new message was received
				if(newMessage){

					rfData->messageNum++;

				}
			}
		}

	}

}

//Function to handle device discovery
void Discovery_Handler(struct rfDataStorage* rfData)
{
	HAL_Delay(200);
	if(rfData->count == 0)
	{
		Im_Here(rfData);								//send i'm here rfData->activeMessage
		rfData->count = 10;
	}
	else
	{
		rfData->count--;
	}

}

//Function to perform any unique message handling
void Message_Handler(struct rfDataStorage* rfData)
{
	//Check if the device is the destination of the message
	if (rfData->toID[rfData->messageNum] != rfData->myDevice.device_id)
	{
		//perform message forwarding
			Message_Forwarder(rfData);
	}

}

void Message_Forwarder(struct rfDataStorage* rfData)
{
	printf("\r\nForwarding message");
	//Check if message should be forwarded
	if( (rfData->num_fwds == 0) && (rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS] < rfData->activeRxMessage[rfData->messageNum][MSG_MAX_HOPS_POS]) )
	{
		//increment number of hops
		rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS]++;
		//Send tehe forwarded data
		Send(rfData->activeRxMessage[rfData->messageNum], rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS]+MSG_BODY_START);
		//Add data to fifo FWD
		rfData->FWDFIFO[rfData->num_fwds].from_id = rfData->fromID[rfData->messageNum];
		rfData->FWDFIFO[rfData->num_fwds].to_id = rfData->toID[rfData->messageNum];
		rfData->FWDFIFO[rfData->num_fwds].type = rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS];
		//Increment numbe rof FWDs
		rfData->num_fwds++;
	}
	else if ( rfData->activeRxMessage[MSG_NUM_HOPS_POS] < rfData->activeRxMessage[rfData->messageNum][MSG_MAX_HOPS_POS] )
	{
		int dupe = false;
		//Iterate through all FWDs and hceck them
		for (int i = 0; i < rfData->num_fwds; i++)
		{
			//Check sensder ID
			if ( rfData->fromID[rfData->messageNum] == rfData->FWDFIFO[i].from_id )
			{
				//Check destination ID
				if ( rfData->toID[rfData->messageNum] == rfData->FWDFIFO[i].to_id)
				{
					//Check message type
					if ( rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] == rfData->FWDFIFO[i].type )
					{
						//determine that message should not be forwarded
						dupe = true;
						break;
					}
				}
			}
		}
		//Forward emssage if required
		if (dupe == false)
		{
			//refresh FWDs if required
			if ( rfData->num_fwds == MAX_NUM_FWDS ){rfData->num_fwds = 0;}
			//Add data to fifo FWD
			rfData->FWDFIFO[rfData->num_fwds].from_id = rfData->fromID[rfData->messageNum];
			rfData->FWDFIFO[rfData->num_fwds].to_id = rfData->toID[rfData->messageNum];
			rfData->FWDFIFO[rfData->num_fwds].type = rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS];
			//increment number of hops
			rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS]++;
			//Send tehe forwarded data
			Send(rfData->activeRxMessage[rfData->messageNum], rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS]+MSG_BODY_START);
		}

	}
}

//Function to send "I am here" message
void Im_Here(struct rfDataStorage* rfData)
{
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_IM_HERE;
	rfData->activeTxMessage[MSG_FROM_ID_POS1] = rfData->myDevice.device_id&0x0F;
	rfData->activeTxMessage[MSG_FROM_ID_POS2] = rfData->myDevice.device_id&0xF0;
	rfData->activeTxMessage[MSG_TO_ID_POS1] = 0;
	rfData->activeTxMessage[MSG_TO_ID_POS2] = 0;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_MAX_HOPS_POS] = 10;
	rfData->activeTxMessageSize = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

}

//Function to send data through RF
void Send(uint8_t message[], uint8_t size)
{
	//printf("Send comamnd");
	uint8_t fullMessage[MSG_SIZE_TRANSFER];

	for(int i = 0;i<size;i++){
		fullMessage[i] = message[i];
	}
	for(int i = size;i<MSG_SIZE_TRANSFER;i++){
		fullMessage[i] = 0;
	}
	for(int i = 0 ;i<MSG_SIZE_TRANSFER;i++){

		printf("\r\nsending rf: %x", fullMessage[i]);

	}
	random_delay();
	HAL_UART_Transmit_IT(&hlpuart1, fullMessage, MSG_SIZE_TRANSFER);
	HAL_Delay(20);

}

//Function to add random delay
void random_delay()	//delays between 0 - 9999 (ms)
{
	uint32_t randomNumber;
	HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber);
	uint32_t delay = randomNumber % 1000;
	HAL_Delay(delay);
}

//Function to create a new queue for operation
void create_queue(struct Queue* queue) {
  queue->head = 0;
  queue->tail = 0;
  queue->size = 0;
}

//Function to destroy a queue
void destroy_queue(struct Queue* queue) {
  free(queue);
}

//Function check if queue is full
bool is_full(struct Queue* queue) {
  return (queue->size == MAX_SIZE);
}

//Function to check if queue is empty
bool is_empty(struct Queue* queue) {
  return (queue->size == 0);
}

//Function to add a value to queue
void enqueue(struct Queue* queue, uint8_t value) {
  if (is_full(queue)) {
    //printf("Queue is full\n");
    return;
  }
  queue->data[queue->tail] = value;
  queue->tail = (queue->tail + 1) % MAX_SIZE;
  queue->size++;
}

//Function to dequeue latest value from the queue
int dequeue(struct Queue* queue) {
  if (is_empty(queue)) {
    //printf("Queue is empty\n");
    return -1;
  }
  int value = queue->data[queue->head];
  queue->head = (queue->head + 1) % MAX_SIZE;
  queue->size--;
  return value;
}



