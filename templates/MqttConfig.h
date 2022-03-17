/* ***********************************************************************
*  Configuration file for connecting to a Mqtt server. See
*  https://www.megunolink.com/articles/wireless/how-do-i-connect-to-a-wireless-network-with-the-esp32/
*  for more information on creating your own configuration library to 
*  simplify credential storage. 
*  *********************************************************************** */

#pragma once

const IPAddress MqttServer(192, 168, 15, 10);
const uint16_t MqttPort = 1883;
const char* MqttUser = "Username"; // or nullptr if none required
const char* MqttPass = "Password"; // or nullptr if none required
