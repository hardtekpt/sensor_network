/*
 * Node script - send sensor data to gateway and receive commands from gateway
*/

#include "comms_protocol.h"

int buttonState;            
int lastButtonState = LOW; 
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 50;  

void setup() {
  //pinMode(gLedPin, OUTPUT);
  pinMode(btnPin, INPUT);
  for(int i=0; i<actN; i++){
    pinMode(actPin[i], OUTPUT); 
  }
  Serial.begin(9600);
  LoRa.setPins(csPin, resetPin, irqPin);
  //LoRa.setTxPower(txPower);
  //LoRa.setSpreadingFactor(spreadingFactor);
  //LoRa.setSignalBandwidth(signalBandwidth);
  //LoRa.setCodingRate4(codingRateDenominator);


  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed.");
    while (true);
  }

  //LoRa.setSyncWord(0xF3);
  LoRa.enableCrc();


  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

  prevMil = millis();
  buttonState = digitalRead(btnPin);
  
  Serial.println("Node startup complete");
}

void loop() {
  unsigned long currentMillis = millis();

  
  // btn sensor
  int reading = digitalRead(btnPin);
  if (reading != lastButtonState)
    lastDebounceTime = currentMillis;
  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState){
      // Send sensor data
      sendSensorData((byte) 0x00, (byte) buttonState);
      buttonState = reading;
    }
  }
  lastButtonState = reading;


  if((currentMillis-prevMil) > TIMEOUT_INTERVAL){
    getMsgFromQueueAndSend(currentMillis);
  }
}
