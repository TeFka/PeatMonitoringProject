/*
 * comms.cpp
 *
 *  Created on: 27 Dec 2022
 *      Author: James Bodenham
 */

#include <RF/comms.h>

//uint8_t devices;
//device rfData->myDevice;

void init_comms(struct rfDataStorage* rfData, uint8_t deviceType, int id, int discovered)
{

	create_queue(rfData->RXFIFO);
	rfData->myDevice.device_id = id;
	rfData->myDevice.device_type = deviceType;
	rfData->myDevice.device_is_discovered = discovered;

	rfData->count = 0;
	rfData->num_fwds = 0;

	for(int i = 0;i<MAX_NUM_FWDS;i++){

		rfData->FWDFIFO[i].from_id = 0;
		rfData->FWDFIFO[i].to_id = 0;
		rfData->FWDFIFO[i].type = 0;

	}

}

void Comms_Handler(struct rfDataStorage* rfData)
{
	if (rfData->myDevice.device_type == DEVICE_CHILD)					//Child Handler
	{
		uint8_t data = 0;
		rfData->messageNum = 0;
		while((is_empty(rfData->RXFIFO)) == false)					//until the FIFO is empty
		{
			data = dequeue(rfData->RXFIFO);					//read from FIFO
			if (data == MSG_PREAMBLE)					//is beginning of a rfData->activeMessage?
			{
				rfData->activeRxMessage[rfData->messageNum][MSG_PREAMBLE_POS] = MSG_PREAMBLE;

				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] = data;

				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] = data;

				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS] = data;

				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS] = data;
				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS] = data;

				for(int i = 0;i<rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS];i++)
				{
					data = dequeue(rfData->RXFIFO);
					rfData->activeRxMessage[rfData->messageNum][MSG_BODY_START+i] = data;

				}
				printf("\r\nID comparison: %d %d", rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS], rfData->myDevice.device_id);

				if (rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] != rfData->myDevice.device_id)
				{
						Message_Forwarder(rfData);
				}

				int newMessage = 1;
				if(rfData->messageNum>0){
					for(int i = 1;i<=rfData->messageNum;i++){
						if(rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_TYPE_POS]&&
							rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_FROM_ID_POS]&&
							rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_TO_ID_POS]){
								newMessage = 0;
							}
					}
				}
				if(newMessage){

					rfData->messageNum++;

				}
			}
		}

		if(rfData->myDevice.device_is_discovered == false)			//if not discovered and theres no rfData->activeMessages
		{
			//Im_Here(rfData);
			Discovery_Handler(rfData);						//go to discovery handler
		}
	}
	else if (rfData->myDevice.device_type == DEVICE_PARENT)			//Parent Handler
	{
		uint8_t data;
		rfData->messageNum = 0;
		while((is_empty(rfData->RXFIFO)) == false)					//until the FIFO is empty
		{
			data = dequeue(rfData->RXFIFO);					//read from FIFO
			if (data == MSG_PREAMBLE)					//is beginning of a rfData->activeMessage?
			{
				rfData->activeRxMessage[rfData->messageNum][MSG_PREAMBLE_POS] = MSG_PREAMBLE;
				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] = data;
				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] = data;
				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS] = data;

				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS] = data;
				data = dequeue(rfData->RXFIFO);
				rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS] = data;

				for(int i = 0;i<rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS];i++)
				{
					data = dequeue(rfData->RXFIFO);
					rfData->activeRxMessage[rfData->messageNum][MSG_BODY_START+i] = data;

				}

				int newMessage = 1;
				if(rfData->messageNum>0){
					for(int i = 1;i<=rfData->messageNum;i++){
						if(rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_TYPE_POS]&&
							rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_FROM_ID_POS]&&
							rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] == rfData->activeRxMessage[rfData->messageNum-i][MSG_TO_ID_POS]){
								newMessage = 0;
							}
					}
				}
				if(newMessage){

					rfData->messageNum++;

				}
			}
		}

	}

}

void Discovery_Handler(struct rfDataStorage* rfData)
{
	HAL_Delay(200);
	if(rfData->count == 0)
	{
		Im_Here(rfData);								//send i'm here rfData->activeMessage
		rfData->count = 5;
	}
	else
	{
		rfData->count--;
	}

}

void Message_Handler(struct rfDataStorage* rfData)
{
	if (rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] != rfData->myDevice.device_id)
	{
			Message_Forwarder(rfData);
	}

}

