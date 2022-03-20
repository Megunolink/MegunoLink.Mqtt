/* ***********************************************************************
*  This example demonstrates processing commands received from a Mqtt
*  server on an ESP8266 or ESP32 board using our Arduino MegunoLink.Mqtt 
*  library. It extendes the classic Arduino blink example with commands
*  to control the blink rate. Find the MegunoLink project file 
*  (Command Processing.mlpz) that goes with this example in the same 
*  folder as the Arduino code file. 
* 
*  You'll need to install MegunoLink's Arduino library to use this example:
*       https://www.megunolink.com/documentation/getting-started/arduino-integration/
*  And the following 3rd party libraries:
*     - PangolinMQTT (https://github.com/philbowles/PangolinMQTT)
*     - AsyncTCP (https://github.com/philbowles/AsyncTCP) --- for ESP32 boards
*     - ESPAsyncTCP (https://github.com/philbowles/AsyncTCP) --- for ESP8266 boards
*
*  For more information:
*     Getting started building Arduino interfaces
*       https://www.megunolink.com/documentation/getting-started/build-arduino-interface/
*     Getting started processing serial commands
*       https://www.megunolink.com/documentation/getting-started/processing-serial-commands/
*     Interface panel reference
*       https://www.megunolink.com/documentation/interface-panel/
*
*  The following serial commands are supported:
*       !OnTime n\r\n
*         Sets the amount of time the LED remains on to n [milliseconds]
*
*       !OffTime n\r\n
*         Sets the amount of time the LED remains off to n [milliseconds]
*
*       !ListAll\r\n
*         Lists current blink parameters & sends them to the interface panel.
* 
*  See: https://github.com/Megunolink/MegunoLink.Mqtt
*  *********************************************************************** */

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include "ESPAsyncTCP.h"
#elif defined(ARDUINO_ARCH_ESP32)
//#include "WiFi.h"
//#include "AsyncTCP.h"
#else
#pragma error('Unsupported architecture. Contact us for more information: www.MegunoLink.com')
#endif

#include "MqttCommandHandler.h"
#include "CommandHandler.h"
#include "EspTicker.h"
#include "PangolinMQTT.h"

// -------------------------------------------------
// Setup WiFi and Mqtt credentials (SSID, Mqtt broker, 
// usernames and passwords). 
//#define USE_CONFIG_FILES // Comment out this line to use Option 2 below
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
// Blink variables that control the blink interval 
// and keep track of when we blinked. 
long LastBlink = 0; // Time we last blinked the LED
int OnTime = 10;    // Amount of time the LED remains on [milliseconds]
int OffTime = 100;  // Amount of time the LED remains off [milliseconds]

// -------------------------------------------------
// Communications 

// Looks after connection to the Mqtt broker, 
// and generating topics based on our unique 
// device id. 
MqttManager g_MQTTClient;

// -------------------------------------------------
// Command handlers
// Two command handlers are implemented. The first 
// receives commands from the Mqtt broker on topic 
// MegunoLink/<deviceid>/command; the second 
// receives commands from the serial port. Both
// handlers share the same command library to save
// memory. 
MqttCommandHandler<> MqttCmds(g_MQTTClient);
CommandProcessor<> SerialCmds(MqttCmds);

/// <summary>
/// Called when the 'OnTime' command is received to set the time the LED remains on. 
/// </summary>
/// <param name="Parameters">Contains new on-time (in milliseconds)</param>
void Cmd_SetOnTime(CommandParameter& Parameters)
{
  // Update Arduino variable with parameter value from serial command. 
  OnTime = Parameters.NextParameterAsInteger(OnTime);
}

/// <summary>
/// Called when the 'OffTime' command is received to set the time the LED remains off. 
/// </summary>
/// <param name="Parameters">Contains new off-time (in milliseconds)</param>
void Cmd_SetOffTime(CommandParameter& Parameters)
{
  // Update Arduino variable with parameter value from serial command. 
  OffTime = Parameters.NextParameterAsInteger(OffTime);
}

/// <summary>
/// Called when the 'ListAll' command is received
/// </summary>
/// <param name="Parameters">No parameters</param>
void Cmd_ListAll(CommandParameter& Parameters)
{
  // Write current timing to serial stream
  Parameters.Response.print(F("OnTime [ms]="));
  Parameters.Response.println(OnTime);
  Parameters.Response.print(F("OffTime [ms]="));
  Parameters.Response.println(OffTime);
}

/// <summary>
/// Register commands that we'll support. 
/// </summary>
void SetupCommands()
{
  MqttCmds.AddCommand(F("OnTime"), Cmd_SetOnTime);
  MqttCmds.AddCommand(F("OffTime"), Cmd_SetOffTime);
  MqttCmds.AddCommand(F("ListAll"), Cmd_ListAll);
}

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

  pinMode(LED_BUILTIN, OUTPUT);
}


// -------------------------------------------------
// Main loop
void loop()
{
  SerialCmds.Process();

  // Update the LED
  uint32_t uNow = millis();
  bool bOn = (uNow - LastBlink) < OnTime;
  digitalWrite(LED_BUILTIN, bOn ? HIGH : LOW);
  if ((uNow - LastBlink) > (OnTime + OffTime))
  {
    LastBlink = uNow;
  }
}
