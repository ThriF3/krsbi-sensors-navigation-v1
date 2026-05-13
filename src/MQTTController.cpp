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

    String clientId = "ESP32-Navigation-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe(topicName.c_str());
      Serial.print("Subscribed to topic: ");
      Serial.println(topicName);
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

  // Convert payload ke String
  String message;

  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  message.trim();

  Serial.print("MQTT message on ");
  Serial.print(incomingTopic);
  Serial.print(": ");
  Serial.println(message);

  // =========================================
  // HANDLE: robot/gerak
  // =========================================
  if (incomingTopic == "robot/gerak")
  {
    StaticJsonDocument<128> doc;

    DeserializationError error =
        deserializeJson(doc, message);

    if (error)
    {
      Serial.println("JSON gerak invalid");
      return;
    }

    float vx = doc["vx"] | 0.0f;
    float vy = doc["vy"] | 0.0f;
    float w  = doc["w"]  | 0.0f;

    vx = constrain(vx, -1.0f, 1.0f);
    vy = constrain(vy, -1.0f, 1.0f);
    w  = constrain(w, -1.0f, 1.0f);

    robot.moveRobot(vx, vy, w);

    moveUntil = millis() + moveDurationMs;

    return;
  }

  // =========================================
  // HANDLE: robot/aksi
  // =========================================
  else if (incomingTopic == "robot/aksi")
  {
    StaticJsonDocument<128> doc;

    DeserializationError error =
        deserializeJson(doc, message);

    if (error)
    {
      Serial.println("JSON aksi invalid");
      return;
    }

    String action = doc["action"] | "";
    int power = doc["power"] | 100;

    // =========================
    // AKSI KICK
    // =========================
    if (action == "kick")
    {
      Serial.print("Kick with power: ");
      Serial.println(power);

      // TODO:
      // robot.kick(power);
    }

    return;
  }

  // =========================================
  // UNKNOWN TOPIC
  // =========================================
  Serial.println("Unknown topic");
}

void MQTTController::handleCommand(String cmd)
{

  if (cmd == "forward")
  {

    Serial.println(topicName + " = forward");
    robot.moveRobot(0.0, 1.0, 0.0);
  }
  else if (cmd == "backward")
  {

    Serial.println(topicName + " = backward");
    robot.moveRobot(0.0, -1.0, 0.0);
  }
  else if (cmd == "right")
  {

    Serial.println(topicName + " = right");
    robot.moveRobot(1.0, 0.0, 0.0);
  }
  else if (cmd == "left")
  {

    Serial.println(topicName + " = left");
    robot.moveRobot(-1.0, 0.0, 0.0);
  }
  else if (
      cmd == "rotate clockwise" ||
      cmd == "clockwise" ||
      cmd == "cw")
  {

    Serial.println(topicName + " = rotate clockwise");
    robot.moveRobot(0.0, 0.0, 0.6);
  }
  else if (
      cmd == "rotate counter clockwise" ||
      cmd == "counter clockwise" ||
      cmd == "ccw")
  {

    Serial.println(topicName + " = rotate counter clockwise");
    robot.moveRobot(0.0, 0.0, -0.6);
  }
  else if (cmd == "stop")
  {

    Serial.println(topicName + " = stop");

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