/***********************************************
Este programa permite generar ciclos de control cortos de minutos
en lugar de horas para verificar el correcto funcionamiento del equipo,
especialmente de los relevadores
Autor: mauriciobenjamin@gmail.com
Github: https://github.com/mauriciobenjamin/control-monitor
 ************************************************/
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <StandardCplusplus.h>
#include <RTCZero.h>
#include <serstream>
#include <string>
#include <vector>
#include <iterator>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "DHT.h"
#include "Adafruit_TSL2591.h"

#define DHTTYPE DHT22
const int BOTON_PIN_ADC  = A0;  // A0 is the boton ADC input
// ADC readings expected for the 5 botons on the ADC input
/*                      Con USB /  Con AC */
#define DER_ADC         1  // 8   // derecha 4
#define ARR_ADC         146 // 151  // up 154
#define ABJ_ADC         329 // 337  // down 344
#define IZQ_ADC         491 // 501  // left 513
#define SELEC_ADC       697 // 711  // derecha 728
#define NO_ADC          928 // No boton
#define HISTERESIS         35  // hysteresis for valid boton sensing window
/*-- CON USB
#define DER_ADC          4// derecha 4
#define ARR_ADC          151 // up 154
#define ABJ_ADC          336// down 344
#define IZQ_ADC          501 // left 513
#define SELEC_ADC        710 // derecha 728
#define NO_ADC           944 // No boton
#define HISTERESIS       5 //  hysteresis for valid boton sensing window--*/

//return values for leerBotones()
const byte BOTON_NO        = 0;
const byte BOTON_DER       = 1;
const byte BOTON_ARR       = 2;
const byte BOTON_ABJ       = 3;
const byte BOTON_IZQ       = 4;
const byte BOTON_SELEC     = 5;
const int ESPERA = -1;
//////////// Constantes de la tarjeta uSD ///////////////
const uint8_t tarjeta = 4; // Pin para la tarjeta SD
const uint8_t intMuestraMin = 0;  //Intervalo de muestreo en minutos
const unsigned long intMuestraSec = 14; //Intervalo de muestreo en segundos
const uint8_t muestraCiclo = 4; // Numero de muestras antes de hacer un limpiado de uSD
const uint muestrapArch = 8640; //Numero de muestras por archivo, considerando una muestra por minuto, se archivarían 30 días de mediciones en cada archivo.
uint8_t sigAlarmSeg = 0;
uint8_t sigAlarmMin = 0;
unsigned long contSeg;
bool alarmaAct = false;
extern "C" char *sbrk(int i); //Esto es para la función de FreeRAM
File archReg;

LiquidCrystal lcd( 5, 6, 9, 10, 11, 12 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )
RTCZero rtc;

char nombreArchiv[15];
float voltBateria;
uint8_t contadorMues;
long regresivo;

const int dhtpin = 13;
uint8_t contadorArch;
DHT dht(dhtpin, DHTTYPE);

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

const int uvSen = A1;


/*-----------------
Clases
-------------------*/

class Relevadores {
private:
  uint8_t relPin;
  bool relEst;

public:
  Relevadores (uint8_t pin) {
    relPin = pin;
    pinMode(relPin, OUTPUT);
    relEst = false;
    digitalWrite(relPin, relEst);

  };
  void actualRel(bool est) {
    relEst = est;
    digitalWrite(relPin, relEst);
  }

  bool edoRel() {
    return relEst;
  }
};

class condiciones {
  uint8_t humedad;
  uint8_t temperatura;
  uint8_t duracion;
  bool aspersor;
  bool luz;

public:
  condiciones (uint8_t hum, uint8_t temp, bool luces, uint8_t tiemp, bool asper)
  {
    humedad = hum;
    temperatura = temp;
    aspersor = asper;
    luz = luces;
    duracion = tiemp;
  };

  condiciones() {};

  void muestra(){
    lcd.setCursor(0,1);
    lcd.print(humedad);
    lcd.print(" % HR ");
    lcd.print(temperatura);
    lcd.print("C ");
    delay(2000);
    lcd.clear();
    lcd.home();
    if(aspersor) {
      lcd.print("Asper");
    } else {
      lcd.print("No Asper");
    }
    lcd.setCursor(9, 0);
    if(luz) {
      lcd.print("Luz");
    } else {
      lcd.print("No luz ");
    };
    lcd.setCursor(0, 1);
    lcd.print("Duracion ");
    lcd.print(duracion); lcd.print(" h");
    delay(200);
  };

  uint8_t usaH() {
    return humedad;
  };

  uint8_t usaT() {
    return temperatura;
  };

  bool usaAsp() {
    return aspersor;
  }

  bool usaLz() {
    return luz;
  };

  uint8_t usaD() {
    return duracion;
  };

  String reg_parametros() {
    String reg = "";
    reg = "Humedad: " + String(humedad) + ", Temperatura: " + String(temperatura) + ", Aspersion: " + String(aspersor) + ", Luz: " + String(luz) + ", Duración: " + String(duracion);
    return reg;
  }

};


/*--------------------------------------------------------------------------------------
Variables
--------------------------------------------------------------------------------------*/

//Variables del teclado
byte botonEdo   = BOTON_NO;
byte botonEdoUlt = botonEdo;
byte boton = BOTON_NO;
uint voltAnt = BOTON_NO;
uint botonVolt = BOTON_NO;
long tiempoAnt;
const long tiempoRef = 50;
bool cambio = false;

