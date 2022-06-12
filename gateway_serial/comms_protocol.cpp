#include "comms_protocol.h"

// Encryption key
uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };

unsigned long prevMil;

cppQueue  msg_q(sizeof(char)*MAX_JSON_PAYLOAD_SIZE, MAX_QUEUE_SIZE, IMPLEMENTATION);
aes256_context ctxt;


void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

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

void onReceive(int packetSize) {
    Serial.print("New msg received: ");
    aes256_init(&ctxt, key);
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    int j = message.length()/ENC_BLOCK_SIZE;
    int h = message.length()/(2*j);
    char buffer1[h+1];
    Payload p;
  
  
    for(int i=0; i<j; i++){
      if (i==0)
        strcpy(buffer1, decryptMsg(message.substring(i*ENC_BLOCK_SIZE, (i+1)*ENC_BLOCK_SIZE)));
      else
        strcat(buffer1, decryptMsg(message.substring(i*ENC_BLOCK_SIZE, (i+1)*ENC_BLOCK_SIZE)));
    }
    Serial.println(buffer1);

    // Check for appID!!!
    if(strstr(buffer1, appID) != NULL){
      sscanf(buffer1, "%*c%*c%*c%*c%*c%3d%3d%3d%d", &p.nodeID, &p.sensorID, &p.msgID, &p.ack);
      sendAck(p.msgID, p.nodeID);
      p.RSSI = LoRa.packetRssi();
      p.SNR = LoRa.packetSnr();
      
      constructJsonAndAddToQueue(p);
    }
    aes256_done(&ctxt); 
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

String splitAndEncrypt(char msg[MAX_PAYLOAD_SIZE]){
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
  
    for(int i = 0; i < BLOCK_SIZE; i++) {
      if(plain[i] < 0x10)
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

void sendAck(int msgID, int nodeID) {
  String enc;
  char payload[MAX_PAYLOAD_SIZE];
  char encP[MAX_ENC_PAYLOAD_SIZE];
  
  sprintf(payload, "%s%.3d%.3d%.3d%.1d", appID, nodeID, 0, msgID, 1);
  Serial.print("Send msg: ");
  Serial.println(payload);

  enc = splitAndEncrypt(payload);
  enc.toCharArray(encP, MAX_ENC_PAYLOAD_SIZE);

  LoRa_sendMessage(encP);
}

// Get a message from the send queue and send it
void relayMsgFromQueueToServer(unsigned long currentMillis){
  if(!msg_q.isEmpty()){
    char msg[MAX_JSON_PAYLOAD_SIZE];
    msg_q.pop(&msg);
    
    Serial.print("relay msg: ");
    Serial.println(msg);
    prevMil = currentMillis;
  }
}

// Construct json string and add to queue
void constructJsonAndAddToQueue(Payload p){
  char msg[MAX_JSON_PAYLOAD_SIZE];

  sprintf(msg, "{\"nodeID\":\"%d\",\"sensorID\":\"%d\"}", p.nodeID, p.sensorID);
  
  msg_q.push(&msg);
}
