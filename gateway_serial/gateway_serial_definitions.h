/*
 * Header file for the gateway program. Contains hardware pinouts,
 * LoRa modem configs and gateway settings
 */
 
#ifndef GATEWAY_SERIAL_DEFINITIONS_H
#define GATEWAY_SERIAL_DEFINITIONS_H

// Baud rate for serial communication
#define BAUD_RATE 9600

// SPI pinout for TTGO boards
//#define SCK 5
//#define MISO 19
//#define MOSI 27

// LoRa Modem Pinout for boards with the Dragino LoRa shield
#define SS 10
#define RST 9
#define DIO0 2  

// Gateway Settings
const int gatewayID = 0xFF;
const byte netID = 0xF3;

#endif
