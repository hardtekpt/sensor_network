/*
 * Communication Protocol library - set of functions and data structures used to build a network using the LoRa modulation radios
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


/*
 * Function: LoRa_rxMode
 * ----------------------------
 *   Sets the LoRa radio to receive mode
 *
 *   returns: void
 */
void LoRa_rxMode() {
  LoRa.disableInvertIQ();
  LoRa.receive();
}

/*
 * Function: LoRa_txMode
 * ----------------------------
 *   Sets the LoRa radio to transmit mode
 *
 *   returns: void
 */
void LoRa_txMode() {
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

/*
 * Function: LoRa_sendMessage
 * ----------------------------
 *   Sets the radio to transmit mode, sends a message string using the LoRa radio
 *   and sets the radio back to receive mode
 *   
 *   message: message to send
 *   nodeID: ID of the destination node
 *
 *   returns: void
 */
void LoRa_sendMessage(byte *message, byte nodeID) {
  LoRa_txMode();
  LoRa.beginPacket();
  LoRa.write(netID);
  LoRa.write(nodeID);
  //Msg msg;
  //message.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);
  //LoRa.print(message);
  //for(int i=0; i<17; i++){
  //  Serial.println(message[i], HEX);
  //}
  LoRa.write(message, MAX_ENC_PAYLOAD_SIZE);
  LoRa.endPacket(false);
  
  //Serial.println(message.length());
  LoRa_rxMode();
}

/*
 * Function: splitAndEncrypt
 * ----------------------------
 *   Encrypts a message (character array) using the AES256 algorythm with the corresponding node key
 *   The encryption is made by encrypting blocks of 16 bytes and joining them together
 *   
 *   msg: message array to be decrypted
 *
 *   returns: a string containing the encrypted message
 */
String splitAndEncrypt(char msg[MAX_PAYLOAD_SIZE]) {
  String enc = "";
  const char * p = msg;
  while (strlen (p) > 0) {
    byte plain [BLOCK_SIZE];
    memset (plain, 0, BLOCK_SIZE);  // ensure trailing zeros
    memcpy (plain, p, min (strlen (p), BLOCK_SIZE));
    aes256_encrypt_ecb(&ctxt, plain);
    enc += String((char *)plain);
    // advance past this block
    p += min (strlen (p), BLOCK_SIZE);
  }  
  Serial.println(enc.length());
  //Serial.print("inside splitAndEncrypt: ");
  //Serial.println(enc);
  return enc;
}

byte *splitAndEncrypt2(char msg[MAX_PAYLOAD_SIZE]) {
  String enc = "";
  const char * p = msg;
  static byte plain [BLOCK_SIZE];
  while (strlen (p) > 0) {
    
    memset (plain, 0, BLOCK_SIZE);  // ensure trailing zeros
    memcpy (plain, p, min (strlen (p), BLOCK_SIZE));
    aes256_encrypt_ecb(&ctxt, plain);
    //for(int i=0; i<17; i++){
    //  Serial.println(plain[i], HEX);
    //}
    return plain;
    enc += String((char *)plain);
    // advance past this block
    p += min (strlen (p), BLOCK_SIZE);
  }  
  //Serial.println(enc.length());
  //Serial.print("inside splitAndEncrypt: ");
  //Serial.println(enc);
  //return plain;
}

/*
 * Function: decryptMsg
 * ----------------------------
 *   Decrypts a message string using the AES256 algorythm with the corresponding node key
 *   
 *   msg: message string to be decrypted
 *
 *   returns: an array of characters containing the decrypted message
 */
char  *decryptMsg(String msg) {
  static char m[MAX_PAYLOAD_SIZE+1];
  msg.toCharArray(m, MAX_PAYLOAD_SIZE+1);
  aes256_decrypt_ecb(&ctxt, (uint8_t *)m);
  return (char *)m;
}

char  *decryptMsg2(char msg[MAX_PAYLOAD_SIZE+1]) {
  static char data[MAX_PAYLOAD_SIZE+1];
  memcpy(data, msg, MAX_PAYLOAD_SIZE+1);
  aes256_decrypt_ecb(&ctxt, (uint8_t *)data);
  return (char *)data;
}

/*
 * Function: sendAck
 * ----------------------------
 *   Send an acknowledge message confirming the reception of an uplink transmission
 *   
 *   msgID: ID of the message being acknowledged
 *   nodeID: ID of the destination node
 *
 *   returns: void
 */
void sendAck(byte msgID, byte nodeID) {
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  byte l = MAX_PAYLOAD_SIZE;
  Msg msg;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msgID, (char)l, 'a', (char)48, (char)48);

  aes256_init(&ctxt, keys[(int)nodeID]);

  byte *plain = splitAndEncrypt2(payload);
  memcpy(msg.msg, plain, MAX_PAYLOAD_SIZE);
  msg.msg[MAX_PAYLOAD_SIZE] = '\0';
  
  //enc = splitAndEncrypt(payload);
  //enc.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);
  aes256_done(&ctxt);

  //Serial.print("add ack to queue: ");
  //Serial.println(payload);
  //Serial.print("enc msg: ");
  //Serial.println(msg.msg);
  
  msg.msgID = msgID;
  msg.flag = 'a';
  msg.nodeID = nodeID;
  msg_q.push(&msg);
}

