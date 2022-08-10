/**
 * @file node_definitions_uno.h
 * @author Francisco Santos (francisco.velez@tecnico.ulisboa.pt)
 * @brief Definitions for node with id 1
 * @version 1.0
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef NODE_DEFINITIONS_1_H
#define NODE_DEFINITIONS_1_H

// Node Encryption key
const uint8_t key[] = { //
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

// LoRa Modem Pinout for boards with the Dragino LoRa shield
#define SS 10
#define RST 9
#define DIO0 2

// Node Settings
const byte nodeID = 0x01;

// Node I/O Pinout

const int sensPin[] = {};
const int actPin[] = {4};

#endif
