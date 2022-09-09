/**
 * @file comms_protocol.cpp
 * @author Francisco Santos (francisco.velez@tecnico.ulisboa.pt)
 * @brief Communication Protocol library - set of functions and data structures used to build a network using the LoRa modulation radios
 * @version 1.0
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "comms_protocol.h"

int currMsg = -1;
int count = 0;
unsigned long prevMilR;
unsigned long prevMil;
int msgCount = 0;

cppQueue  relay_q(sizeof(char)*MAX_JSON_PAYLOAD_SIZE, MAX_R_QUEUE_SIZE, IMPLEMENTATION);
cppQueue  msg_q(sizeof(Msg), MAX_QUEUE_SIZE, IMPLEMENTATION);
aes256_context ctxt;


/**
 * @brief Sets the LoRa radio to receive mode
 * 
 * @return void
 */
void LoRa_rxMode() {
  LoRa.disableInvertIQ();
  LoRa.receive();
}

/**
 * @brief Sets the LoRa radio to transmit mode
 * 
 * @return void
 */
void LoRa_txMode() {
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

/**
 * @brief Sets the radio to transmit mode, sends a message string using the LoRa radio
 *        and sets the radio back to receive mode
 * 
 * @param message message to send
 * @param nodeID ID of the destination node
 * @return void
 */
void LoRa_sendMessage(byte *message, byte nodeID) {
  LoRa_txMode();
  LoRa.beginPacket();
  LoRa.write(netID);
  LoRa.write(nodeID);
  LoRa.write(message, MAX_ENC_PAYLOAD_SIZE);
  LoRa.endPacket(false);
  LoRa_rxMode();
}

/**
 * @brief Encrypts a message (character array) using the AES256 algorythm with the corresponding node key
 *        The encryption is made by encrypting blocks of 16 bytes and joining them together
 * 
 * @param msg message array to be decrypted
 * @return byte* a byte array containing the encrypted message
 */
byte *encrypt(char msg[MAX_PAYLOAD_SIZE]) {
  String enc = "";
  const char * p = msg;
  static byte plain [BLOCK_SIZE];
  memset (plain, 0, BLOCK_SIZE);  // ensure trailing zeros
  memcpy (plain, p, mymin (strlen (p), BLOCK_SIZE));
  aes256_encrypt_ecb(&ctxt, plain);
  return plain;
}

/**
 * @brief Decrypts a message string using the AES256 algorythm with the corresponding node key
 * 
 * @param msg message string to be decrypted
 * @return char* an array of characters containing the decrypted message
 */
char  *decryptMsg(char msg[MAX_PAYLOAD_SIZE+1]) {
  static char data[MAX_PAYLOAD_SIZE+1];
  memcpy(data, msg, MAX_PAYLOAD_SIZE+1);
  aes256_decrypt_ecb(&ctxt, (uint8_t *)data);
  return (char *)data;
}

/**
 * @brief returns the minimum value between two integers
 * 
 * @param a first integer to compare
 * @param b second integer to compare
 * @return int the smaller between a and b
 */
int mymin(int a, int b){
  if (a>b)
    return b;
  return a;
}

/**
 * @brief Send an acknowledge message confirming the reception of an uplink transmission
 * 
 * @param msgID ID of the message being acknowledged
 * @param nodeID ID of the destination node
 * @return void
 */
void sendAck(byte msgID, byte nodeID) {
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  byte l = MAX_PAYLOAD_SIZE;
  Msg msg;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msgID, (char)l, 'a', (char)48, (char)48);

  aes256_init(&ctxt, keys[(int)nodeID]);

  byte *plain = encrypt(payload);
  memcpy(msg.msg, plain, MAX_PAYLOAD_SIZE);
  msg.msg[MAX_PAYLOAD_SIZE] = '\0';

  aes256_done(&ctxt);
  
  msg.msgID = msgID;
  msg.flag = 'a';
  msg.nodeID = nodeID;
  msg_q.push(&msg);
}

/**
 * @brief Send a status request message asking for a specific node to respond with a status update
 * 
 * @param nodeID ID of the destination node
 * @return void
 */
void sendStatusRequest(byte nodeID) {
  Msg msg;
  String enc;

  char payload[MAX_PAYLOAD_SIZE];
  msgCount ++;
  if (msgCount == 0)
    msgCount ++;
  msg.msgID = (byte) msgCount;
  msg.flag = 's';
  msg.nodeID = nodeID;
  byte l = (byte) MAX_PAYLOAD_SIZE;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msg.msgID, (char)l, 's', (char)48, (char)48);

  if (nodeID == BROADCAST_ID)
    aes256_init(&ctxt, keys[0]);
  else
    aes256_init(&ctxt, keys[(int)nodeID]);

  byte *plain = encrypt(payload);
  memcpy(msg.msg, plain, MAX_PAYLOAD_SIZE);
  msg.msg[MAX_PAYLOAD_SIZE] = '\0';

  aes256_done(&ctxt);

  if (!msg_q.push(&msg)){
    char msgText[MAX_JSON_PAYLOAD_SIZE];
    sprintf(msgText, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\",\"status\":\"%d\"}", millis(), msg.msgID, 'd', msg.nodeID, 0);
    relay_q.push(&msgText);
  }
}

