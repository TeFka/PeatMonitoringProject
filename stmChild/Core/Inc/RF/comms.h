/*
 * comms.hpp
 *
 *  Created on: Dec 26, 2022
 *      Author: James
 */

#ifndef INC_COMMS_H_
#define INC_COMMS_H_

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MMIO8(addr) (*(volatile uint8_t *)(addr))
#define U_ID 0x1FFF7590

#define MSG_SIZE_TRANSFER 20
#define MAX_HOPS 5
#define MAX_NUM_FWDS 20

#define MSG_PREAMBLE_POS 0
#define MSG_TYPE_POS 1
#define MSG_TO_ID_POS 2
#define MSG_FROM_ID_POS 3
#define MSG_NUM_HOPS_POS 4
#define MSG_SIZE_POS 5

#define MSG_BODY_START 6
#define MSG_BODY END 8
#define MSG_END_TRANSFER_POS 9

#define MSG_PREAMBLE 0xFE
#define MSG_END 0xFF

#define DEVICE_PARENT 0x00
#define DEVICE_CHILD 0x99

//Child/Mother control commands
#define MSG_TYPE_IM_HERE 0x43
#define MSG_TYPE_YOUR_THERE 0x45
#define MSG_TYPE_SEND_DATA 0x46
#define MSG_TYPE_GIVE_DATA 0x47
#define MSG_TYPE_DONE 0x48

//remote control commands
#define MSG_TYPE_GIVE_BROKEN_DATA 0x50
#define MSG_TYPE_GIVE_STATUS_DATA 0x51
#define MSG_TYPE_GIVE_LOW_BATTERY_DATA 0x52
#define MSG_TYPE_GIVE_MEASUREMENT_DATA 0x53
#define MSG_TYPE_REMOTE_DONE 0x54

#define MAX_SIZE 150

struct Queue{
  int data[MAX_SIZE];
  int head;
  int tail;
  int size;
};


void create_queue(struct Queue* queue);
void destroy_queue(struct Queue* queue);
bool is_full(struct Queue* queue);
bool is_empty(struct Queue* queue);
void enqueue(struct Queue* queue, uint8_t value);
int dequeue(struct Queue* queue);

struct device
{
	//uint8_t unique_id[4];
	uint8_t device_id;
	uint8_t device_type;
	bool device_status;
	bool device_is_discovered;
	bool dataRequest;
};

struct messages
{
	uint8_t from_id;
	uint8_t to_id;
	uint8_t type;
};

struct newDevicesStorage{

	uint8_t numOfNewDevices;
	uint16_t newDevices[50];

};

struct rfDataStorage{

	struct Queue* RXFIFO;
	struct device myDevice;
	uint8_t count;
	uint8_t num_fwds;
	struct messages FWDFIFO[20];

	uint8_t activeTxMessageSize;
	uint8_t activeTxMessage[20];

	uint8_t latestRequestIDs[20];
	uint8_t atestRequestIDNum;

	uint8_t activeRxMessageSize;
	uint8_t activeRxMessage[10][20];
	uint8_t messageNum;

	struct newDevicesStorage newDevices;
};

void init_comms(struct rfDataStorage* rfData, uint8_t deviceType, int id, int discovered);		//must be called in setup to initialise comms variables
void Comms_Handler(struct rfDataStorage* rfData);						//Handles all communications, call in main loop.

void Discovery_Handler(struct rfDataStorage* rfData); 				//Discover handler with no arguments (not yet received a message)
void Discovery_Handler_Message(struct rfDataStorage* rfData); 	//^ with argument.
void Message_Handler(struct rfDataStorage* rfData);	//Message handler, called within comms handler
void Message_Forwarder(struct rfDataStorage* rfData);	//Message forwarder, ^
void Im_Here(struct rfDataStorage* rfData);								//Function to send Im here message, ^
void Youre_There(struct rfDataStorage* rfData, uint8_t device_id);
void Send(uint8_t message[], uint8_t size);				//Send function which is passed message arg
void Send_Data(struct rfDataStorage* rfData);
void random_delay();						//Creates a random delay, used for reducing message 'collisions' on the network

#endif /* INC_COMMS_H_ */