uint8_t ciclos = 1;
uint8_t etapas = 1;
uint16_t luxes;
uint8_t horas = 12;
uint8_t minutos = 30;
uint8_t segundos = 30;
uint8_t dia = 13;
uint8_t mes = 9;
uint8_t ano = 16;
float uv;
float hR;
float tA;
uint falta;
long retiene;
byte inicial;
byte etapaActual;
byte cicloActual;

Relevadores relHum(A2), relTem(A3), relAsp(A5), relLuz(A4);

std::vector<condiciones> para_etapas;
/*--------------------------------
FUNCIONES
___________________________*/

uint8_t defHumedad(), defTemperatura();
bool defLuz();
bool defAsper();
uint8_t defNumCiclos();
uint8_t defDuracion();
byte leerBotones();
void inicio();
void programar();
void parametros();
void variables();
void correr();
void reloj();
void guardar();
void seleccionar();
bool confirma();
void fijHoras();
void fijMinutos();
void fijSegundos();
void fijDia();
void fijMes();
void fijAno();
void muestraTiempo();
void muestra();
void dosDigitos(int val);
void registroInicial();
void datosPantalla();
void humPantalla();
void luzPantalla();
void regresivoPantalla();
void serialMed();
void controlReg();

void sensorHRT();
uint16_t sensorLuz();
float brutoaW(int bruto);
float sensorUV();

String marcaHora();
String marcaFecha();
String marcatiempo();
void regMed();
void alarmMatch();
String dosDigitosS();
uint8_t decrem(uint8_t var, uint16_t limite, uint8_t max, uint8_t incremento);
uint8_t increm(uint8_t var, uint16_t limite, uint8_t min, uint8_t incremento);
// Cambiar para el feather

/*-------
Estados de la máquina de estados
------------------------------------------------*/
enum estados { INICIO=0, RELOJ, SELECCIONAR, PROGRAMAR, PARAMETROS, GUARDAR, CORRER };
estados opEstados = INICIO;

enum tiempos { HORAS=0, MINUTOS, SEGUNDOS, DIA, MES, ANO, SALIR };
tiempos opReloj = HORAS;

enum Parametros { HUM=0, TEMP, ASPER, LUZ, DUR, DEFINIR };
Parametros opPara = HUM;

enum datos {hyT=0, lUv, cuenta, hhmm };
datos opDatos = hyT;