void Message_Forwarder(struct rfDataStorage* rfData)
{
	//printf("\r\nForwarding message, fwds: %d", rfData->num_fwds);
	if( (rfData->num_fwds == 0) && (rfData->activeRxMessage[MSG_NUM_HOPS_POS] < MAX_HOPS) )
	{
		rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS]++;
		Send(rfData->activeRxMessage[rfData->messageNum], rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS]+MSG_BODY_START);
		rfData->FWDFIFO[rfData->num_fwds].from_id = rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS];
		rfData->FWDFIFO[rfData->num_fwds].to_id = rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS];
		rfData->FWDFIFO[rfData->num_fwds].type = rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS];
		rfData->num_fwds++;
	}
	else if ( rfData->activeRxMessage[MSG_NUM_HOPS_POS] < MAX_HOPS )
	{
		int dupe = false;
		for (int i = 0; i < rfData->num_fwds; i++)
		{

			if ( rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS] == rfData->FWDFIFO[i].from_id )
			{
				if ( rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS] == rfData->FWDFIFO[i].to_id )
				{
					if ( rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS] == rfData->FWDFIFO[i].type )
					{
						dupe = true;
						break;
					}
				}
			}
		}
		if (dupe == false)
		{
			if ( rfData->num_fwds == MAX_NUM_FWDS ){rfData->num_fwds = 0;}
			rfData->FWDFIFO[rfData->num_fwds].from_id = rfData->activeRxMessage[rfData->messageNum][MSG_FROM_ID_POS];
			rfData->FWDFIFO[rfData->num_fwds].to_id = rfData->activeRxMessage[rfData->messageNum][MSG_TO_ID_POS];
			rfData->FWDFIFO[rfData->num_fwds].type = rfData->activeRxMessage[rfData->messageNum][MSG_TYPE_POS];

			rfData->activeRxMessage[rfData->messageNum][MSG_NUM_HOPS_POS]++;
			Send(rfData->activeRxMessage[rfData->messageNum], rfData->activeRxMessage[rfData->messageNum][MSG_SIZE_POS]+MSG_BODY_START);
		}

	}
	//printf("Forwarding done");
}

void Im_Here(struct rfDataStorage* rfData)
{

	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_IM_HERE;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessageSize = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;

	//printf("\r\nsending from device: %d->%d", rfData->myDevice.device_id, rfData->activeTxMessage[MSG_FROM_ID_POS]);

	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

}

void Send_Data(struct rfDataStorage* rfData)
{
	//printf("\r\nsending data");
	rfData->activeTxMessage[MSG_PREAMBLE_POS] = MSG_PREAMBLE;
	rfData->activeTxMessage[MSG_TYPE_POS] = MSG_TYPE_SEND_DATA;
	rfData->activeTxMessage[MSG_FROM_ID_POS] = rfData->myDevice.device_id;
	rfData->activeTxMessage[MSG_NUM_HOPS_POS] = 0;
	rfData->activeTxMessage[MSG_SIZE_POS] = rfData->activeTxMessageSize;
	Send(rfData->activeTxMessage, rfData->activeTxMessageSize+MSG_BODY_START);

}


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
	/*for(int i = 0 ;i<MSG_SIZE_TRANSFER;i++){

		printf("\r\nsending rf: %x", fullMessage[i]);

	}*/
	random_delay();
	HAL_UART_Transmit_IT(&hlpuart1, fullMessage, MSG_SIZE_TRANSFER);
	HAL_Delay(20);

}

void random_delay()	//delays between 0 - 9999 (ms)
{
	uint32_t randomNumber;
	HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber);
	uint32_t delay = randomNumber % 1000;
	HAL_Delay(delay);
}

void create_queue(struct Queue* queue) {
  queue->head = 0;
  queue->tail = 0;
  queue->size = 0;
}

void destroy_queue(struct Queue* queue) {
  free(queue);
}

bool is_full(struct Queue* queue) {
  return (queue->size == MAX_SIZE);
}

bool is_empty(struct Queue* queue) {
  return (queue->size == 0);
}

void enqueue(struct Queue* queue, uint8_t value) {
  if (is_full(queue)) {
    //printf("Queue is full\n");
    return;
  }
  queue->data[queue->tail] = value;
  queue->tail = (queue->tail + 1) % MAX_SIZE;
  queue->size++;
}

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



