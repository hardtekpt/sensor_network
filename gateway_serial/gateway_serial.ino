/*
 * Gateway Script - communicate with nodes and forward data to ROS
 */

#include "comms_protocol.h"

/*
 * Function: setup
 * ----------------------------
 *   Runs once at boot. Configure the serial communication. Configure the LoRa radio.
 *
 *   returns: void
 */
void setup() {
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
    while (true);                       // if failed, do nothing
  }

  LoRa.setSyncWord(netID);
  LoRa.enableCrc();
  LoRa_rxMode();

  prevMil = millis();

  Serial.println("Startup complete");
}

/*
 * Function: loop
 * ----------------------------
 *   Main loop function. checks for incoming uplink messages and downlink requests from the server.
 *   calls relayMsgFromQueueToServer and getMsgFromQueueAndSend on fixed schedules to avoid 
 *   congestion of the communication channel
 *
 *   returns: void
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
  
  if((currentMillis-prevMilR) > RELAY_INTERVAL){
    relayMsgFromQueueToServer(currentMillis);
  }

  if((currentMillis-prevMil) > TIMEOUT_INTERVAL){
    getMsgFromQueueAndSend(currentMillis);
  }
}
