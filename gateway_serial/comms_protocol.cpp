#include "comms_protocol.h"

// Encryption key
uint8_t key[] = { //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

int currMsg = -1;
int count = 0;
unsigned long prevMilR;
unsigned long prevMil;

cppQueue  relay_q(sizeof(char)*MAX_JSON_PAYLOAD_SIZE, MAX_R_QUEUE_SIZE, IMPLEMENTATION);
cppQueue  msg_q(sizeof(Msg), MAX_QUEUE_SIZE, IMPLEMENTATION);
aes256_context ctxt;


void LoRa_rxMode() {
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode() {
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write(netID);
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

char  *decryptMsg(String msg) {
  static uint8_t data[17];
  char aux[3];
  int i = 0;

  for (size_t count = 0; count < 32; count += 2) {

    msg.substring(count, count + 2).toCharArray(aux, 3);
    sscanf(aux, "%x", &data[i]);
    i++;
  }

  aes256_decrypt_ecb(&ctxt, data);
  //Serial.print((char *)data);
  return (char *)data;
}

void onReceive(int packetSize) {
  byte rNetID = LoRa.read();

  String message = "";
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  if (rNetID = netID) {
    //Serial.println("Msg received");

    int j = message.length() / ENC_BLOCK_SIZE;
    int h = message.length() / (2 * j);
    char buffer1[h + 1];
    byte len;
    Payload p;

    aes256_init(&ctxt, key);
    for (int i = 0; i < j; i++) {
      if (i == 0)
        strcpy(buffer1, decryptMsg(message.substring(i * ENC_BLOCK_SIZE, (i + 1)*ENC_BLOCK_SIZE)));
      else
        strcat(buffer1, decryptMsg(message.substring(i * ENC_BLOCK_SIZE, (i + 1)*ENC_BLOCK_SIZE)));
    }
    aes256_done(&ctxt);

    sscanf(buffer1, "%c%c%c%c%c%c", &p.nodeID, &p.msgID, &len, &p.flag, &p.sensorID, &p.sensorVal);

    Msg msg;
    p.RSSI = LoRa.packetRssi();
    p.SNR = LoRa.packetSnr();
    if (p.flag == 'u') {
      sendAck(p.msgID, p.nodeID);
      //p.sensorVal = buffer1[14];
    }
    if (p.flag == 's') {
      sendAck(p.msgID, p.nodeID);
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

void onTxDone() {
  //Serial.println("TxDone");
  LoRa_rxMode();
}

String splitAndEncrypt(char msg[MAX_PAYLOAD_SIZE]) {
  // Split the msg into 16 byte chunks,
  // Encrypt them
  // Join them back together
  String enc = "";
  aes256_init(&ctxt, key);
  const char * p = msg;

  while (strlen (p) > 0) {
    byte plain [BLOCK_SIZE];
    memset (plain, 0, BLOCK_SIZE);  // ensure trailing zeros
    // copy block into plain array
    memcpy (plain, p, min (strlen (p), BLOCK_SIZE));
    // encrypt it
    //encrypt_it (plain, key);
    aes256_encrypt_ecb(&ctxt, plain);

    String hexstring = "";

    for (int i = 0; i < BLOCK_SIZE; i++) {
      if (plain[i] < 0x10)
        hexstring += '0';
      hexstring += String(plain[i], HEX);
    }
    //Serial.println(hexstring);
    enc += hexstring;

    // advance past this block
    p += min (strlen (p), BLOCK_SIZE);
  }
  aes256_done(&ctxt);
  return enc;
}

void sendAck(byte msgID, byte nodeID) {
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  char encP[MAX_ENC_PAYLOAD_SIZE];
  byte l = MAX_PAYLOAD_SIZE;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msgID, (char)l, 'a', (char)48, (char)48);

  enc = splitAndEncrypt(payload);
  enc.toCharArray(encP, MAX_ENC_PAYLOAD_SIZE);

  LoRa_sendMessage(encP);
}

// Get a message from the send queue and send it
void relayMsgFromQueueToServer(unsigned long currentMillis) {
  if (!relay_q.isEmpty()) {
    char msg[MAX_JSON_PAYLOAD_SIZE];
    relay_q.pop(&msg);

    Serial.print("rm");
    Serial.println(msg);
  }
  prevMilR = currentMillis;
}

// Construct json string and add to queue
void constructJsonAndAddToQueue(Payload p) {
  char msg[MAX_JSON_PAYLOAD_SIZE];

  switch (p.flag) {
    case 'u':
      sprintf(msg, "{\"f\":\"%c\",\"nID\":\"%d\",\"sID\":\"%d\",\"sVal\":\"%d\",\"RSSI\":\"%d\",\"SNR\":\"%d.%02d\"}", p.flag, p.nodeID, (p.sensorID - 1), (p.sensorVal - 1), p.RSSI, (int)p.SNR, (int)(p.SNR * 100) % 100);
      break;
    case 's':
      sprintf(msg, "{\"f\":\"%c\",\"nID\":\"%d\",\"state\":\"%d\",\"RSSI\":\"%d\",\"SNR\":\"%d.%02d\"}", p.flag, p.nodeID, 1, p.RSSI, (int)p.SNR, (int)(p.SNR * 100) % 100);
      break;
    case 'a':
      sprintf(msg, "{\"f\":\"%c\",\"nID\":\"%d\",\"actID\":\"%d\",\"actVal\":\"%d\",\"RSSI\":\"%d\",\"SNR\":\"%d.%02d\"}", p.flag, p.nodeID, (p.sensorID - 1), (p.sensorVal - 1), p.RSSI, (int)p.SNR, (int)(p.SNR * 100) % 100);
      break;
    case 'f':
      sprintf(msg, "{\"f\":\"%c\",\"nID\":\"%d\",\"state\":\"%d\"}", 's', p.nodeID, 0);
      break;
  }
  relay_q.push(&msg);
}

void relayDownlinkMsg(char *dlMsg) {
  char flag = dlMsg[0];
  int nodeID;

  switch (flag) {
    case 's':
      sscanf(dlMsg, "%*c,%d", &nodeID);
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

void sendStatusRequest(byte nodeID) {
  Msg msg;
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  msg.msgID = (byte) random(MAX_MSG_ID);
  msg.flag = 's';
  msg.nodeID = nodeID;
  byte l = (byte) MAX_PAYLOAD_SIZE;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msg.msgID, (char)l, 's', (char)48, (char)48);

  enc = splitAndEncrypt(payload);
  enc.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);

  // Add msg to msg queue
  msg_q.push(&msg);
}

// Get a message from the send queue and send it
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
      LoRa_sendMessage(msg.msg);
      prevMil = currentMillis;
    } else {
      Serial.print("Failed to send msg with id: ");
      Serial.println(msg.msgID);
      // Check flag and relay a msg to the server!!!
      if (msg.flag == 's' || msg.flag == 'c') {
        Payload p;
        p.flag = 'f';
        p.nodeID = msg.nodeID;
        constructJsonAndAddToQueue(p);
      }
      msg_q.drop();
    }
  }
}

void sendActuatorControl(byte nodeID, byte actID, byte actVal) {
  Msg msg;
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  msg.msgID = (byte) random(MAX_MSG_ID);
  msg.actID = actID;
  msg.actVal = actVal;
  msg.flag = 'c';
  byte l = (byte)MAX_PAYLOAD_SIZE;

  sprintf(payload, "%c%c%c%c%c%c", (char)nodeID, (char)msg.msgID, (char)l, 'c', (char)actID, (char)actVal);

  enc = splitAndEncrypt(payload);
  enc.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);

  // Add msg to msg queue
  msg_q.push(&msg);
}
