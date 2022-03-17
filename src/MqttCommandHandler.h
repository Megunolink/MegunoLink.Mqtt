#pragma once
#include <Arduino.h>
#include <Stream.h>

#include "utility/CommandDispatcherBase.h"
#include "utility/MqttParser.h"

template <int MAX_COMMANDS = 10, int MAX_VARIABLES = 10> class MqttCommandHandler 
  : public MLP::CommandDispatcherBase, public MLP::MqttParser
{
  // Array of commands we can match & dispatch. 
  MLP::CommandCallback m_Commands[MAX_COMMANDS];

  // Array of variables we can match & set/print
  MLP::VariableMap m_Variables[MAX_VARIABLES];

public:

  MqttCommandHandler(MqttManager& rMqttClient)
    : CommandDispatcherBase(m_Commands, MAX_COMMANDS, m_Variables, MAX_VARIABLES)
    , MqttParser(*(static_cast<MLP::CommandDispatcherBase*>(this)), rMqttClient)
  {
  }
};
