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
const char appID[] = "90077";
const int nodeID = 1;

// Node I/O Pinout
const int gLedPin = 4;
const int btnPin = 3;

#endif