/*--------------------------------------------------------------------------------------
setup()
Called by the Arduino framework once, before the main loop begins
--------------------------------------------------------------------------------------*/
void setup()
{
  //boton adc input
  Serial.begin(9600);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  rtc.begin();
  lcd.begin( 16, 2 );
  pinMode(dhtpin, INPUT);
  pinMode( BOTON_PIN_ADC, INPUT );         //ensure A0 is an input
  lcd.clear(); lcd.setCursor( 0, 0 );   //top left
  lcd.print( "Control de Camar" );
  //
  lcd.setCursor( 0, 1 );   //bottom left
  //          1234567890123456
  lcd.print( "    Ambiental   " );
  delay(2000);
  Serial.println("Inicializando la tarjeta SD...");
  lcd.clear(); lcd.home(); lcd.print("  Inicia SD");
  if (!SD.begin(tarjeta)) {
    lcd.setCursor(0, 1);
    lcd.print("No hay tarjeta");
    Serial.print("No hay tarjeta");
    delay(2000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print(" Tarjeta lista ");
    Serial.println("Tarjeta lista");
  }
  delay(2000);

/* Esperando implementación de la SD
  strcpy(filename, "MONIT00.CSV");
  CreateFile();
*/
}

////////////////////// Loop Principal  /////////////////////
void loop() {
  while(leerBotones() != 0) {};
  lcd.clear();
  switch (opEstados) {
    case INICIO:
     inicio();
     break;
    case RELOJ:
      reloj();
      break;
  //  case SELECCIONAR:
  //    seleccionar();
  //    break;
    case PROGRAMAR:
     programar();
     break;
    case PARAMETROS:
     parametros();
     break;
  //  case GUARDAR:
  //    guardar();
  //    break;
    case CORRER:
     correr();
     break;
  }
}

void inicio() {
  uint8_t botones = 0;
  lcd.clear();
  lcd.home();
  lcd.print("Presione derecha");
  lcd.setCursor(0,1);
  lcd.print("para iniciar");
  while(botones != BOTON_DER) {
   botones = leerBotones();
   }
   opEstados = RELOJ;
   return;
}

void reloj() {
  lcd.clear();
  lcd.home();
  //         123456789123456
  lcd.print(" Hora del reloj");
  lcd.setCursor(4,1);
  lcd.print(" :    :");
  while(opReloj != SALIR) {
    switch (opReloj) {
      case HORAS:
        fijHoras();
        break;
      case MINUTOS:
        fijMinutos();
        delay(200);
        break;
      case SEGUNDOS:
        fijSegundos();
        delay(200);
        break;
      case DIA:
        fijDia();
        delay(200);
        break;
      case MES:
        fijMes();
        delay(200);
        break;
      case ANO:
        fijAno();
        delay(200);
        break;
    }
  }
  if(opEstados == INICIO) {
    return;
  }
  uint8_t botones = 0;
  while (true) {
    muestraTiempo();
    botones = leerBotones();
    if (botones == BOTON_SELEC) {
      opEstados = PROGRAMAR;
      return;
    }
    if (botones == BOTON_IZQ) {
      opEstados = INICIO;
      return;
    }
  }
}

/* FUNCIONES DEL RELOJ---------------------
// Aquí van todas las funciones para programar, mostrar el tiempo real y generar marcas de tiempo
----------------------------------------*/
void muestraTiempo() {
  lcd.setCursor(0, 0);
  lcd.print("Hora:  ");
  lcd.setCursor(0, 1);
  lcd.print("Fecha: ");
  uint8_t s = rtc.getSeconds();
  uint8_t h = rtc.getHours();
  uint8_t m = rtc.getMinutes();
  uint8_t d = rtc.getDay();
  uint8_t me = rtc.getMonth();
  uint8_t a = rtc.getYear();
  lcd.setCursor(7,0);
  dosDigitos(h); lcd.print(":"); dosDigitos(m); lcd.print(":"); dosDigitos(s);
  lcd.setCursor(7,1);
  dosDigitos(d); lcd.print("/"); dosDigitos(me); lcd.print("/"); dosDigitos(a);
}

void dosDigitos(int val) {
  if (val < 10) {
    lcd.print("0");
  }
  lcd.print(val);
}

String dosDigitosS(int val) {
  String numero = "";
  if (val < 10) {
    numero += "0";
  }
  numero += String(val);
  return numero;
}

void fijHoras() {
  uint8_t botones = 0;
  uint8_t h = horas;
  while (true) {
    botones = leerBotones();
    uint8_t incremento = 1;
    if (botones == BOTON_SELEC) {
      rtc.setHours(h);
      horas = h;
      opReloj = DIA;
      return;
    }
    if (botones == BOTON_IZQ) {
      rtc.setHours(h);
      horas = h;
      opEstados = INICIO;
      return;
    }
    if (botones == BOTON_DER) {
      rtc.setHours(h);
      horas = h;
      opReloj = MINUTOS;
      return;
    }
    if (botones == BOTON_ARR) {
      h = increm(h, 23, 0, 1);
    }
    if (botones == BOTON_ABJ) {
      h = decrem(h, 0, 23, 1);
    }
    lcd.setCursor(2,1);
    dosDigitos(h);
  }
}

void fijMinutos() {
  uint8_t botones = 0;
  uint8_t m = minutos;
  while (true) {
    botones = leerBotones();
    uint8_t incremento = 1;
    if (botones == BOTON_SELEC) {
      rtc.setMinutes(m);
      minutos = m;
      opReloj = DIA;
      return;
    }
    if (botones == BOTON_IZQ) {
      rtc.setMinutes(m);
      minutos = m;
      opReloj = HORAS;
      return;
    }
    if (botones == BOTON_DER) {
      rtc.setMinutes(m);
      minutos = m;
      opReloj = SEGUNDOS;
      return;
    }
    if (botones == BOTON_ARR) {
      m = increm(m, 59, 0, 1);
    }
    if (botones == BOTON_ABJ) {
      m = decrem(m, 0, 59, 1);
    }
    lcd.setCursor(7,1);
    dosDigitos(m);
  }
}

void fijSegundos() {
  uint8_t botones = 0;
  uint8_t s = segundos;
  while (true) {
    botones = leerBotones();
    uint8_t incremento = 1;
    if (botones == BOTON_SELEC) {
      rtc.setSeconds(s);
      segundos = s;
      opReloj = DIA;
      return;
    }
    if (botones == BOTON_IZQ) {
      rtc.setSeconds(s);
      segundos = s;
      opReloj = MINUTOS;
      return;
    }
    if (botones == BOTON_ARR) {
      s += incremento;
      if (s > 59) {
        s = 0;
      }
    }
    if (botones == BOTON_ABJ) {
      s -= incremento;
      if (s < 0 || s > 60) {
        s = 59;
      }
    }
    lcd.setCursor(12,1);
    dosDigitos(s);
  }
}

void fijDia() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Fecha DD/MM/AA ");
  uint8_t botones = 0;
  uint8_t d = dia;
  while (true) {
    botones = leerBotones();
    uint8_t incremento = 1;
    if (botones == BOTON_SELEC) {
      rtc.setDay(d);
      dia = d;
      opReloj = SALIR;
      return;
    }
    if (botones == BOTON_IZQ) {
      rtc.setDay(d);
      dia = d;
      opReloj = HORAS;
      return;
    }
    if (botones == BOTON_DER) {
      rtc.setDay(d);
      dia = d;
      opReloj = MES;
      return;
    }
    if (botones == BOTON_ARR) {
      d += incremento;
      if (d > 31) {
        d = 1;
      }
    }
    if (botones == BOTON_ABJ) {
      d -= incremento;
      if (d < 1) {
        d = 31;
      }
    }
    lcd.setCursor(2,1);
    if (d < 10) {
      lcd.print("0");
      lcd.print(d);
    } else {
      lcd.print(d);
    }
  }
}

void fijMes() {
  uint8_t botones = 0;
  uint8_t me = mes;
  lcd.setCursor(4,1);
  lcd.print(" /    /");
  while (true) {
    botones = leerBotones();
    uint8_t incremento = 1;
    if (botones == BOTON_SELEC) {
      rtc.setMonth(me);
      mes = me;
      opReloj = SALIR;
      return;
    }
    if (botones == BOTON_IZQ) {
      rtc.setMonth(me);
      mes = me;
      opReloj = DIA;
      return;
    }
    if (botones == BOTON_DER) {
      rtc.setMonth(me);
      mes = me;
      opReloj = ANO;
      return;
    }
    if (botones == BOTON_ARR) {
      me += incremento;
      if (me > 12 ) {
        me = 1;
      }
    }
    if (botones == BOTON_ABJ) {
      me -= incremento;
      if (me < 1) {
        me = 12;
      }
    }
    lcd.setCursor(7, 1);
    if (me < 10) {
      lcd.print("0");
      lcd.print(me);
    } else {
      lcd.print(me);
    }
  }
}

