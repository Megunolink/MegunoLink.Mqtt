#include "MqttParser.h"
#include <string>
#include <sstream>
#include "FixedStringBuffer.h"

using namespace MLP;
using namespace std;

char const* CommandTopic = "command";
char const* ResponseTopic = "response";

MqttParser::MqttParser(CommandDispatcherBase& rCommandHandler, MqttManager& rMqttClient)
  : m_rCommandHandler(rCommandHandler)
  , m_rMQTTClient(rMqttClient)
{
  SetupEvents();
}

void MqttParser::SetupEvents()
{
  m_rMQTTClient.SubscribeToMessage(
    [this](const char* topic, const uint8_t* payload, size_t len, uint8_t uQOS, bool bRetain, bool bDup)
  {
    OnDispatchMessage(topic, payload, len, uQOS, bRetain, bDup);
  });

  m_rMQTTClient.SubscribeToConnect(
    [this](bool bDirty)
  {
    OnConnected(bDirty);
  });
}

void MqttParser::SubscribeToCommands()
{
  m_rMQTTClient.subscribe(m_rMQTTClient.BuildTopic(CommandTopic).c_str(), 2);
}

void MqttParser::OnConnected(bool bSessionPresent)
{
  SubscribeToCommands();
}

void MqttParser::OnDispatchMessage(const char* topic, const uint8_t* payload, size_t len, uint8_t uQOS, bool bRetain, bool bDup)
{
  //Serial.printf("T=%u Message %s qos%d dup=%d retain=%d len=%d\n", millis(), topic, uQOS, bDup, bRetain, len);
  //PANGO::dumphex(payload, len, 16);
  string strCommandTopic = m_rMQTTClient.BuildTopic(CommandTopic);
  if (strncmp(strCommandTopic.c_str(), topic, strCommandTopic.length()) == 0)
  {
    char const* pchBuffer = (char const*)payload;
    int nCommandSize = len;

    // Strip off carriage return/line feed characters (if present)
    if (nCommandSize > 0 && (pchBuffer[nCommandSize - 1] == '\r' || pchBuffer[nCommandSize - 1] == '\n'))
    {
      --nCommandSize;
    }
    if (nCommandSize > 0 && (pchBuffer[nCommandSize - 1] == '\r' || pchBuffer[nCommandSize - 1] == '\n'))
    {
      --nCommandSize;
    }

    if (len > 0 && *pchBuffer == '!')
    {
      ++pchBuffer;
      --nCommandSize;

      char achBuffer[nCommandSize + 1];
      memcpy(achBuffer, pchBuffer, nCommandSize);
      achBuffer[nCommandSize] = 0;

      FixedStringBuffer<256> strResponse;
      m_rCommandHandler.DispatchCommand(achBuffer, strResponse);

      if (strResponse.length() > 0)
      {
        m_rMQTTClient.publish(m_rMQTTClient.BuildTopic(ResponseTopic).c_str(), (uint8_t*)strResponse.c_str(), strResponse.length());
      }
    }
  }
}

void MqttParser::OnDeviceIdChanged(bool bMqttConnected, char const* pchOldDeviceId, char const* pNewDeviceId)
{
  if (bMqttConnected)
  {
    m_rMQTTClient.unsubscribe(m_rMQTTClient.BuildTopic(pchOldDeviceId, CommandTopic).c_str());
    SubscribeToCommands();
  }
}

