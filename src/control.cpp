#include <Arduino.h>
#include "DHT.h" //Biblioteca del sensor de humedad
#include <LiquidCrystal.h>

#define DHTTYPE DHT22

const int dhtpin = A1;
const int uvpin = A2;
const int relRocio = 10;
const int relHum = 11;
const int relTem = 13;
const int relLuz = 12;

uint8_t humedad;
uint8_t temperatura;
uint8_t lluvia;
uint8_t luz;
uint8_t ciclos;
uint8_t etapas;
float uv;
float hR;
float tA;

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );

unsigned long tPrevio = 0; //COntador de tiempo general

void encender(int rel);
void apagar(int rel);
void sensorHRT();

//Configuraciones
DHT dht(dhtpin, DHTTYPE);

// Clases
class Control {
  int relPin;

};

void setup() {
  pinMode(relRocio, OUTPUT);
  pinMode(relHum, OUTPUT);
  pinMode(relLuz, OUTPUT);
  pinMode(relTem, OUTPUT);
}

void loop() {
  if(lluvia == 1) {
    encender(relRocio);
  } else {
    apagar(relRocio);
  }
  if(luz == 1) {
    encender(relLuz);
  } else {
    apagar(relLuz);
  }
  if(humedad < hR ) {
    encender(relHum);
  } else {
    apagar(relHum);
  }
  if(temperatura < tA) {
    encender(relTem);
  } else {
    apagar(relTem);
  }
}

void encender(int rel) {
  digitalWrite(rel,HIGH);
}

void apagar(int rel) {
  digitalWrite(rel,LOW);
}

void sensorHRT() {
  unsigned long tLecturas = 2000;
  unsigned long tActual = millis();
  if(tLecturas < tActual - tPrevio) {
    tPrevio = tActual;
  } else {
    hR = dht.readHumidity();
    tA = dht.readTemperature();
    if (isnan(hR) || isnan(tA)) {
      lcd.clear();
      lcd.print("Error en HR/T");
      delay(3000);
      return;
    }
  }
}