/**
 * @brief Send a control message to set a value for a node's actuator
 * 
 * @param nodeID ID of the destination node
 * @param actID ID of the actuator to control
 * @param actVal  Value to set the actuator to
 * @return void
 */
void sendActuatorControl(byte nodeID, byte actID, byte actVal) {
  Msg msg;
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  msgCount ++;
  if (msgCount == 0)
    msgCount ++;
  msg.msgID = (byte) msgCount;
  msg.actID = actID;
  msg.actVal = actVal;
  msg.flag = 'c';
  msg.nodeID = nodeID;
  byte l = (byte)MAX_PAYLOAD_SIZE;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msg.msgID, (char)l, 'c', (char)actID, (char)actVal);

  aes256_init(&ctxt, keys[(int)nodeID]);

  byte *plain = encrypt(payload);
  memcpy(msg.msg, plain, MAX_PAYLOAD_SIZE);
  msg.msg[MAX_PAYLOAD_SIZE] = '\0';

  aes256_done(&ctxt);

  // Add msg to msg queue
  msg_q.push(&msg);
}

/**
 * @brief Get a message from the send queue and send it. Implements retransmission 
 *        in case an acknowledge message is not received. Aware of a failed transmission.
 * 
 * @param currentMillis current time in millisenconds since boot
 * @return void
 */
void getMsgFromQueueAndSend(unsigned long currentMillis) {
  if (!msg_q.isEmpty()) {

    // START print msg queue
    /*Msg test;
    for(int i=0; i<msg_q.getCount(); i++){
      msg_q.peekIdx(&test, i);
      Serial.print(test.msgID);
      Serial.print(", ");
    }
    Serial.println(msg_q.getCount());*/
    // END print msg queue

    
    Msg msg;
    msg_q.peek(&msg);

    if (currMsg == msg.msgID)
      count ++;
    else
      count = 0;

    currMsg = msg.msgID;
    if (count < MAX_N_RETRY) {
      Payload p;
      p.msgID = msg.msgID;
      p.flag = 'd';
      p.nodeID = msg.nodeID;
      p.sensorID = 1; // Using sensorID as status
      constructJsonAndAddToQueue(p);
      
      LoRa_sendMessage(msg.msg, msg.nodeID);

      if(msg.flag == 'a')
        msg_q.drop();
      prevMil = currentMillis;
    } else {
      if (msg.flag == 's' || msg.flag == 'c') {
        Payload p;
        p.msgID = msg.msgID;
        p.flag = 'f';
        p.nodeID = msg.nodeID;
        constructJsonAndAddToQueue(p);
      }
      msg_q.drop();
    }
  } else {
    prevMil = currentMillis;
  }
}

/**
 * @brief Get a message from the relay queue and send it to the server via serial communication. 
 * 
 * @param currentMillis current time in millisenconds since boot
 * @return void
 */
 void relayMsgFromQueueToServer(unsigned long currentMillis) {
  if (!relay_q.isEmpty()) {
    
    char msg[MAX_JSON_PAYLOAD_SIZE];
    relay_q.pop(&msg);
    int i;
    for(i=0; i<MAX_JSON_PAYLOAD_SIZE; i++)
      if (msg[i] == '\0')
        break;
    

    Serial.write("rm");
    Serial.write(msg, i);
    Serial.write("\n");
  }
  prevMilR = currentMillis;
}

