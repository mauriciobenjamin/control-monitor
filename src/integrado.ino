/* Registrador de datos para el Feather M0 Adalogger
 *  usa el reloj de tiempo real interno, cuatro sensores, controla una pantalla LCD
 *  y cuatro relevadores para controlar hasta cuatro dispositivos
 *
 *  Autor: Mauricio Jimenez
 *  fecha: 11/07/16
 *  Versión: 0.01
 *
 *  Basado en codigo abierto de AdaFruit
 */

 /*
  * ADC voltajes para los 5 botones de la entrada analoga A0
  * Derecha: 0.02V
  * Arriba: 0.48V
  * Abajo: 1.06V
  * Izquierda: 1.58V
  * Seleccionar: 2.23V
  */

#include <LiquidCrystal.h> //Biblioteca de la pantalla
#include <Wire.h> // Biblioteca para I2C
#include <SPI.h>
#include <SD.h> //Bibliotecas para la tarjeta SD
#include <RTCZero.h> //Biblioteca del reloj de tiempo real
#include <Adafruit_Sensor.h> //Biblioteca de normalización de sensores
#include "Adafruit_TSL2591.h" // Biblioteca del sensor de luz
#include "DHT.h" // Biblioteca del sensor de humedad y temperatura

/** Configuración del sensor de HR y T **/
#define DHTPIN A1 //Pin del sensor de humedad
#define DHTTYPE DHT22 // Tipo de sensor de humedad
#define cardSelect 4 //Pin de la tarjeta SD

/**Configuración de la LCD**/
LiquidCrystal lcd(11, 12, 5, 6, 9, 10); //Inicia el LCD con los pines configurados
DHT dht(DHTPIN, DHTTYPE); //Inicializa el sensor de HR y T

/** Configuración del sensor de luminancia **/
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); //Inicializa el sensor de iluminancia y le asigna un numero

/** Configuración del reloj de tiempo real **/
RTCZero rtc; // Inicia el reloj de tiempo real

/* Establece la hora del reloj */
const byte horas = 10;
const byte minutos = 16;
const byte segundos = 30;

/* Establece el día del reloj  */
const byte dia = 11;
const byte mes = 07;
const byte ano = 16;

/* Configuracion del sensor de UV */
int uvSen = A2;
float uv;
float h;

/**************************************************************************/
/*
    Configuración de la ganancia y el tiempo de integración del sensor de luz
    */
/**************************************************************************/
void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */
  Serial.println("------------------------------------");
  Serial.print  ("Ganancia:         ");
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println("1x (Baja)");
      break;
    case TSL2591_GAIN_MED:
      Serial.println("25x (Media)");
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println("428x (Alta)");
      break;
    case TSL2591_GAIN_MAX:
      Serial.println("9876x (Maxima)");
      break;
  }
  Serial.print  ("Tiempo:       ");
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(" ms");
  Serial.println("------------------------------------");
  Serial.println("");
}

void tiempo() {
  Serial.print(rtc.getDay()); Serial.print("/");
  Serial.print(rtc.getMonth()); Serial.print("/");
  Serial.print(rtc.getYear()); Serial.print(";");
  Serial.print(rtc.getHours()); Serial.print(":");
  Serial.println(rtc.getMinutes());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(rtc.getDay()); lcd.print("/");
  lcd.print(rtc.getMonth()); lcd.print("/");
  lcd.print(rtc.getYear()); lcd.print(";");
  lcd.setCursor(0,1);
  lcd.print(rtc.getHours()); lcd.print(":");
  lcd.print(rtc.getMinutes());
  delay(1500);
}

void sensorLuz(void) {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print("Luminancia: "); Serial.print(tsl.calculateLux(full, ir)); Serial.println(" Lux\t");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Lum: "); lcd.print(tsl.calculateLux(full, ir)); lcd.print("Lux");
  delay(1500);
}

void sensorHRT() {
  // El sensor necesita al menos 2 seg para hacer lecturas
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Error al leer el sensor de H y T");
    return;
  }

  Serial.print("Humedad: "); Serial.print(h); Serial.println("%\t");
  Serial.print("Temperatura: "); Serial.print(t); Serial.println(" °C\t");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humedad: "); lcd.print(h); lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("Temp: "); lcd.print(t); lcd.print(" C");
  delay(3000);
  }


float brutoaW(int bruto) {
    float watt = bruto * 3.3 / 1024;
    return (watt * 4.77);
  }

  void sensorUV() {
    int valorBruto = analogRead(uvSen);
    float uv = brutoaW(valorBruto);
    Serial.print("Irradiación UV: "); Serial.print(uv); Serial.println(" mW/cm²\t");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("UV: "); lcd.print(uv); lcd.print(" mW/cm2");
    delay(2000);
  }


void setup() {
  Serial.begin(9600);
  Serial.print('Inicio de prueba');

  rtc.begin(); //inicializa el reloj
  rtc.setHours(horas);
  rtc.setMinutes(minutos);
  rtc.setSeconds(segundos);
  rtc.setDay(dia);
  rtc.setMonth(mes);
  rtc.setYear(ano);

  if (tsl.begin())
  {
    Serial.println("Found a TSL2591 sensor");
  } else {
    Serial.println("No se encontro sensor de luz");
    error (4);
  }

  //Configuración del LCD
  lcd.begin(16, 2);
  lcd.print("Iniciando el sistema");

  strcpy(filename, "REG000.CSV");
  CreateFile();
}

void loop() {
  tiempo();
  sensorHRT();
  sensorUV();
  sensorLuz();
  String marcaTiempo = "";
  marcaTiempo += rtc.getHours();
  marcaTiempo += ":";
  marcaTiempo += rtc.getMinutes();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String cadenaDatos = "";
  cadenaDatos += marcaTiempo; cadenaDatos += ", ";
  cadenaDatos += String(h); cadenaDatos += ", "; cadenaDatos += String(t);
  Serial.println(cadenaDatos);

  //Abrir el archivo en la tarjeta SD
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // Si el archivo se pudo abrir entonces iniciar el registro
  if (dataFile) {
    dataFile.println(cadenaDatos);
    dataFile.close();
  } else {
    Serial.println("Error al abrir datalog.txt");
    lcd.clear(); lcd.setCursor(0,0);
    lcd.print("Error al abrir datalog.txt");
    delay (1000);
  }
}