void fijAno() {
  uint8_t botones = 0;
  uint8_t a = ano;
  while (true) {
    botones = leerBotones();
    uint8_t incremento = 1;
    if (botones == BOTON_SELEC) {
      rtc.setYear(a);
      ano = a;
      opReloj = SALIR;
      return;
    }
    if (botones == BOTON_IZQ) {
      rtc.setYear(a);
      ano = a;
      opReloj = MES;
      return;
    }
    if (botones == BOTON_ARR) {
      a += incremento;
      if (a > 99) {
        a = 0;
      }
    }
    if (botones == BOTON_ABJ) {
      a -= incremento;
      if (a < 0) {
        a = 99;
      }
    }
    lcd.setCursor(12, 1);
    if (a < 10) {
      lcd.print("0");
      lcd.print(a);
    } else {
      lcd.print(a);
    }
  }
}

String marcatiempo() {
  String hS = dosDigitosS(rtc.getHours());
  String minS = dosDigitosS(rtc.getMinutes());
  String secS = dosDigitosS(rtc.getSeconds());
  String dS = dosDigitosS(rtc.getDay());
  String meS = dosDigitosS(rtc.getMonth());
  String aS = dosDigitosS(rtc.getYear());
  String marca = "";
  marca = dS + "/" + meS + "/" + aS + ", " + hS + ":" + minS + ":" + secS;
  return marca;
}

String marcaFecha() {
  String dS = dosDigitosS(rtc.getDay());
  String meS = dosDigitosS(rtc.getMonth());
  String aS = dosDigitosS(rtc.getYear());
  String marca = "";
  marca = dS + "/" + meS + "/" + aS;
  return marca;
}

String marcaHora() {
  String hS = dosDigitosS(rtc.getHours());
  String minS = dosDigitosS(rtc.getMinutes());
  String secS = dosDigitosS(rtc.getSeconds());
  String marca = "";
  marca = hS + ":" + minS + ":" + secS;
  return marca;
}

/*------- FUNCIONES DE ADMINISTRACIÖN DE DATOS ------------------
// Aquí van las funciones que leen las programaciones ya guardadas, guardan
 programaciones nuevas-----------------------------------------

void seleccionar() {
  lcd.clear(); lcd.home(); lcd.print("Desea usar un ");
  lcd.setCursor(0,1); lcd.print("ciclo previo ");
  uint8_t botones = 0;
  bool selecc = true;
  while (true) {
    botones = leerBotones();
    if (botones == BOTON_SELEC && selecc == true) {
      selec2();
      return;
    }
    if (botones == BOTON_SELEC && selecc == false) {
      opEstados = PROGRAMAR;
      return;
    }
    if (botones == BOTON_IZQ) {
      opEstados = RELOJ;
      return;
    }
    if (botones == BOTON_ABJ) {
      selecc = decrem(selecc, 0, 1, 1);
    }
    if (botones == BOTON_ARR) {
      selecc = increm(selecc, 1, 0, 1);
    }
    lcd.setCursor(14,1);
    if (selecc) {
      lcd.print("Si");
    } else {
      lcd.print("No")
    }
  }
}

void selec2() {
  lcd.clear(); lcd.home(); lcd.print("Protocolo a usar");
  uint8_t botones = 0;
  File datosGuardados = SD.open()
  while (true) {
    botones = leerBotones();
    if (botones = BOTON_SELEC) {
      opEstados = CORRER;
      return;
    }
    if (botones = BOTON_ARR) {

    }

  }
}

void muestraArch(File dir) {
  uint8_t numArchivos;
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    numArchivos ++;
    entry.close();
  }
  char listaArch[numArchivos];
  for (size_t i = 0; i < numArchivos; i++) {
    File entry = dir.openNextFile();
    listaArch.push_back(entry);
  }

/*------- FUNCIONES PARA LA PROGRAMACIÖN DE PARAMETROS -------------
// Aquí van las funciones para programar los parametros de funcionamiento
// del dispositivo ---------------------------------------------*/

void programar() {
  lcd.clear();
  lcd.setCursor(0,0);
  /*         123567890123456*/
  lcd.print("Cuantas");
  lcd.setCursor(0,1);
  lcd.print("etapas?");
  uint8_t botones = 0;
  while(true)
  {
     botones = leerBotones();
     uint8_t incremento = 1;
     if (botones == BOTON_SELEC) {
       opEstados = PARAMETROS;
       return;
     }
     if (botones == BOTON_IZQ) {
       opEstados = RELOJ;
       return;
     }
     if (botones == BOTON_DER) {
       opEstados = PARAMETROS;
       return;
     }
     if (botones == BOTON_ARR) {
       etapas = increm(etapas, 50, 1, 1);
     }
     if (botones == BOTON_ABJ) {
       etapas = decrem(etapas, 1, 50, 1);
     }
     lcd.setCursor(10,1);
     dosDigitos(etapas);
  }
}

