/*
 * Header file for the node program. Contains hardware pinouts, 
 * LoRa modem configs and node settings
*/

#ifndef NODE_DEFINITIONS_4_H
#define NODE_DEFINITIONS_4_H

// Node Encryption key
const uint8_t key[] = { //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x4f
};

// SPI pinout for TTGO boards
#define SCK 5
#define MISO 19
#define MOSI 27

// LoRa Modem Pinout for TTGO boards
#define SS 18
#define RST 14
#define DIO0 26

// Node Settings
const byte nodeID = 0x04;

// Node I/O Pinout
const int vbatPin = 35;

const int sensPin[] = {16};
const int actPin[] = {LED_BUILTIN};

#endif
