

#ifndef COMMS_PROTOCOL_H
#define COMMS_PROTOCOL_H

#include <Arduino.h>
#include <SPI.h>              
#include <LoRa.h>
#include "gateway_serial_definitions.h"
#include <cppQueue.h>
#include <aes256.h>

#define  IMPLEMENTATION  FIFO

// LoRa msg payload settings
#define RELAY_INTERVAL 200
#define MAX_JSON_PAYLOAD_SIZE 100
#define MAX_R_QUEUE_SIZE 1
#define MAX_QUEUE_SIZE 5
#define MAX_N_RETRY 5
#define TIMEOUT_INTERVAL 3000

#define BLOCK_SIZE 16
#define MAX_PAYLOAD_SIZE 16
#define ENC_BLOCK_SIZE (2*BLOCK_SIZE)
#define MAX_ENC_PAYLOAD_SIZE ((MAX_PAYLOAD_SIZE/BLOCK_SIZE)*ENC_BLOCK_SIZE)+1

#define MAX_MSG_ID 256

// Encryption key
extern uint8_t key[];

// LoRa Modem Settings
const long frequency = 868E6;
const int txPower = 14;
const int spreadingFactor = 7;
const long signalBandwidth = 125E3;
const int codingRateDenominator = 5;

typedef struct strPayload {
  byte nodeID;
  byte sensorID;
  byte sensorVal;
  byte msgID;
  char flag;
  int RSSI;
  float SNR;
} Payload;

typedef struct strMsg {
  char msg[MAX_ENC_PAYLOAD_SIZE];
  byte msgID;
  char flag;
  byte nodeID;
  byte actID;
  byte actVal;
} Msg;

extern int currMsg;
extern int count;
extern unsigned long prevMilR;
extern unsigned long prevMil;

extern cppQueue relay_q;
extern cppQueue msg_q;
extern aes256_context ctxt;

void LoRa_rxMode();
void LoRa_txMode();
void LoRa_sendMessage(String message);
char  *decryptMsg(String msg);
void onReceive(int packetSize);
void onTxDone();
String splitAndEncrypt(char msg[MAX_PAYLOAD_SIZE]);
void sendAck(byte msgID, byte nodeID);
void relayMsgFromQueueToServer(unsigned long currentMillis);
void constructJsonAndAddToQueue(Payload p);
void relayDownlinkMsg(char *dlMsg);
void getMsgFromQueueAndSend(unsigned long currentMillis);
void sendStatusRequest(byte nodeID);
void sendActuatorControl(byte nodeID, byte actID, byte actVal);

#endif
