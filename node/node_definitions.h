/*
 * Header file for the node program. Contains hardware pinouts, 
 * LoRa modem configs and node settings
*/

#ifndef NODE_DEFINITIONS_H
#define NODE_DEFINITIONS_H

// Baud rate for serial communication
#define BAUD_RATE 115200

// Node Settings
const byte netID = 0xF3;

#include "node_definitions/node_definitions_1.h"

const int sensN = sizeof(sensPin)/sizeof(int);
const int actN = sizeof(actPin)/sizeof(int);

#endif