/*
 * Function: sendStatusRequest
 * ----------------------------
 *   Send a downlink message asking for a status update from the node
 *   
 *   nodeID: ID of the destination node
 *
 *   returns: void
 */
void sendStatusRequest(byte nodeID) {
  Msg msg;
  String enc;

  //while (enc.length() != MAX_PAYLOAD_SIZE){
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

    byte *plain = splitAndEncrypt2(payload);
    memcpy(msg.msg, plain, MAX_PAYLOAD_SIZE);

    //msg.msg = splitAndEncrypt2(payload);
    msg.msg[MAX_PAYLOAD_SIZE] = '\0';

    
    //for(int i=0; i<17; i++){
    //  Serial.println(plain[i], HEX);
    //}

    //if (enc[7] == '\0')
      //Serial.println("teste");
    //Serial.print("enc msg: ");
    //Serial.println(enc);
    aes256_done(&ctxt);
    
  
    //Serial.print("add statusRequest to queue: ");
    //Serial.println(payload);
    
    
  
    //char buffer1[MAX_ENC_PAYLOAD_SIZE];
    //aes256_init(&ctxt, keys[(int)nodeID]);
    //strcpy(buffer1, decryptMsg(String(msg.msg)));
    //aes256_done(&ctxt);
    //buffer1[6] = '\0';
    //Serial.print("decoded: ");
    //Serial.println(buffer1);
  
    
  //}
  //enc.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);
  msg_q.push(&msg);
}

/*
 * Function: sendActuatorControl
 * ----------------------------
 *   Send a downlink message to control a node's actuator
 *   
 *   nodeID: ID of the destination node
 *   actID: ID of the actuator to control
 *   actVal: Value to assign to the relevant actuator
 *
 *   returns: void
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
  byte *plain = splitAndEncrypt2(payload);
  memcpy(msg.msg, plain, MAX_PAYLOAD_SIZE);
  msg.msg[MAX_PAYLOAD_SIZE] = '\0';
  //enc = splitAndEncrypt(payload);
  aes256_done(&ctxt);
  //enc.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);

  // Add msg to msg queue
  msg_q.push(&msg);
}

/*
 * Function: getMsgFromQueueAndSend
 * ----------------------------
 *   Get a message from the send queue and send it. Implements retransmission 
 *   in case an acknowledge message is not received. Aware of a failed transmission.
 *   
 *   currentMillis: current time in millisenconds since boot
 *
 *   returns: void
 */
void getMsgFromQueueAndSend(unsigned long currentMillis) {
  if (!msg_q.isEmpty()) {
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
  }
}

/*
 * Function: relayMsgFromQueueToServer
 * ----------------------------
 *   Get a message from the relay queue and send it to the server via serial communication. 
 *   
 *   currentMillis: current time in millisenconds since boot
 *
 *   returns: void
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

/*
 * Function: constructJsonAndAddToQueue
 * ----------------------------
 *   Builds a json string containg the message information and adds the string to the relay queue
 *   
 *   p: payload structure containing the message information along with RSSI, SNR and battery voltage
 *
 *   returns: void
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
      sprintf(msg, "{\"t\":\"%lu\",\"msgID\":\"%d\",\"f\":\"%c\",\"nID\":\"%d\"}\0", millis(), p.msgID, p.flag, p.nodeID);
      break;
  }
  relay_q.push(&msg);
}

/*
 * Function: relayDownlinkMsg
 * ----------------------------
 *   Relays the downlink messages received from the server to the corresponding node. Formats the message 
 *   into a compact form
 *   
 *   dlMsg: character array containing the downlink message to be relayed
 *
 *   returns: void
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
  }
}

/*
 * Function: onReceive
 * ----------------------------
 *   Called every time a new message is received. Filters unwanted messages, decrypts the payload,
 *   gets the relevant fields from the payload and sends back an acknowledge message if necessary.
 *   Finally, calls constructJsonAndAddToQueue to build a json message destined for the server.
 *   
 *   packetSize: size of the incoming message in bytes
 *
 *   returns: void
 */
void onReceive(int packetSize) {
  byte rNetID = LoRa.read();
  byte rnID = LoRa.read();
  char buffer1[MAX_ENC_PAYLOAD_SIZE];
  String message = "";
  int i=0;
  while (LoRa.available()) {
    //message += (char)LoRa.read();
    buffer1[i] = (char)LoRa.read();
    i++;
  }

  if (rNetID = netID) {
    //Serial.println("Msg received");
    //Serial.println(message.length());

    //int j = message.length() / ENC_BLOCK_SIZE;
    //int h = message.length() / (1 * j);
    //char buffer1[h + 1];
    byte len;
    Payload p;

    aes256_init(&ctxt, keys[(int)rnID]);
    //for (int i = 0; i < j; i++) {
    //  if (i == 0)
    //    strcpy(buffer1, decryptMsg(message.substring(i * ENC_BLOCK_SIZE, (i + 1)*ENC_BLOCK_SIZE)));
    //  else
    //    strcat(buffer1, decryptMsg(message.substring(i * ENC_BLOCK_SIZE, (i + 1)*ENC_BLOCK_SIZE)));
   // }
    strcpy(buffer1, decryptMsg2(buffer1));
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
