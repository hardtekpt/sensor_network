/*
 * Node script - send sensor data to gateway and receive commands from gateway
*/

#include "comms_protocol.h"

/*
 * Function: setup
 * ----------------------------
 *   Runs once at boot. Configure the serial communication. Configure the LoRa radio.
 *   Configure the sensors and actuators input mode
 *
 *   returns: void
 */
void setup() {
  for(int i=0; i<sensN; i++){
    pinMode(sensPin[i], INPUT); 
  }
  for(int i=0; i<actN; i++){
    pinMode(actPin[i], OUTPUT); 
  }
  Serial.begin(BAUD_RATE);

  #if defined(ESP32)
    SPI.begin(SCK, MISO, MOSI, SS);
  #endif
  //LoRa.setTxPower(txPower);
  //LoRa.setSpreadingFactor(spreadingFactor);
  //LoRa.setSignalBandwidth(signalBandwidth);
  //LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed.");
    while (true);
  }

  LoRa.setSyncWord(netID);
  LoRa.enableCrc();
  LoRa_rxMode();

  prevMil = millis();
  prevMilSU = millis();
  
  Serial.println("Node startup complete");
}

/*
 * Function: loop
 * ----------------------------
 *   Main loop function. checks for incoming uplink messages and downlink requests from the server.
 *   calls getMsgFromQueueAndSend on fixed schedules to avoid congestion of the communication channel
 *   and sends an uplink message with the node status periodically.
 *
 *   returns: void
 */
void loop() {
  unsigned long currentMillis = millis();

  // Receive Downlink msg
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    onReceive(packetSize);
  }

  // Send Uplink msg
  if((currentMillis-prevMil) > TIMEOUT_INTERVAL){
    getMsgFromQueueAndSend(currentMillis);
  }

  // Send node status
  if((currentMillis-prevMilSU) > STATUS_UPDATE_INTERVAL){
    byte msgID = random(MAX_MSG_ID);
    sendStatus(msgID);
    prevMilSU = currentMillis;
  }
}
