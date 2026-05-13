#include <Arduino.h>
#include "Navigation.h"
#include "MQTTController.h"

// ==========================================
// 1. KONFIGURASI JARINGAN & MQTT
// ==========================================
const char* WIFI_SSID = "Lutfi Alamsyah";
const char* WIFI_PASSWORD = "lutfialamsyah16";

// WAJIB DIGANTI: Gunakan IP IPV4 Laptop HP Victus-mu!
const char* MQTT_SERVER = "10.106.245.210"; 
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "robot/gerak"; // Opsional, karena di MQTTController.cpp sudah kita hardcode subscibe 2 topik

// Inisialisasi Objek
Navigation robot;
MQTTController mqtt(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_PORT, MQTT_TOPIC, robot);

// -------------------- Ultrasonic + MOSFET pins --------------------
const int trigPin   = 22;
const int echoPin   = 23;
const int mosfetPin = 14;

const int buzzerPin = 16;
const int LEDPin = 17;

#define SOUND_SPEED 0.034f

// Fungsi untuk membaca jarak (Bisa digunakan untuk fitur auto-rem jika dekat dinding nantinya)
float readUltrasonicDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);
  if (duration == 0) {
    return -1.0f; 
  }
  return (duration * SOUND_SPEED) / 2.0f;
}

// ==========================================
// 3. SETUP & LOOP UTAMA
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Inisialisasi Omni Soccer Robot ===");

  // Setup Sensor
  // pinMode(trigPin, OUTPUT);
  // pinMode(echoPin, INPUT);

  // WAJIB AKTIF: Agar pin motor dan pin penendang diset sebagai OUTPUT
  robot.begin();

  // Mulai koneksi WiFi dan MQTT
  mqtt.begin();
  
  Serial.println("=== Sistem Siap. Menunggu Perintah dari Flutter... ===");
}

void loop() {
  // Fungsi ini wajib dipanggil terus agar ESP32 bisa menangkap data JSON dari Flutter
  mqtt.loop();

  // Opsional: Cek jarak ultrasonik setiap beberapa saat jika ingin dikembangkan
  // float distance = readUltrasonicDistanceCm();
  // if (distance > 0 && distance < 10.0) {
  //    Serial.println("AWAS NABRAK!");
  // }
}