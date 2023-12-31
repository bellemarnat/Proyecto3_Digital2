/*----------------------------------------------------------------
Universidad del Valle de Guatemala
Electrónica Digital 2
Profesores: Pablo Mazariegos y José Morales
Proyecto 3 – Proyecto #3 I2C y NeoPixel
Isabelle Marnat – 21020
----------------------------------------------------------------*/
// ---------------- Librerias ----------------
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Wire.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>

// ---------------- Definiciones ----------------
// Definiciones para el MPU6050
#define SDA_PIN 21
#define SCL_PIN 22

//  NeoPixel
#define LED_PIN 15
#define LED_COUNT 16

Adafruit_MPU6050 mpu;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
void colorWipe(uint32_t color, int wait);

void colorWipe(uint32_t color, int wait)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

// ---------------- Void Setup ----------------
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  if (!mpu.begin())
  {
    Serial.println("No se pudo encontrar el chip MPU6050");
    while (1)
      delay(10);
  }
  Serial.println("MPU6050 encontrado!");

  // Configuración de rangos y filtro del MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial2.begin(115200); // Iniciar comunicación Serial con la TIVA C

  strip.begin();
  strip.show();            // Apagar todos los píxeles
  strip.setBrightness(50); // Establecer el brillo
}

// ---------------- Void Loop ----------------
void loop()
{
  if (Serial2.available() > 0)
  {
    char receivedChar = Serial2.read();
    Serial.println(receivedChar);
    if (receivedChar == 'd')
    {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      Serial2.printf("Acceleration,%.2f,%.2f,%.2f,Rotation,%.2f,%.2f,%.2f\n",
                     a.acceleration.x, a.acceleration.y, a.acceleration.z,
                     g.gyro.x, g.gyro.y, g.gyro.z);
      Serial.printf("Acceleration,%.2f,%.2f,%.2f,Rotation,%.2f,%.2f,%.2f\n",
                    a.acceleration.x, a.acceleration.y, a.acceleration.z,
                    g.gyro.x, g.gyro.y, g.gyro.z);
    }
    
    switch (receivedChar) {
    case 'g':
      colorWipe(strip.Color(0, 255, 0), 50); // Verde para éxito
      break;
    case 'r':
      colorWipe(strip.Color(255, 0, 0), 50); // Rojo para error
      break;
    case 'b':
      colorWipe(strip.Color(0, 0, 255), 50); // Azul para standby
      break;
    case 'n':
      colorWipe(strip.Color(0, 0, 0), 0); // Apagado/reinicio
      break;
    }
  }
}
