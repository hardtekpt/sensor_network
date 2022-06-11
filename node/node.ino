/*
 * Node script - send sensor data to gateway and receive commands from gateway
*/

#include "comms_protocol.h"

int buttonState;            
int lastButtonState = LOW; 
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 50;  

void setup() {
  pinMode(gLedPin, OUTPUT);
  pinMode(btnPin, INPUT);
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(csPin, resetPin, irqPin);
  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  Serial.println("LoRa init succeeded.");

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

  prevMil = millis();
  buttonState = digitalRead(btnPin);
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
      sendSensorData(0);
      buttonState = reading;
    }
  }
  lastButtonState = reading;


  if((currentMillis-prevMil) > TIMEOUT_INTERVAL){
    getMsgFromQueueAndSend(currentMillis);
  }
}
