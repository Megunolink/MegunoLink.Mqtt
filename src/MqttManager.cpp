#include "MqttManager.h"
#include <iostream>
#include <sstream>

using namespace std;

char const* StatusTopic = "status";
char const* Status_Online = "online";
char const* Status_Offline = "offline";

MqttManager::MqttManager(char const* pchRootTopic /*= nullptr*/)
{
  if (pchRootTopic != nullptr)
  {
    m_strRootTopic = pchRootTopic;
  }
  else
  {
    m_strRootTopic = "MegunoLink";
  }
  m_bMqttConnected = false;
  m_bNetworkConnected = false; 

  onConnect([this](bool bSessionPresent) { OnMqttConnect(bSessionPresent); });
  onDisconnect([this](int8_t nReason) { OnMqttDisconnect(nReason); });
  onMessage([this](const char* topic, const uint8_t* payload, size_t len, uint8_t uQOS, bool bRetain, bool bDup)
  {
    OnMqttDispatchMessage(topic, payload, len, uQOS, bRetain, bDup);
  });
}

void MqttManager::OnNetworkConnected()
{
  m_bNetworkConnected = true;
  StartMqttConnection();
}

void MqttManager::OnNetworkConnectionLost()
{
  m_bNetworkConnected = false;
  m_tmrMqttReconnect.detach();
}

void MqttManager::UpdateDeviceId(const char* pchDeviceId)
{
  PublishStatus(Status_Offline);

  string strOldDeviceId(m_strDeviceId);
  m_strDeviceId = pchDeviceId;

  std::for_each(
    m_DeviceIdChangedHandlers.begin(),
    m_DeviceIdChangedHandlers.end(),
    [this, strOldDeviceId](MQTT_hDeviceIdChanged& handler) { handler(m_bMqttConnected, strOldDeviceId.c_str(), m_strDeviceId.c_str()); });

  PublishStatus(Status_Online);
}

void MqttManager::PrintDeviceId()
{
  Serial.print(F("Device id: "));
  Serial.println(GetDeviceId().c_str());
}

string MqttManager::BuildStreamTopic() const
{
  return BuildTopic("stream");
}

std::string MqttManager::BuildTopic(const char* pchLeaf) const
{
  return BuildTopic(GetDeviceId().c_str(), pchLeaf);
}

std::string MqttManager::BuildTopic(char const* pchDeviceId, char const* pchLeaf) const
{
  string strTopic(m_strRootTopic);
  strTopic.reserve(30);
  strTopic += '/';
  strTopic += pchDeviceId;
  strTopic += '/';
  strTopic += pchLeaf;
  return strTopic;
}

void MqttManager::SubscribeToConnect(PANGO_cbConnect handler)
{
  m_ConnectHandlers.push_back(handler);
}

void MqttManager::SubscribeToDisconnect(PANGO_cbDisconnect handler)
{
  m_DisconnectHandlers.push_back(handler);
}

void MqttManager::SubscribeToMessage(PANGO_cbMessage handler)
{
  m_MessageHandlers.push_back(handler);
}

void MqttManager::SubscribeToDeviceIdChanged(MQTT_hDeviceIdChanged handler)
{
  m_DeviceIdChangedHandlers.push_back(handler);
}

string MqttManager::GetDeviceId() const
{
  if (!m_strDeviceId.empty())
  {
    return m_strDeviceId;
  }


  return m_strDeviceId;
}

void MqttManager::StartMqttConnection()
{
  // If we don't have an id yet, generate one. 
  if (m_strDeviceId.empty())
  {
#if defined(ARDUINO_ARCH_ESP8266)
    stringstream StringWriter;
    uint32_t uId = ESP.getChipId();
    StringWriter << std::hex << uId;
    m_strDeviceId = StringWriter.str();
#elif defined(ARDUINO_ARCH_ESP32)
    stringstream StringWriter(m_strDeviceId);
    uint32_t uId = ESP.getEfuseMac();
    StringWriter << std::hex << uId;
    m_strDeviceId = StringWriter.str();
#else
    m_strDeviceId = "default";
#endif
  }
  setWill(BuildTopic(StatusTopic).c_str(), 1, true, "offline");
  connect();
}

void MqttManager::OnMqttConnect(bool bSessionPresent)
{
  m_bMqttConnected = true;

  Serial.println(F("Mqtt connected"));
  Serial.flush();

  PublishStatus(Status_Online);

  std::for_each(
    m_ConnectHandlers.begin(),
    m_ConnectHandlers.end(),
    [bSessionPresent](PANGO_cbConnect& handler) { handler(bSessionPresent); });
}

void MqttManager::OnMqttDisconnect(int8_t nReason)
{
  m_bMqttConnected = false;
  std::for_each(
    m_DisconnectHandlers.begin(),
    m_DisconnectHandlers.end(),
    [nReason](PANGO_cbDisconnect& handler) { handler(nReason); });

  if (m_bNetworkConnected)
  {
    m_tmrMqttReconnect.once(MqttReconnectDelay, [this]() { StartMqttConnection(); });
  }
}

void MqttManager::OnMqttDispatchMessage(const char* topic, const uint8_t* payload, size_t len, uint8_t uQOS, bool bRetain, bool bDup)
{
  std::for_each(
    m_MessageHandlers.begin(),
    m_MessageHandlers.end(),
    [topic, payload, len, uQOS, bRetain, bDup](PANGO_cbMessage& handler) { handler(topic, payload, len, uQOS, bRetain, bDup); });
}

void MqttManager::PublishStatus(char const* pchStatus)
{
  publish(BuildTopic(StatusTopic).c_str(), pchStatus, strlen(pchStatus), 1, true);
}
