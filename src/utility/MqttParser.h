#pragma once
#include "utility/CommandDispatcherBase.h"
#include "MqttManager.h"
#include <string>

namespace MLP
{
  class MqttParser
  {
  protected:
    /// <summary>
    /// The commands to dispatch messages to
    /// </summary>
    CommandDispatcherBase& m_rCommandHandler;

    /// <summary>
    /// The object responsible for sending and receiving Mqtt packets. 
    /// </summary>
    MqttManager& m_rMQTTClient;

  protected:
    MqttParser(MLP::CommandDispatcherBase& rCommandHandler, MqttManager& rMqttClient);



  private:
    void SetupEvents();

    void SubscribeToCommands();

    void OnConnected(bool bSessionPresent);
    void OnDispatchMessage(const char* topic, const uint8_t* payload, size_t len, uint8_t uQOS, bool bRetain, bool bDup);
    void OnDeviceIdChanged(bool bMqttConnected, char const* pchOldDeviceId, char const* pNewDeviceId);
  };
}