void parametros() {
  lcd.clear();
  lcd.home();
  lcd.print("Etapa ");
  for (size_t i = 0; i < etapas; i++) {
    lcd.setCursor(6, 0); lcd.print(i+1);
    lcd.setCursor(0, 1);
    opPara = HUM;
    uint8_t valHum, valTem;
    bool valLuz;
    bool valAsper;
    uint8_t valDur;
    while (opPara != DEFINIR) {
      switch (opPara) {
        case HUM:
        valHum = defHumedad();
        delay(200);
        break;
        case TEMP:
        valTem = defTemperatura();
        delay(200);
        break;
        case LUZ:
        valLuz = defLuz();
        delay(200);
        break;
        case ASPER:
        valAsper = defAsper();
        delay(200);
        break;
        case DUR:
        valDur = defDuracion();
        delay(200);
        break;
      }
    }
    Serial.println(valHum);
    Serial.println(valTem);
    Serial.println(valAsper);
    Serial.println(valLuz);
    Serial.println(valDur);
    Serial.println("Asi entro el dato");
    if (opEstados == PROGRAMAR) {
      return;
    }
    condiciones cond_etapa(valHum, valTem, valLuz, valDur, valAsper);
    para_etapas.push_back(cond_etapa);
    delay(200);
  }
  ciclos = defNumCiclos();
  //Solo para verificar que los datos sean correctos
  uint8_t botones = 0;
  for (size_t i = 0; i < etapas; i++) {
    Serial.print("Etapa"); Serial.println(i+1);
    Serial.print("Humedad: ");Serial.println(para_etapas[i].usaH());
    Serial.print("Temperatura: ");Serial.println(para_etapas[i].usaT());
    Serial.print("Aspersion: ");Serial.println(para_etapas[i].usaAsp());
    Serial.print("Luz: ");Serial.println(para_etapas[i].usaLz());
    Serial.print("Duración: ");Serial.println(para_etapas[i].usaD());
    lcd.clear();
    lcd.home();
    lcd.print("Etapa: "); lcd.print(i+1);
    para_etapas[i].muestra();
    botones = 0;
    while (botones != BOTON_DER) {
      botones = leerBotones();
      if (botones == BOTON_IZQ) {
        i--;
        if (i < 0) {
          i = 0;
        }
      }
    };
  }

  lcd.clear();
  lcd.home(); lcd.print("Iniciar");
  lcd.setCursor(0, 1); lcd.print("Presiona Select");
  botones = 0;
  while (true) {
    botones = leerBotones();
    if (botones == BOTON_IZQ) {
      opEstados = PROGRAMAR;
      return;
    }
    if (botones == BOTON_SELEC) {
      opEstados = CORRER;
      return;
    }
  }
}


void registroInicial() {
  String datos = "";
  strcpy(nombreArchiv, "REGIST00.TXT");
  for (size_t i = 0; i < 100; i++) {
    nombreArchiv[6] = '0' + i/10;
    nombreArchiv[7] = '0' + i%10;
    if (!SD.exists(nombreArchiv)) {
      break;
    }
  }
  archReg = SD.open(nombreArchiv, FILE_WRITE);
  if (archReg) {
    archReg.println("Registro de prueba de intemperismo");
    archReg.println("Inicio de prueba: "+ marcatiempo());
    archReg.print("Etapas por ciclo: "); archReg.println(etapas);
    archReg.print("Ciclos programados: "); archReg.println(ciclos);
    archReg.println("Parametros por etapa:");
    for (size_t i = 0; i < etapas; i++) {
      archReg.print("Etapa "); archReg.println(i+1);
      archReg.println("Parametros: " + para_etapas[i].reg_parametros());
    }
    archReg.println("Fecha, Hora, Humedad, Temperatura, Luz, Uv, Humidificador, IR, Luces, Aspersión, Ciclo, Etapa");
    archReg.close();

    Serial.println("Registro de prueba de intemperismo");
    Serial.println("Registro en " + String(nombreArchiv));
    Serial.println("Inicio de prueba: "+ marcatiempo());
    Serial.println("Etapas por ciclo: " + String(etapas));
    Serial.println("Ciclos programados: " + String(ciclos));
    Serial.println("Parametros por etapa:");
    for (size_t i = 0; i < etapas; i++) {
      Serial.println("Etapa " + String(i+1));
      Serial.println("Parametros: " + para_etapas[i].reg_parametros());
    }
    Serial.println("Fecha, Hora, Humedad, Temperatura, Luz, Uv, Humidificador, IR, Luces, Aspersión, Ciclo, Etapa");
  } else {
    lcd.clear(); lcd.home();
    lcd.print("Error en archivo");
    Serial.println("Error en archivo");
  }
}