/**
 * @brief Builds a json string containg the message information and adds the string to the relay queue
 * 
 * @param p payload structure containing the message information along with RSSI, SNR and battery voltage
 * @return void
 */
 void constructJsonAndAddToQueue(Payload p) {
  char msg[MAX_JSON_PAYLOAD_SIZE];

  switch (p.flag) {
    case 'u':
      sprintf(msg, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\",\"sID\":\"%d\",\"sVal\":\"%d\",\"RSSI\":\"%d\",\"SNR\":\"%d.%02d\",\"VBAT\":\"%d.%01d\"}", millis(), p.msgID, p.flag, p.nodeID, (p.sensorID - 1), (p.sensorVal - 1), p.RSSI, (int)p.SNR, (int)(p.SNR * 100) % 100, (int)p.VBAT, (int)(p.VBAT * 10) % 10);
      break;
    case 's':
      sprintf(msg, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\",\"state\":\"%d\",\"RSSI\":\"%d\",\"SNR\":\"%d.%02d\",\"VBAT\":\"%d.%01d\"}", millis(), p.msgID, p.flag, p.nodeID, 1, p.RSSI, (int)p.SNR, (int)(p.SNR * 100) % 100, (int)p.VBAT, (int)(p.VBAT * 10) % 10);
      break;
    case 'a':
      sprintf(msg, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\",\"actID\":\"%d\",\"actVal\":\"%d\",\"RSSI\":\"%d\",\"SNR\":\"%d.%02d\",\"VBAT\":\"%d.%01d\"}", millis(), p.msgID, p.flag, p.nodeID, (p.sensorID - 1), (p.sensorVal - 1), p.RSSI, (int)p.SNR, (int)(p.SNR * 100) % 100, (int)p.VBAT, (int)(p.VBAT * 10) % 10);
      break;
    case 'f':
      sprintf(msg, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\",\"state\":\"%d\",\"RSSI\":\"0\",\"SNR\":\"0\",\"VBAT\":\"0\"}", millis(), p.msgID, 's', p.nodeID, 0);
      break;
    case 'd':
      sprintf(msg, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\",\"status\":\"%d\"}", millis(), p.msgID, p.flag, p.nodeID, p.sensorID);
      break;
  }
  relay_q.push(&msg);
}

/**
 * @brief Relays the downlink messages received from the server to the corresponding node. Formats the message 
 *        into a compact form
 * 
 * @param dlMsg character array containing the downlink message to be relayed
 */
void relayDownlinkMsg(char *dlMsg) {
  char flag = dlMsg[0];
  int nodeID;

  switch (flag) {
    case 's':
      sscanf(dlMsg, "%*c,%d", &nodeID);
      if(nodeID == -1)
        sendStatusRequest((byte)BROADCAST_ID);
      else
        sendStatusRequest((byte)nodeID);
      break;
    case 'c':
      int actID;
      int actVal;
      sscanf(dlMsg, "%*c,%d,%d,%d", &nodeID, &actID, &actVal);
      sendActuatorControl((byte)nodeID, (byte)(actID + 1), (byte)(actVal + 1));
      break;
    case 'p':
      int sf;
      long sb;
      int crd;
      sscanf(dlMsg, "%*c,%d,%ld,%d", &crd, &sb, &sf);
      LoRa.setSignalBandwidth(sb);
      LoRa.setCodingRate4(crd);
      LoRa.setSpreadingFactor(sf);
      break;
  }
}

/**
 * @brief Called every time a new message is received. Filters unwanted messages, decrypts the payload,
 *        gets the relevant fields from the payload and sends back an acknowledge message if necessary.
 *        Finally, calls constructJsonAndAddToQueue to build a json message destined for the server.
 * 
 * @param packetSize size of the incoming message in bytes
 */
void onReceive(int packetSize) {
  byte rNetID = LoRa.read();
  byte rnID = LoRa.read();
  char buffer1[MAX_ENC_PAYLOAD_SIZE];
  String message = "";
  int i=0;
  while (LoRa.available()) {
    buffer1[i] = (char)LoRa.read();
    i++;
  }

  if (rNetID = netID) {
    byte len;
    Payload p;

    aes256_init(&ctxt, keys[(int)rnID]);
    strcpy(buffer1, decryptMsg(buffer1));
    aes256_done(&ctxt);

    buffer1[8] = '\0';

    //Serial.println(buffer1);
    char a,b;
    //Serial.println(buffer1);
    if(sscanf(buffer1, "%c%c%c%c%c%c%c%c", &p.nodeID, &p.msgID, &len, &p.flag, &p.sensorID, &p.sensorVal, &a, &b) == 8){
      p.VBAT = (int)(a-1) + (int)(b-1) * 0.1;
      //Serial.println(p.VBAT);
      Msg msg;
      p.RSSI = LoRa.packetRssi();
      p.SNR = LoRa.packetSnr();
      if (p.flag == 'u') {
        sendAck(p.msgID, p.nodeID);
        //p.sensorVal = buffer1[14];
      }
      if (p.flag == 's') {
        //sendAck(p.msgID, p.nodeID);
        msg_q.peek(&msg);
        if (p.msgID == msg.msgID) {
          msg_q.drop();
        }
      } else if (p.flag == 'a') {
        msg_q.peek(&msg);
        if (p.msgID == msg.msgID) {
          p.sensorID = msg.actID;
          p.sensorVal = msg.actVal;
          msg_q.drop();
        }
      }
      constructJsonAndAddToQueue(p);
    }
  }
}
