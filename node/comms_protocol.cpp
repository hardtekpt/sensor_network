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
unsigned long prevMil;

cppQueue  msg_q(sizeof(Msg), MAX_QUEUE_SIZE, IMPLEMENTATION);
aes256_context ctxt;

// Set the LoRa radio modem to receive mode
void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

// Set the LoRa radio to transmit mode
void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

// Transmit a string message using the LoRa radio modem
void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

// Always go back to receive mode after transmitting a message
void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

// Decrypt an encrypted message string using the AES256 protocol and the defined key
char  *decryptMsg(String msg){
  static uint8_t data[17];
  char aux[3];
  int i=0;

  for (size_t count = 0; count < 32; count+=2) {
    
    msg.substring(count, count+2).toCharArray(aux,3);
    sscanf(aux, "%x", &data[i]);
    i++;
  }

  aes256_decrypt_ecb(&ctxt, data);
  //Serial.print((char *)data);
  return (char *)data;
}

// Encrypt the payload by splitting it into blocks, encrypting them and joining them back together
String splitAndEncrypt(char msg[MAX_PAYLOAD_SIZE]){
  String enc = "";
  aes256_init(&ctxt, key);
  const char * p = msg;

  while (strlen (p) > 0) {
    byte plain [BLOCK_SIZE];
    memset (plain, 0, BLOCK_SIZE);  // ensure trailing zeros
    memcpy (plain, p, min (strlen (p), BLOCK_SIZE));

    aes256_encrypt_ecb(&ctxt, plain);

    String hexstring = "";  
    for(int i = 0; i < BLOCK_SIZE; i++) {
      if(plain[i] < 0x10)
        hexstring += '0';
      hexstring += String(plain[i], HEX);
    }
    //Serial.println(hexstring);
    enc += hexstring;
    p += min (strlen (p), BLOCK_SIZE);
  }
  aes256_done(&ctxt);
  return enc;
}

// Add a message to the send queue when a sensor is triggered
void sendSensorData(int sensorID){
  Msg msg;
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  msg.msgID = (int) random(1000);
  
  sprintf(payload, "%s%.3d%.3d%.3d%.1d", appID, nodeID, sensorID, msg.msgID, 0);
  Serial.println(payload);

  enc = splitAndEncrypt(payload);
  enc.toCharArray(msg.msg, MAX_ENC_PAYLOAD_SIZE);
  
  // Add msg to msg queue
  msg_q.push(&msg);
}

// Get a message from the send queue and send it
void getMsgFromQueueAndSend(unsigned long currentMillis){
  if(!msg_q.isEmpty()){
    Msg msg;
    msg_q.peek(&msg);

    if(currMsg == msg.msgID)
      count ++;
    else
      count = 0;

    currMsg = msg.msgID;
    if(count < MAX_N_RETRY){
      Serial.print("send msg: ");
      Serial.println(msg.msg);
      LoRa_sendMessage(msg.msg);
      prevMil = currentMillis;
    } else {
      Serial.print("Failed to send msg with id: ");
      Serial.println(msg.msgID);
      msg_q.drop();
    }
  }
}

// Handle received messages by the node
void onReceive(int packetSize) {
  Serial.println("New msg received");
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  int j = message.length()/ENC_BLOCK_SIZE;
  int h = message.length()/(2*j);
  char buffer1[h+1];
  Payload p;

  aes256_init(&ctxt, key);
  for(int i=0; i<j; i++){
    if (i==0)
      strcpy(buffer1, decryptMsg(message.substring(i*32, (i+1)*32)));
    else
      strcat(buffer1, decryptMsg(message.substring(i*32, (i+1)*32)));
  }
  aes256_done(&ctxt); 

  // Check for appID!!!
  if(strstr(buffer1, appID) != NULL){
    sscanf(buffer1, "%*c%*c%*c%*c%*c%3d%3d%3d%d", &p.nodeID, &p.sensorID, &p.msgID, &p.ack);
    Msg msg;
    msg_q.peek(&msg);
    if(p.nodeID == nodeID){
      if(p.msgID == msg.msgID){
        if(p.ack){
          Serial.print("Message with ID: ");
          Serial.print(p.msgID);
          Serial.println(" delivered!");
          msg_q.drop();
          digitalWrite(gLedPin, 1);
          delay(400);
          digitalWrite(gLedPin, 0);
        }
      }
    }
  }
}
