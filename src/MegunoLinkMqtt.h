#pragma once

#include "MegunoLink.h"
#include "MLPMsgBuffer.h"
#include "MegunoLinkBufferWrapper.h"

#include "MqttManager.h"

class MegunoLinkMqtt : public MLPMsgDestination, public MegunoLinkBufferWrapper
{
protected:
  // Destination for data. 
  MqttManager& m_rMqttManager;

public:
  MegunoLinkMqtt(MqttManager& rMqttManager);

  virtual void flush();

};