void controlReg() {
  lcd.clear(); lcd.home();
  uint8_t botones = 0;
  int contCiclos = 1;
  regresivo = 0;
  while (ciclos >= contCiclos || ciclos == 0) {
    cicloActual = contCiclos;
    for (size_t i = 0; i < etapas; i++) {
      etapaActual = i;
      if (para_etapas[i].usaAsp() && !relAsp.edoRel()) {
        relAsp.actualRel(HIGH);
        Serial.println("Relevador Lluvia encendido");
      } else if (!para_etapas[i].usaAsp() && relAsp.edoRel()) {
        relAsp.actualRel(LOW);
        Serial.println("Relevador Lluvia apagado");
      }
      if (para_etapas[i].usaLz() && !relLuz.edoRel()) {
        relLuz.actualRel(HIGH);
        Serial.println("Relevador Luz encendido");
      }
      if (!para_etapas[i].usaLz() && relLuz.edoRel()) {
        relLuz.actualRel(LOW);
        Serial.println("Relevador Luz apagado");
      }
      uint8_t duracionC = para_etapas[i].usaD();
      Serial.print("Se registra una duración de "); Serial.println(duracionC);
      regresivo = duracionC * 4;
      regMed();
      archReg.flush();
      float hrPromedio;
      float temPromedio;
      int luxPromedio;
      float uvPromedio;
      while(regresivo > 0) {
        sigAlarmSeg = rtc.getSeconds();
        botones = leerBotones();
        if (botones == BOTON_IZQ) {
          archReg.close();
          bool confir = confirma();
          if (confir) {
            return;
          }
        }
        if (contadorMues >= muestraCiclo) {
          hrPromedio = hrPromedio / muestraCiclo;
          temPromedio = temPromedio / muestraCiclo;
          luxPromedio = luxPromedio / muestraCiclo;
          uvPromedio = uvPromedio / muestraCiclo;
          archReg = SD.open(nombreArchiv, FILE_WRITE);
          String cadenaDatos = "";
          cadenaDatos += marcatiempo();
          cadenaDatos += ", "; cadenaDatos += hrPromedio;        cadenaDatos += ", "; cadenaDatos += temPromedio;         cadenaDatos += ", "; cadenaDatos += luxPromedio;
          cadenaDatos += ", "; cadenaDatos += uvPromedio;
          cadenaDatos += ", "; cadenaDatos += relHum.edoRel();
          cadenaDatos += ", "; cadenaDatos += relTem.edoRel();
          cadenaDatos += ", "; cadenaDatos += relLuz.edoRel();
          cadenaDatos += ", "; cadenaDatos += relAsp.edoRel();
          cadenaDatos += ", "; cadenaDatos += cicloActual;
          cadenaDatos += ", "; cadenaDatos += (etapaActual+1);
          archReg.println(cadenaDatos);
          contadorMues = 0;
          Serial.println("Se guardaron datos");
          archReg.flush();
          hrPromedio = 0;
          temPromedio = 0;
          luxPromedio = 0;
          uvPromedio = 0;
        }
        if(alarmaAct == false) {
          sigAlarmSeg = (sigAlarmSeg + intMuestraSec)%60;
          rtc.setAlarmSeconds(sigAlarmSeg);
          rtc.enableAlarm(rtc.MATCH_SS);
          alarmaAct = true;
          hrPromedio += hR;
          temPromedio += tA;
          luxPromedio += luxes;
          uvPromedio += uv;
          contadorMues ++;
          regresivo --;
          falta = regresivo;
          serialMed();
          Serial.print("Faltan "); Serial.println(falta/4);
          rtc.attachInterrupt(alarmMatch);
        }
        if(hR < para_etapas[i].usaH() && !relHum.edoRel()) {
          relHum.actualRel(HIGH);
          Serial.println("Se activo Humidificador");
        }
        if (hR > para_etapas[i].usaH()){
          if (relHum.edoRel()) {
            relHum.actualRel(LOW);
            Serial.println("Se apago el Humidificador");
          }
        }
        if (tA < para_etapas[i].usaT() && !relTem.edoRel()) {
          relTem.actualRel(HIGH);
          Serial.println("Se activo el IR");
        }
        if (tA > para_etapas[i].usaT() && relTem.edoRel()){
          relTem.actualRel(LOW);
          Serial.println("Se apago el IR");
        }
        datosPantalla();
      }
    }
    contCiclos ++;
  }
  archReg.close();
  opEstados = INICIO;
  relHum.actualRel(LOW);
  relTem.actualRel(LOW);
  relLuz.actualRel(LOW);
  relAsp.actualRel(LOW);
  return;
}

void alarmMatch() {
  alarmaAct = false;
}

//Funciones de los sensores

uint16_t sensorLuz() {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  luxes = tsl.calculateLux(full, ir);
  /** Serial.print("Luminancia: "); Serial.print(tsl.calculateLux(full, ir)); Serial.println(" Lux\t");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Lum: "); lcd.print(tsl.calculateLux(full, ir)); lcd.print("Lux"); **/
  return luxes;
}

float brutoaW(int bruto) {
    float watt = bruto * 3.3 / 1024;
    return (watt * 4.77);
  }

float sensorUV() {
  int valorBruto = analogRead(uvSen);
  uv = brutoaW(valorBruto);
  return uv;
}
//Función para crear una linea de registro en el archivo

void regMed() {
  hR = dht.readHumidity();
  tA = dht.readTemperature();
  uint16_t luxesA = sensorLuz();
  float uvA = sensorUV();
  archReg = SD.open(nombreArchiv, FILE_WRITE);
  String cadenaDatos = "";
  cadenaDatos += marcatiempo();
  cadenaDatos += ", "; cadenaDatos += hR;
  cadenaDatos += ", "; cadenaDatos += tA;
  cadenaDatos += ", "; cadenaDatos += luxesA;
  cadenaDatos += ", "; cadenaDatos += uvA;
  cadenaDatos += ", "; cadenaDatos += relHum.edoRel();
  cadenaDatos += ", "; cadenaDatos += relTem.edoRel();
  cadenaDatos += ", "; cadenaDatos += relLuz.edoRel();
  cadenaDatos += ", "; cadenaDatos += relAsp.edoRel();
  cadenaDatos += ", "; cadenaDatos += cicloActual;
  cadenaDatos += ", "; cadenaDatos += etapaActual;
  archReg.println(cadenaDatos);
}

