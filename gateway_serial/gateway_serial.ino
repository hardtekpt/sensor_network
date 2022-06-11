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

  Serial.begin(9600);
  Serial.println("Startup complete");
}

void loop()
{

  // implement queue and print formated data

  
  if(newmsg == 1){    
    //Serial.println(bufferG);
    newmsg = 0;
  }
}
