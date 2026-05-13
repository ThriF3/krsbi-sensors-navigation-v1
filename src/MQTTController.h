#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Navigation.h"

class MQTTController {
  public:
    MQTTController(
      const char* ssid,
      const char* password,
      const char* mqttServer,
      uint16_t mqttPort,
      const char* topic,
      Navigation& robot
    );

    void begin();
    void loop();

  private:
    static MQTTController* instance;

    WiFiClient wifiClient;
    PubSubClient client;
    Navigation& robot;

    const char* ssid;
    const char* password;
    const char* mqttServer;
    uint16_t mqttPort;
    String topicName;

    unsigned long moveUntil = 0;
    const unsigned long moveDurationMs = 1500;

    void connectWiFi();
    void connectMQTT();
    void handleMessage(char* topic, byte* payload, unsigned int length);
    void handleCommand(String cmd);

    static void mqttCallbackStatic(char* topic, byte* payload, unsigned int length);
};

#endif