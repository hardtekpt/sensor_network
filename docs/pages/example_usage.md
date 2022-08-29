# Example Usage

As an example, a network was set-up with 2 node devices. The Arduino uno with the SX1276 Dragino shield was used as the hardware platform for the gateway and the TTGO LoRa ESP32 modules were used for the node devices. Regarding sensors and actuators, a motion sensor was attached to each of the nodes and the builtin LED was used as an actuator.

`node_definitions_1.h` file:

    #ifndef NODE_DEFINITIONS_1_H
    #define NODE_DEFINITIONS_1_H

    // Node Encryption key
    const uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
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
    const byte nodeID = 0x01;

    // Node I/O Pinout
    const int vbatPin = 35;

    const int sensPin[] = {16};
    const int actPin[] = {LED_BUILTIN};

    #endif

`node_definitions_2.h` file:

    #ifndef NODE_DEFINITIONS_2_H
    #define NODE_DEFINITIONS_2_H

    // Node Encryption key
    const uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x2f
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
    const byte nodeID = 0x02;

    // Node I/O Pinout
    const int vbatPin = 35;

    const int sensPin[] = {16};
    const int actPin[] = {LED_BUILTIN};

    #endif

`gateway_serial_definitions.h` file:

    #ifndef GATEWAY_SERIAL_DEFINITIONS_H
    #define GATEWAY_SERIAL_DEFINITIONS_H

    // Baud rate for serial communication
    #define BAUD_RATE 9600

    // LoRa Modem Pinout for boards with the Dragino LoRa shield
    #define SS 10
    #define RST 9
    #define DIO0 2  

    // Gateway Settings
    const byte netID = 0xF3;

    #endif

On the `comms_protocol.h` file the keys arrays should be:

    // Encryption keys
    const uint8_t keys[][KEY_SIZE] = {{ //
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x0f
    },{ //
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    },
    { //
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x2f
    }};

After this, the code can be uploaded to both the nodes and the gateway.

`wsn_config.yaml` file:

    wsn_config:
    gateways: 
        - {
            'serial_port': '/dev/ttyUSB0'
        }
    nodes:
        - id: 0x01
          location: 
              - x: 3
                y: 1
          sensors: 
              - id: 0x00
                name: 'motion sensor'
                pinMap: [16]
          actuators: 
              - id: 0x00
                name: 'BUILTINLED'
                pinMap: [5]

        - id: 0x02
          location: 
              - x: 4
                y: 3
          sensors: 
              - id: 0x00
                name: 'motion sensor'
                pinMap: [16]
          actuators: 
              - id: 0x00
                name: 'BUILTINLED'
                pinMap: [5]

The Python application is now configured for this network and can be run.