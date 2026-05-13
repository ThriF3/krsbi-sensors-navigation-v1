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
    
    // SINKRONISASI FLUTTER: Set ke 200ms agar robot sigap mengerem
    // karena aplikasi Flutter mengirim data (throttling) setiap 50ms.
    const unsigned long moveDurationMs = 200;

    void connectWiFi();
    void connectMQTT();
    void handleMessage(char* topic, byte* payload, unsigned int length);
    void handleCommand(String cmd);

    static void mqttCallbackStatic(char* topic, byte* payload, unsigned int length);
};

#endif