//Función para crear una linea de registro para monitoreo serial, SE PUEDE BORRAR en la definitiva.
void serialMed() {
  uint16_t luxesA = sensorLuz();
  float uvA = sensorUV();
  String cadenaDatos = "";
  cadenaDatos += marcatiempo();
  cadenaDatos += ", "; cadenaDatos += hR;
  cadenaDatos += ", "; cadenaDatos += tA;
  cadenaDatos += ", "; cadenaDatos += luxesA;
  cadenaDatos += ", "; cadenaDatos += uvA;
  cadenaDatos += ", "; cadenaDatos += relHum.edoRel();
  cadenaDatos += ", "; cadenaDatos += relTem.edoRel();
  cadenaDatos += ", "; cadenaDatos += relLuz.edoRel();
  cadenaDatos += ", "; cadenaDatos += relAsp.edoRel();
  cadenaDatos += ", "; cadenaDatos += cicloActual;
  cadenaDatos += ", "; cadenaDatos += etapaActual;
  Serial.println(cadenaDatos);
}

void datosPantalla() {
  switch (opDatos) {
    case hyT:
      if (!inicial) {
        retiene = millis();
        inicial = 1;
      }
      if (inicial == 1) {
        humPantalla();
        inicial ++;
      }
      if (retiene + 3000 < millis()) {
        opDatos = lUv;
      }
      break;
    case lUv:
      if (retiene + 6000 < millis()) {
        opDatos = cuenta;
      }
      if (inicial == 2) {
        luzPantalla();
        inicial ++;
      }
      break;
    case cuenta:
      if (inicial == 3) {
        regresivoPantalla();
        inicial ++;
      }
      if (retiene + 9000 < millis()) {
        opDatos = hhmm;
      }
      break;
    case hhmm:
      muestraTiempo();
      if (retiene + 12000 < millis()) {
        opDatos = hyT;
        inicial = 0;
      }
      break;
  }
}

void humPantalla() {
  sensorHRT();
  lcd.clear(); lcd.home(); lcd.print("Humedad: "); lcd.print(hR); lcd.print("%");
  lcd.setCursor(0,1); lcd.print("Temp: "); lcd.print(tA);
}

void luzPantalla() {
  sensorLuz();
  sensorUV();
  lcd.clear(); lcd.home(); lcd.print("Luz: "); lcd.print(luxes); lcd.print(" Lux");
  lcd.setCursor(0, 1); lcd.print("UV: "); lcd.print(uv); lcd.print(" mW/cm2");
}

void regresivoPantalla() {
  lcd.clear(); lcd.home(); lcd.print("Quedan ");
  dosDigitos(falta/4); lcd.print(":"); dosDigitos((falta%4)*15);
  lcd.setCursor(0, 1); lcd.print("para completar");
}

uint8_t defHumedad() {
  lcd.setCursor(0, 1);
           //123456789123456
  lcd.print("HR       %      ");
  uint8_t botones = 0;
  uint8_t valHum = 50;
  while (true) {
   botones = leerBotones();
   if (botones == BOTON_SELEC){
     opPara = TEMP;
     return valHum;
   }
   if (botones == BOTON_IZQ) {
     opEstados = PARAMETROS;
     opPara = DEFINIR;
     return valHum;
   }
   if (botones == BOTON_ARR) {
     valHum = increm(valHum, 99, 10, 1);
   }
   if (botones == BOTON_ABJ) {
     valHum = decrem(valHum, 10, 99, 1);
   }
   if (botones == BOTON_DER) {
     opPara = TEMP;
     return valHum;
   }
   lcd.setCursor(6, 1);
   dosDigitos(valHum);
  }
}

uint8_t defTemperatura() {
  lcd.setCursor(0, 1);
  lcd.print("Temp        C   ");
  uint8_t valTem = 20;
  uint8_t botones = 0;
  while (true) {
   botones = leerBotones();
   if (botones == BOTON_SELEC) {
     opPara = LUZ;
     return valTem;
   }
   if (botones == BOTON_IZQ) {
     opPara = HUM;
     return valTem;
   }
   if (botones == BOTON_DER) {
     opPara = LUZ;
     return valTem;
   }
   if (botones == BOTON_ARR) {
     valTem = increm(valTem, 80, 0, 1);
   }
   if (botones == BOTON_ABJ) {
     valTem = decrem(valTem, 0, 80, 1);
   }
   lcd.setCursor(6, 1);
   dosDigitos(valTem);
  };
}

