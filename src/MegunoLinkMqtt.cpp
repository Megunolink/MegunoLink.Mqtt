#include "MegunoLinkMqtt.h"

MegunoLinkMqtt::MegunoLinkMqtt(MqttManager &rMqttManager)
  : m_rMqttManager(rMqttManager)
  , MegunoLinkBufferWrapper(this)
{
}

void MegunoLinkMqtt::flush()
{
  if (m_rMqttManager.IsMqttConnected())
  {
    m_rMqttManager.publish(m_rMqttManager.BuildStreamTopic().c_str(), c_str(), length());
  }
  begin();
}

