
/* ***********************************************************************
*  This example demonstrates sending visualizer data to a Mqtt broker 
*  where it can be picked up for plotting by MegunoLink's time-plot 
*  visualizer. The same approach can be used to send data and commands
*  to any MegunoLink visualizer. Find the MegunoLink project 
*  (Plotting Mqtt data.mlpz), that goes with this example, in the same folder
*  as this Arduino code file.
*
*  You'll need to install MegunoLink's Arduino library to use this example:
*       https://www.megunolink.com/documentation/getting-started/arduino-integration/
*  And the following 3rd party libraries:
*     - PangolinMQTT (https://github.com/philbowles/PangolinMQTT)
*     - AsyncTCP (https://github.com/philbowles/AsyncTCP) --- for ESP32 boards
*     - ESPAsyncTCP (https://github.com/philbowles/AsyncTCP) --- for ESP8266 boards
*
*  For more information:
*     MegunoLink Mqtt library: https://github.com/Megunolink/MegunoLink.Mqtt
*     Getting started with plotting:
*       https://www.megunolink.com/documentation/getting-started/plotting-data/
*     Time plot visualizer reference:
*       https://www.megunolink.com/documentation/plotting/time-plot/
*     Installing the MegunoLink Arduino library:
*       https://www.megunolink.com/documentation/getting-started/arduino-integration/
*     Sending data to multiple plots:
*       https://www.megunolink.com/documentation/plotting/sending-data-to-multiple-plots/
* 
*  *********************************************************************** */

#if defined(ARDUINO_ARCH_ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#else
#pragma error('Unsupported architecture. Contact us for more information: www.MegunoLink.com')
#endif

#include "MegunoLinkMqtt.h"
#include "ArduinoTimer.h"
#include "EspTicker.h"

// -------------------------------------------------
// Setup WiFi and Mqtt credentials (SSID, Mqtt server, 
// usernames and passwords). 
#define USE_CONFIG_FILES // Comment out this line to use Option 2 below
#if defined(USE_CONFIG_FILES)

// Include SSID and password from a library file. See:
// https://www.megunolink.com/articles/wireless/how-do-i-connect-to-a-wireless-network-with-the-esp32/
#include "WiFiConfig.h"
#include "MqttConfig.h"

#else

// Option 2
const char* SSID = "Your SSID";
const char* WiFiPassword = "Your Password";

const IPAddress MqttServer(192, 168, 15, 10);
const uint16_t MqttPort = 1883;
const char* MqttUser = "MqttServerUserName";
const char* MqttPass = "MqttServerPassword";

#endif

// -------------------------------------------------
// Communications 

// Looks after connection to the Mqtt Server, 
// and generating topics based on our unique 
// device id. 
MqttManager g_MQTTClient;

// Implements sending data and commands to MegunoLink
// visualizers via a Mqtt broker. 
MegunoLinkMqtt g_MegunoLinkMqtt(g_MQTTClient);

// -------------------------------------------------
// WiFi connection. 

// Time between attempting to reconnect lost connections
const int WiFiReconnectDelay = 5; // seconds
EspTicker g_tmrWifiReconnect;

void SetupWiFi()
{
  WiFi.mode(WIFI_STA);

#if defined(ARDUINO_ARCH_ESP8266)

  // ESP8266 version
  static WiFiEventHandler hStationGotIP = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP&)
  {
    WiFi_GotIP();
  });

  static WiFiEventHandler hStationDisconnected = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& e)
  {
    WiFi_Disconnected((uint8_t)e.reason);
  });

#elif defined(ARDUINO_ARCH_ESP32)

  // ESP32 version
  WiFi.onEvent([](WiFiEvent_t e, WiFiEventInfo_t info)
  {
    WiFi_GotIP();

  }, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);

  WiFi.onEvent([](WiFiEvent_t e, WiFiEventInfo_t info)
  {
    WiFi_Disconnected(info.disconnected.reason);

  }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
#endif
}

void ConnectToWifi()
{
  WiFi.begin(SSID, WiFiPassword);
}

void WiFi_GotIP()
{
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());
  Serial.flush();

  // Let the Mqtt manager know that that a 
  // network connection is now available
  // so it can connect to the Mqtt broker. 
  g_MQTTClient.OnNetworkConnected();
}

void WiFi_Disconnected(uint8_t uReason)
{
  Serial.println("WiFi lost connection: ");
  Serial.println(uReason); Serial.flush();

  g_tmrWifiReconnect.once(WiFiReconnectDelay, ConnectToWifi);

  // Let the Mqtt manager know we've lost our 
  // network connection so it doesn't try 
  // reconnecting ot the Mqtt broker. 
  g_MQTTClient.OnNetworkConnectionLost();
}

// -------------------------------------------------
// Setup

void SetupMqtt()
{
  g_MQTTClient.setServer(MqttServer, MqttPort);
  if (MqttUser != nullptr)
  {
    g_MQTTClient.setCredentials(MqttUser, MqttPass);
  }

  g_MQTTClient.setCleanSession(true);
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Mqtt Blink 2.0"));
  Serial.print(F("Built: ")); Serial.println(F(__TIMESTAMP__));
  g_MQTTClient.PrintDeviceId();

  SetupWiFi();
  SetupMqtt();
  SetupCommands();

  ConnectToWifi();
}


// -------------------------------------------------
// Main loop
void loop()
{
  SerialCmds.Process();

  static ArduinoTimer tmrPlot;
  if (g_MQTTClient.IsMqttConnected() && tmrPlot.TimePassed_Milliseconds(500))
  {
    float frequency = 0.01; //Hz
    float seconds = (float)millis() / 1000;
    double dY = sin(2 * 3.141 * frequency * seconds);

    // Send data to the plot.
    g_MegunoLinkMqtt.GetTimePlot().SendFloatData(F("Sine"), dY, 4);

    // Messages work too. 
    g_MegunoLinkMqtt.println(F("Hello world!"));
  }
}
