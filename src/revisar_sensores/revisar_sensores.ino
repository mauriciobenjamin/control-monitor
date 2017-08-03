/*------------------------------------------------
Pequeño programa para revisar el funcionamiento de
los sensores sin tener que programar ciclos de
funcionamiento ni nada más

Autor: mauriciobenjamin@gmail.com
Github: https://github.com/mauriciobenjamin/control-monitor
https://learn.adafruit.com/adafruit-feather-m0-adalogger/setup
------------------------------------------------*/
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "DHT.h"
#include "Adafruit_TSL2591.h"

#define DHTTYPE DHT22
LiquidCrystal lcd(5, 6, 9, 10 ,11, 12);
const int dhtpin = 13;
DHT dht(dhtpin, DHTTYPE);
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

uint16_t luxes;
float hR;
float tA;
unsigned long tiempoAnt = millis();

void sensorHRT();
uint16_t sensorLuz();

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(dhtpin, INPUT);
  lcd.clear(); lcd.home();
}

void loop() {
  sensorLuz();
  delay(3000);
  sensorHRT();
  delay(3000);
}

uint16_t sensorLuz() {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  luxes = tsl.calculateLux(full, ir);
  Serial.print("Luminancia: "); Serial.print(luxes); Serial.println(" Lux\t");
  Serial.print("IR: "); Serial.println(ir);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Lum: "); lcd.print(luxes); lcd.print(" Lux");
  lcd.setCursor(0,1);
  lcd.print("IR: "); lcd.print(ir);
  return luxes, ir;
}

void sensorHRT() {
  unsigned long tLecturas = 2000;
  unsigned long tActual = millis();
  if(tActual > tiempoAnt + tLecturas) {
    hR = dht.readHumidity();
    tA = dht.readTemperature();
    lcd.clear(); lcd.home(); lcd.print("Humedad: "); lcd.print(hR); lcd.print("%");
    lcd.setCursor(0,1); lcd.print("Temp: "); lcd.print(tA); lcd.print(" C");
    Serial.print("Humedad: "); Serial.print(hR); Serial.println("%");
    Serial.print("Temperatura: "); Serial.print(tA); Serial.println(" °C");
    tiempoAnt = tActual;
    if (isnan(hR) || isnan(tA)) {
      lcd.clear();
      lcd.print("Error en HR/T");
      Serial.println("Error en HR/T");
      delay(3000);
      return;
    }
  }
  return hR, tA;
}
