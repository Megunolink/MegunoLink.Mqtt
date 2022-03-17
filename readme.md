# MegunoLink Mqtt Library Overview
This Arduino library implements support for sending data and commands to MegunoLink visualizers and processing command messages sent using a Mqtt server. It supports both ESP8266 and ESP32 devices.

# Installation
This library requires:
* [PangolinMQTT](https://github.com/philbowles/PangolinMQTT)
* [AsyncTCP](https://github.com/philbowles/AsyncTCP) (for ESP32 boards)
* [ESPAsyncTCP](https://github.com/philbowles/AsyncTCP) (for ESP8266 boards)
* [MegunoLink](https://github.com/Megunolink/MLP) core library.

Download each of the above 3rd party libraries and put them into your Arduino libraries folder (typically `My Documents\Arduino\libraries`). The original AsyncTCP (https://github.com/me-no-dev/AsyncTCP) and ESPAsyncTCP (https://github.com/me-no-dev/ESPAsyncTCP) are a vast improvement over the standard Arduino libraries but include a couple of bugs that Phil Bowles fixed in the repositories referenced above. 

You can use [MegunoLink's Ardunio integration setup](https://www.megunolink.com/documentation/install/arduino-integration-setup/) tool to install the [MegunoLink](https://github.com/Megunolink/MLP) core library.

Finally, install the MegunoLink Mqtt library from this repository by downloading and placing it in your Arduino libraries folder. 

# Using the MegunoLink Mqtt Library
This library supports both sending commands and data to MegunoLink, using [MegunoLinkMqtt](/MegunoLink/MegunoLink.Mqtt/src/MegunoLinkMqtt.h), and receiving commands from MegunoLink, using [MqttCommandLineHandler](/MegunoLink/MegunoLink.Mqtt/src/MqttCommandHandler.h), via a Mqtt broker. In both cases, communication with the broker is managed by [MqttManager](/MegunoLink/MegunoLink.Mqtt/src/MqttManager.h). 

Our examples can retrieve the Mqtt server information from a [`MqttConfig.h`](/MegunoLink/MegunoLink.Mqtt/templates/MqttConfig.h) file in your [configuration library](https://www.megunolink.com/articles/wireless/how-do-i-connect-to-a-wireless-network-with-the-esp32/), or you can add your credentials manually. Just take care not to commit files containing passwords to a public repository. 