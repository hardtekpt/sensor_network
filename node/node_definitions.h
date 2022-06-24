/*
 * Header file for the node program. Contains hardware pinouts, 
 * LoRa modem configs and node settings
*/
#ifndef NODE_DEFINITIONS_H
#define NODE_DEFINITIONS_H

// LoRa Modem Pinout
const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

// Node Settings
const byte netID = 0xF3;
const byte nodeID = 0x01;

// Node I/O Pinout
//const int gLedPin = 4;
const int btnPin = 3;
const int actPin[] = {4, 5};
const int actN = sizeof(actPin)/sizeof(int);


#endif
