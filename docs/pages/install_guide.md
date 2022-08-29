# Install Guide

## Requirements

In order to set-up such a wireless sensor network, the following software is required:

- Arduino IDE;
- Python3.

Additionally, the following Arduino libraries are required:

- [LoRa.h](https://github.com/sandeepmistry/arduino-LoRa);
- [cppQueue.h](https://github.com/SMFSW/Queue);
- [aes256.h](https://github.com/ilvn/aes256).

## Clone the repository

After ensuring all the aforementioned requirements are met this repository can be cloned and the contents can be opened in an editor like VScode:

    git clone https://github.com/hardtekpt/sensor_network
    cd sensor_network
    code .

## Configuration

The packages now need to be configured according to the hardware available. The network has been tested with both the Arduino Uno platform using the SX1276 Dragino Shield and with the TTGO LoRa ESP32 modules. To this extent, the pin map for the LoRa radio must be configured for all devices:

- On the Gateway edit the file `gateway_serial_definitions.h`;
- On every Node use the `node_definitions` folder and create a file for each node. This file is referenced by `node_definitions.h`.

Additionally, every node needs a unique 32 byte encryption key. This key must also be added to the `gateway_serial_definitions.h` file. Finally, every node needs a unique hexadecimal ID. The gateway has the encryption keys of all the nodes in an array indexed by the node's ID.

Regarding the Network Manager, the `wsn_config.yaml` file must be edited to include:

- The serial port where gateway is attached;
- Information regarding each node:
    - ID;
    - Geographic location;
    - Sensors;
    - Actuators;

It should be noted that example configurations for each file are included in the repository.

The last thing to do is to attach any sensors and actuators to the nodes and upload the code. For this, see [Example Usage](https://hardtekpt.github.io/sensor_network_docs/pages/example_usage/).