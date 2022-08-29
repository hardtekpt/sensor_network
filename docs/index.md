# Sensor Network

This repository holds the Sensor Network code stack for a LoRa based wireless sensor network. It contains the packages needed for deploying and monitoring a network with a gateway and node devices. 

![GitHub last commit (branch)](https://img.shields.io/github/last-commit/hardtekpt/sensor_network/master)
![GitHub contributors](https://img.shields.io/github/contributors/hardtekpt/sensor_network)
[![GitHub issues](https://img.shields.io/github/issues/hardtekpt/sensor_network)](https://github.com/hardtekpt/sensor_network/issues)
[![GitHub forks](https://img.shields.io/github/forks/hardtekpt/sensor_network)](https://github.com/hardtekpt/sensor_network/network)
[![GitHub stars](https://img.shields.io/github/stars/hardtekpt/sensor_network)](https://github.com/hardtekpt/sensor_network/stargazers)
[![License](https://img.shields.io/github/license/hardtekpt/sensor_network?color=blue)](https://github.com/hardtekpt/sensor_network/blob/main/LICENSE)

# Introduction

The Sensor Network repository consists on a set of software that allows for a wireless sensor network to be deployed using the LoRa modulation technology. To this purpose, the repository includes Arduino packages for the node devices and gateway device.

Additionally a python application with a GUI to monitor and manage the network is included. This application is also used to connect the network to the ROS middleware so as to provide integration with vehicles like UAVs and AGVs.

The figure below depicts the network architecture used.

![WSN Architecture Diagram](assets/wsl_net_arch.png "WSN Architecture Diagram")