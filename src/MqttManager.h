#pragma once
#include "PangolinMQTT/src/PangolinMQTT.h"
#include "EspTicker.h"

#include <vector>
#include <string>

using MQTT_hDeviceIdChanged = std::function<void(bool bIsMqttConnected, char const* pchOldDeviceId, char const* pchNewDeviceId)>;


class MqttManager : private PangolinMQTT
{
private:
  std::vector<PANGO_cbConnect> m_ConnectHandlers;
  std::vector<PANGO_cbDisconnect> m_DisconnectHandlers;
  std::vector<PANGO_cbMessage> m_MessageHandlers;
  std::vector<MQTT_hDeviceIdChanged> m_DeviceIdChangedHandlers;

  const int MqttReconnectDelay = 5;

  EspTicker m_tmrMqttReconnect;

  /// <summary>
  /// Root topic. Defaults to "MegunoLink" if empty.
  /// </summary>
  std::string m_strRootTopic;

  /// <summary>
  /// Id of this device, for forming the Mqtt topics. 
  /// </summary>
  std::string m_strDeviceId;

  /// <summary>
  /// True when the network transport we use to talk to the Mqtt
  /// server is connected. 
  /// </summary>
  bool m_bNetworkConnected;

  /// <summary>
  /// When it is okay to send to the MQTT server. 
  /// </summary>
  bool m_bMqttConnected;

public:
  MqttManager(char const* pchRootTopic = nullptr);

  using PangolinMQTT::setCleanSession;
  using PangolinMQTT::setClientId;
  using PangolinMQTT::setCredentials;
  using PangolinMQTT::setKeepAlive;
  using PangolinMQTT::setServer;
  using PangolinMQTT::setWill;
  using PangolinMQTT::subscribe;
  using PangolinMQTT::unsubscribe;
  using PangolinMQTT::publish;
  using PangolinMQTT::xPublish;

  void OnNetworkConnected();
  void OnNetworkConnectionLost();

  bool IsMqttConnected() const { return m_bMqttConnected; }

  void UpdateDeviceId(char const* pchDeviceId);
  void PrintDeviceId();

  std::string BuildStreamTopic() const;
  std::string BuildTopic(char const* pchLeaf) const;
  std::string BuildTopic(char const* pchDeviceId, char const* pchLeaf) const;

  void SubscribeToConnect(PANGO_cbConnect handler);
  void SubscribeToDisconnect(PANGO_cbDisconnect handler);
  void SubscribeToMessage(PANGO_cbMessage handler);
  void SubscribeToDeviceIdChanged(MQTT_hDeviceIdChanged handler);

private:
  std::string GetDeviceId() const;

  void StartMqttConnection();

  void OnMqttConnect(bool bSessionPresent);
  void OnMqttDisconnect(int8_t nReason);
  void OnMqttDispatchMessage(const char* topic, const uint8_t* payload, size_t len, uint8_t uQOS, bool bRetain, bool bDup);

  void PublishStatus(char const* pchStatus);
};