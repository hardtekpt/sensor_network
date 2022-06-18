/*
 * Header file for the gateway program. Contains hardware pinouts,
 * LoRa modem configs and gateway settings
*/
#ifndef GATEWAY_SERIAL_DEFINITIONS_H
#define GATEWAY_SERIAL_DEFINITIONS_H

// Lora Modem Pinout
const int csPin = 10;         
const int resetPin = 9;   
const int irqPin = 2;   

// Gateway Settings
const int gatewayID = 0xFF;
const byte netID = 0xF3;


#endif
