/*----------------------------------------------------------------
Universidad del Valle de Guatemala
Electrónica Digital 2
Profesores: Pablo Mazariegos y José Morales
Proyecto 3 – Proyecto #3 I2C y NeoPixel
Isabelle Marnat – 21020
----------------------------------------------------------------*/
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Wire.h"

// Definiciones para el MPU6050
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); 

  if (!mpu.begin()) {
    Serial.println("No se pudo encontrar el chip MPU6050");
    while (1) delay(10);
  }
  Serial.println("MPU6050 encontrado!");

  // Configuración de rangos y filtro del MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial2.begin(115200); // Iniciar comunicación Serial con la TIVA C
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Imprimir y enviar datos
  Serial.printf("Acceleration X: %.2f, Y: %.2f, Z: %.2f m/s^2, Rotation X: %.2f, Y: %.2f, Z: %.2f rad/s\n",
                a.acceleration.x, a.acceleration.y, a.acceleration.z,
                g.gyro.x, g.gyro.y, g.gyro.z);
                delay(1000);

  // Formato de envío: "Acceleration,X,Y,Z,Rotacion,X,Y,Z;"
  Serial2.printf("Acceleration,%.2f,%.2f,%.2f,Rotation,%.2f,%.2f,%.2f;",
                a.acceleration.x, a.acceleration.y, a.acceleration.z,
                g.gyro.x, g.gyro.y, g.gyro.z);
                delay(1000);
}
