/**
 * @file gateway_serial.ino
 * @author Francisco Santos (francisco.velez@tecnico.ulisboa.pt)
 * @brief Gateway script - send sensor data to gateway and receive commands from gateway
 * @version 1.0
 * @date 2022-08-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "comms_protocol.h"

/**
 * Function: setup
 * ----------------------------
 * @brief Arduino setup function
 * 
 * Runs once at boot. Configure the serial communication. Configure the LoRa radio.
 * Configure the sensors and actuators input mode
 * 
 * @return void
 */
void setup() {
  Serial.begin(BAUD_RATE);

  #if defined(ESP32)
    SPI.begin(SCK, MISO, MOSI, SS);
  #endif

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(frequency)) {
    Serial.write("LoRa init failed.\n");
    while (true);                       // if failed, do nothing
  }
  
  LoRa.setTxPower(txPower);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setSpreadingFactor(spreadingFactor);
  
  LoRa.setSyncWord(netID);
  LoRa.enableCrc();
  LoRa_rxMode();

  prevMil = millis();

  Serial.write("Startup complete\n");
}

/**
 * Function: loop
 * ----------------------------
 * @brief Arduino loop function
 * 
 * Main loop function. checks for incoming uplink messages and downlink requests from the server.
 * calls getMsgFromQueueAndSend on fixed schedules to avoid congestion of the communication channel
 * and sends an uplink message with the node status periodically.
 * 
 * @return void
 */
void loop()
{
  unsigned long currentMillis = millis();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    onReceive(packetSize);
  }

  // Receive downlink msgs from server
  if (Serial.available() > 0) {
    String dlMsg = Serial.readString();
    char msg[dlMsg.length()];
    dlMsg.toCharArray(msg, dlMsg.length());

    relayDownlinkMsg(msg);
  }
  
  //if((currentMillis-prevMilR) > RELAY_INTERVAL){
  relayMsgFromQueueToServer(currentMillis);
  //}

  if((currentMillis-prevMil) > TIMEOUT_INTERVAL){
    //sendStatusRequest(1);
    getMsgFromQueueAndSend(currentMillis);
  }
}
