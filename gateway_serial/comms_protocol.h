/*
 * Header file for the communication protocol library. Contains the used data structures,
 * function declaration and general configuartion options for the library
 */

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
#define RELAY_INTERVAL 500
#define MAX_JSON_PAYLOAD_SIZE 120
#define MAX_R_QUEUE_SIZE 2
#define MAX_QUEUE_SIZE 5
#define MAX_N_RETRY 5
#define TIMEOUT_INTERVAL 2000

#define BLOCK_SIZE 16
#define MAX_PAYLOAD_SIZE 16
#define ENC_BLOCK_SIZE (1*BLOCK_SIZE)
#define MAX_ENC_PAYLOAD_SIZE ((MAX_PAYLOAD_SIZE/BLOCK_SIZE)*ENC_BLOCK_SIZE)+1
#define KEY_SIZE 32

#define MAX_MSG_ID 256

#define BROADCAST_ID 0xFF

// Encryption keys
const uint8_t keys[][KEY_SIZE] = {{ //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x0f
},{ //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
},
{ //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x2f
},
{ //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x3f
},{ //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x4f
}};

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
  float VBAT;
  double milis;
} Payload;

typedef struct strMsg {
  byte msg[MAX_ENC_PAYLOAD_SIZE];
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
extern int msgCount;

extern cppQueue relay_q;
extern cppQueue msg_q;
extern aes256_context ctxt;

void LoRa_rxMode();
void LoRa_txMode();
void LoRa_sendMessage(byte *message, byte nodeID);
char  *decryptMsg(String msg);
char  *decryptMsg2(char msg[MAX_PAYLOAD_SIZE+1]);
void onReceive(int packetSize);
void onTxDone();
String splitAndEncrypt(char msg[MAX_PAYLOAD_SIZE]);
byte *splitAndEncrypt2(char msg[MAX_PAYLOAD_SIZE]);
void sendAck(byte msgID, byte nodeID);
void relayMsgFromQueueToServer(unsigned long currentMillis);
void constructJsonAndAddToQueue(Payload p);
void relayDownlinkMsg(char *dlMsg);
void getMsgFromQueueAndSend(unsigned long currentMillis);
void sendStatusRequest(byte nodeID);
void sendActuatorControl(byte nodeID, byte actID, byte actVal);

#endif
