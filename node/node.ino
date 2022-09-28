
/**
 * @file node.ino
 * @author Francisco Santos (francisco.velez@tecnico.ulisboa.pt)
 * @brief Node script - send sensor data to gateway and receive commands from gateway
 * @version 1.0
 * @date 2022-08-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "comms_protocol.h"
int motionState = 0;
long t1;
long t2;

/**
 * @brief Arduino setup function
 * 
 * Runs once at boot. Configure the serial communication. Configure the LoRa radio.
 * Configure the sensors and actuators input mode
 * 
 * @return void
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
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed.");
    while (true);
  }

  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setSignalBandwidth(signalBandwidth);

  LoRa.setSyncWord(netID);
  LoRa.enableCrc();
  LoRa_rxMode();

  prevMil = millis();
  prevMilSU = millis();
  
  Serial.println("Node startup complete");
}


/**
 * @brief Arduino loop function
 * 
 * Main loop function. checks for incoming uplink messages and downlink requests from the server.
 * calls getMsgFromQueueAndSend on fixed schedules to avoid congestion of the communication channel
 * and sends an uplink message with the node status periodically.
 * 
 * @return void
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
    //sendStatus(msgID);
    prevMilSU = currentMillis;
  }

  // Send sensor data
  if(millis() > 30000){
    int val = digitalRead(sensPin[0]);
    if((val == 1) && (motionState == 0)){
      Serial.println("motion detected");
      sendSensorData(0, 1);
      t1 = millis();
    }
    motionState = val;
  }

  
}
