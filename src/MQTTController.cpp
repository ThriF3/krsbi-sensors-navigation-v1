#include "MQTTController.h"
#include <ArduinoJson.h>

MQTTController *MQTTController::instance = nullptr;

MQTTController::MQTTController(
    const char *ssid,
    const char *password,
    const char *mqttServer,
    uint16_t mqttPort,
    const char *topic,
    Navigation &robot)
    : client(wifiClient),
      robot(robot),
      ssid(ssid),
      password(password),
      mqttServer(mqttServer),
      mqttPort(mqttPort),
      topicName(topic)
{
  instance = this;
}

void MQTTController::begin()
{
  connectWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(MQTTController::mqttCallbackStatic);
  connectMQTT();
}

void MQTTController::loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  if (!client.connected())
  {
    connectMQTT();
  }

  client.loop();

  // Watchdog pengereman otomatis
  if (moveUntil != 0 && millis() > moveUntil)
  {
    robot.stopRobot();
    moveUntil = 0;
  }
}

void MQTTController::connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void MQTTController::connectMQTT()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT... ");

    String clientId = "ESP32-Omni-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");

      // SINKRONISASI FLUTTER: Subscribe ke 2 topik spesifik
      client.subscribe("robot/gerak");
      client.subscribe("robot/aksi");

      Serial.println("Subscribed to: robot/gerak & robot/aksi");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 2 seconds");
      delay(2000);
    }
  }
}

void MQTTController::mqttCallbackStatic(char *topic, byte *payload, unsigned int length)
{
  if (instance != nullptr)
  {
    instance->handleMessage(topic, payload, length);
  }
}

void MQTTController::handleMessage(
    char *topic,
    byte *payload,
    unsigned int length)
{
  String incomingTopic = String(topic);

  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  message.trim();

  // Memori yang cukup untuk JSON Payload
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  // LOGIKA 1: PERGERAKAN (robot/gerak)
  if (incomingTopic == "robot/gerak")
  {
    Serial.println("robot/gerak");
    if (!error)
    {
      float vx = doc["vx"] | 0.0f;
      float vy = doc["vy"] | 0.0f;
      float w = doc["w"] | 0.0f;

      vx = constrain(vx, -1.0f, 1.0f);
      vy = constrain(vy, -1.0f, 1.0f);
      w = constrain(w, -1.0f, 1.0f);

      // Debug output
      Serial.print("vx: ");
      Serial.print(vx);

      Serial.print(" | vy: ");
      Serial.print(vy);

      Serial.print(" | w: ");
      Serial.println(w);

      robot.moveRobot(vx, vy, w);
      moveUntil = millis() + moveDurationMs;
    }
    else
    {
      message.toLowerCase();
      handleCommand(message);
    }
  }
  // LOGIKA 2: AKSI TENDANGAN (robot/aksi)
  else if (incomingTopic == "robot/aksi")
  {
    Serial.println("robot/aksi");
    if (!error)
    {
      String action = doc["action"];

      if (action == "kick")
      {
        Serial.println("AKSI: MENENDANG BOLA!");
        robot.tendang();
      }
    }
  }
}

void MQTTController::handleCommand(String cmd)
{
  if (cmd == "forward")
  {
    robot.moveRobot(0.0, 1.0, 0.0);
  }
  else if (cmd == "backward")
  {
    robot.moveRobot(0.0, -1.0, 0.0);
  }
  else if (cmd == "right")
  {
    robot.moveRobot(1.0, 0.0, 0.0);
  }
  else if (cmd == "left")
  {
    robot.moveRobot(-1.0, 0.0, 0.0);
  }
  else if (cmd == "rotate clockwise" || cmd == "clockwise" || cmd == "cw")
  {
    robot.moveRobot(0.0, 0.0, 0.6);
  }
  else if (cmd == "rotate counter clockwise" || cmd == "counter clockwise" || cmd == "ccw")
  {
    robot.moveRobot(0.0, 0.0, -0.6);
  }
  else if (cmd == "stop")
  {
    robot.stopRobot();
    moveUntil = 0;
    return;
  }
  else
  {
    Serial.println("Unknown command");
    return;
  }
  moveUntil = millis() + moveDurationMs;
}