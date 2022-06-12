/*
 * Gateway Script - communicate with nodes and forward data to ROS
*/
#include "comms_protocol.h"

void setup()
{
  
  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
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
  
  if((currentMillis-prevMil) > RELAY_INTERVAL){
    relayMsgFromQueueToServer(currentMillis);
  }
}
