/*
 * Header file for the node program. Contains hardware pinouts, 
 * LoRa modem configs and node settings
*/

#ifndef NODE_DEFINITIONS_H
#define NODE_DEFINITIONS_H

// Baud rate for serial communication
#define BAUD_RATE 115200

#include "node_definitions/node_definitions_3.h"

const int sensN = sizeof(sensPin)/sizeof(int);
const int actN = sizeof(actPin)/sizeof(int);

#endif