void sensorHRT() {
  unsigned long tLecturas = 2000;
  unsigned long tActual = millis();
  if(tLecturas < tActual - tiempoAnt) {
    tiempoAnt = tActual;
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
  return hR tA;
}


/* Función para fijar la presencia/ausencia de luz en el ciclo*/
bool defLuz() {
  lcd.setCursor(0, 1);
  lcd.print("Con luz?        ");
  uint8_t botones = 0;
  bool valLuz = false;
  while (true) {
    botones = leerBotones();
    if (botones == BOTON_SELEC) {
      opPara = ASPER;
      return valLuz;
    }
    if (botones == BOTON_IZQ) {
      opPara = TEMP;
      return valLuz;
    }
    if (botones == BOTON_DER) {
      opPara = ASPER;
      return valLuz;
    }
    if (botones == BOTON_ARR) {
      valLuz = increm(valLuz, 1, 0, 1);
    }
    if (botones == BOTON_ABJ) {
      valLuz = decrem(valLuz, 0, 1, 1);
    }
    lcd.setCursor(9,1);
    if (valLuz) {
      lcd.print("Si");
    } else {
      lcd.print("No");
    }
  };
}

bool defAsper() {
  lcd.setCursor(0,1);
  lcd.print("Lluvia?         ");
  bool chipotle = 0;
  uint8_t botones = 0;
  while(true) {
    botones = leerBotones();
    if (botones == BOTON_SELEC) {
      opPara = DUR;
      Serial.println(chipotle);
      return chipotle;
    }
    if (botones == BOTON_IZQ) {
      opPara = LUZ;
      Serial.println(chipotle);
      return chipotle;
    }
    if (botones == BOTON_DER) {
      opPara = DUR;
      Serial.println(chipotle);
      return chipotle;
    }
    if (botones == BOTON_ARR) {
      chipotle = increm(chipotle, 1, 0, 1);
    }
    if (botones == BOTON_ABJ) {
      chipotle = decrem(chipotle, 0, 1, 1);
    }
    lcd.setCursor(9,1);
    if (chipotle) {
      lcd.print("Si");
    } else {
      lcd.print("No");
    }
  }
}

uint8_t defDuracion() {
  lcd.setCursor(0, 1);
  lcd.print("Duracion     min");
  uint8_t valDur = 16;
  uint8_t botones = 0;
  while (true) {
   botones = leerBotones();
   if (botones == BOTON_SELEC) {
     opPara = DEFINIR;
     return valDur;
   }
   if (botones == BOTON_IZQ) {
     opPara = LUZ;
     return valDur;
   }
   if (botones == BOTON_ARR) {
     valDur = increm(valDur, 24, 1, 1);
   }
   if (botones == BOTON_ABJ) {
     valDur = decrem(valDur, 1, 24, 1);
   }
   lcd.setCursor(10, 1);
   dosDigitos(valDur);
  };
}

uint8_t defNumCiclos() {
  lcd.clear();
  lcd.home(); lcd.print("Numero de ");
  lcd.setCursor(0, 1);
  //         1234567890123456
  lcd.print("Ciclos          ");
  uint8_t botones = 0;
  while (true) {
   botones = leerBotones();
   if (botones == BOTON_SELEC) {
     return ciclos;
   }
   if (botones == BOTON_IZQ) {
     opEstados = PARAMETROS;
     return ciclos;
   }
   if (botones == BOTON_DER) {
    return ciclos;
   }
   if (botones == BOTON_ARR) {
     ciclos = increm(ciclos, 100, 0, 1);
   }
   if (botones == BOTON_ABJ) {
     ciclos = decrem(ciclos, 0, 100, 1);
   }
   lcd.setCursor(8, 1);
   dosDigitos(ciclos);
  };
}

void correr() {
  registroInicial();
  controlReg();
  return;
}

bool confirma() {
  lcd.clear(); lcd.home();
  lcd.print("Confirma reinicio");
  uint8_t botones = 0;
  lcd.setCursor(7,1);
  lcd.print("No");
  bool confir = false;
  while (true) {
    botones = leerBotones();
    if (botones == BOTON_ARR) {
      lcd.setCursor(7,1);
      lcd.print("Si");
      confir = true;
    }
    if (botones == BOTON_ABJ) {
      lcd.setCursor(7,1);
      lcd.print("No");
      confir = false;
    }
    if (botones == BOTON_SELEC && confir) {
      return confir;
    }
  }
}

uint8_t increm(uint8_t var, uint16_t limite, uint8_t mini, uint8_t incremento) {
  var += incremento;
  if (var > limite || var < mini) {
    var = mini;
  }
  return var;
}

uint8_t decrem(uint8_t var, uint16_t limite, uint8_t max, uint8_t incremento) {
  var -= incremento;
  if (var < limite || var > max) {
    var = max;
  }
  return var;
}

byte leerBotones() {
  botonVolt = analogRead(BOTON_PIN_ADC);
  if(botonVolt <= voltAnt + HISTERESIS && botonVolt >= voltAnt - HISTERESIS){
    cambio = false;
  } else if(botonVolt <= voltAnt + 3 && botonVolt >= voltAnt) {
     cambio = false;
  } else {
    cambio = true;
  }
  if(cambio) {
    if(botonVolt <= ( DER_ADC + HISTERESIS )){
      botonEdo = BOTON_DER;
    }
    else if(botonVolt >= ( ARR_ADC - HISTERESIS )  && botonVolt <= ( ARR_ADC + HISTERESIS )){
      botonEdo = BOTON_ARR;
    }
    else if(botonVolt >= ( ABJ_ADC - HISTERESIS ) && botonVolt <= ( ABJ_ADC + HISTERESIS ) ) {
      botonEdo = BOTON_ABJ;
    }
    else if(botonVolt >= ( IZQ_ADC - HISTERESIS ) && botonVolt <= ( IZQ_ADC + HISTERESIS ) ) {
      botonEdo = BOTON_IZQ;
    }
    else if(botonVolt >= ( SELEC_ADC - HISTERESIS ) && botonVolt <= ( SELEC_ADC + HISTERESIS )) {
      botonEdo = BOTON_SELEC;
    }
    else if (botonVolt > NO_ADC) {
      botonEdo = BOTON_NO;
    }
    voltAnt = botonVolt;
  }
  if ((millis() - tiempoAnt) > tiempoRef && botonEdoUlt == BOTON_NO) {
    boton = botonEdo;
  } else {
    boton = BOTON_NO;
  }
  if (cambio) {
    tiempoAnt = millis();
  }
  if (!cambio) {
      boton = BOTON_NO;
  }
  botonEdoUlt = botonEdo;
  return boton;
}
