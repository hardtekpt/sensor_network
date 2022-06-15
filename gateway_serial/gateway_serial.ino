/*
 * Gateway Script - communicate with nodes and forward data to ROS
*/
#include "comms_protocol.h"

void setup()
{
  
  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed.");
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

  prevMil = millis();

  Serial.begin(9600);
  Serial.println("Startup complete");
}

void loop()
{
  unsigned long currentMillis = millis();

  // Receive downlink msgs from server
  if (Serial.available() > 0) {
    // read the incoming string:
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
