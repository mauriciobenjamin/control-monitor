// ************************************************
// Programar una secuencia propia de pruebas
// Arriba/Abajo para cambiar los valores de cada parametro
// Derecha para aceptar un parametro y pasar al siguiente
// Izquierda para regresar al parametro anterior
// Seleccionar/Shift para cambiar de 10 en 10 (revisar si no se necesita otra funcionalidad después)
// ************************************************
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


#define BOTON_PIN_ADC           A0  // A0 is the boton ADC input
// ADC readings expected for the 5 botons on the ADC input
#define DER_ADC            8  // derecha
#define ARR_ADC          154  // up
#define ABJ_ADC          338  // down
#define IZQ_ADC          500  // left
#define SELEC_ADC        708  // derecha
#define HISTERESIS         7  // hysteresis for valid boton sensing window
//return values for leerBotones()
const byte BOTON_NO        = 0;
const byte BOTON_DER       = 1;
const byte BOTON_ARR       = 2;
const byte BOTON_ABJ       = 3;
const byte BOTON_IZQ       = 4;
const byte BOTON_SELEC     = 5;
const int ESPERA = -1;
#define tarjeta   4 // Pin para la tarjeta SD
#define intMuestraMin 5  //Intervalo de muestreo en minutos
#define intMuestraSec 00 //Intervalo de muestreo en segundos
#define muestraCiclo 6 // Numbero de muestras antes de hacer un limpiado de uSD
#define muestrapArch 8640 //Numbero de muestras por archivo, considerando una muestra por minuto, se archivarían 30 días de mediciones en cada archivo.
extern "C" char *sbrk(int i); //Esto es para la función de FreeRAM
/*--------------------------------------------------------------------------------------
Init the LCD library with the LCD pins to be used
--------------------------------------------------------------------------------------*/
LiquidCrystal lcd( 11, 12, 5, 6, 9, 10 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )
RTCZero rtc;
// Esperando implmentación de la SD SD sd;
File registro;
char nombreArchiv[15];
float voltBateria;
uint8_t contadorMues;
uint8_t contadorArch;

/*-----------------
Clases
-------------------*/

class condiciones {
  uint8_t humedad;
  uint8_t temperatura;
  int duracion;
  uint8_t lluvia;
  uint8_t luz;
public:
  condiciones (uint8_t hum, uint8_t temp, uint8_t lluv, uint8_t luces, int tiemp)
  {
    humedad = hum;
    temperatura = temp;
    luz = luces;
    lluv = lluvia;
    duracion = tiemp;
  };
  condiciones() {};
  void muestra(){
    long dura = millis();
    lcd.setCursor(0,1);
    lcd.print(humedad);
    lcd.print(" % HR ");
    lcd.print(temperatura);
    lcd.print("C ");
    while (dura + 2000 > millis()) {
    }
    lcd.clear();
    lcd.home();
    if(luz == 1) {
      lcd.print("Luz");
    } else {
      lcd.print("No luz ");
    };
    lcd.setCursor(7, 0);
    if(lluvia == 1) {
      lcd.print("LLuvia");
    } else {
      lcd.print("No lluvia ");
    };
    lcd.setCursor(0, 1);
    lcd.print("Duracion ");
    lcd.print(duracion); lcd.print(" min");
  };

  uint8_t usaH() {
    return humedad;
  };

  uint8_t usaT() {
    return temperatura;
  };

  uint8_t usaLz() {
    return luz;
  };

  uint8_t usaLl() {
    return lluvia;
  };

  uint8_t usaD() {
    return duracion;
  };

};

/*--------------------------------------------------------------------------------------
Variables
--------------------------------------------------------------------------------------*/

//Variables del teclado
byte botonPresionado  = false;
byte botonLiberado = false;
byte botonEdo   = BOTON_NO;
byte botonEdoUlt = botonEdo;
uint voltAnt = BOTON_NO;
uint botonVolt = BOTON_NO;
long tiempoAnt = 0;
const long tiempoRef = 100;
long apretarLargo = 1000;
bool largo = false;
bool cambio = false;

uint8_t etapas = 1;
uint16_t luxes;
uint16_t uv;
uint8_t hrAmb;
uint8_t tAmb;
int rel1 = 10;
int rel2 = 11;
uint8_t horas = 12;
uint8_t segundos = 00;
uint8_t minutos = 00;
uint8_t dia = 23;
uint8_t mes = 8;
uint8_t ano = 16;

std::vector<condiciones> pasos;
/*--------------------------------
FUNCIONES
___________________________*/

uint8_t defHumedad(), defTemperatura();
uint8_t defLuz(), defRocio();
int defDuracion();
byte leerBotones();
void inicio();
void programar();
void parametros();
void variables();
void correr();
void reloj();
void fijHoras();
void fijMinutos();
void fijSegundos();
void fijDia();
void fijMes();
void fijAno();
void muestraTiempo();
void muestra();
void dosDigitos();
uint8_t decrem(uint8_t var, uint16_t limite, uint8_t max, uint8_t incremento);
uint8_t increm(uint8_t var, uint16_t limite, uint8_t min, uint8_t incremento);
// Cambiar para el feather

/*-------
Estados de la máquina de estados
------------------------------------------------*/
enum estados { INICIO=0, RELOJ, PROGRAMAR, PARAMETROS, CORRER };
estados opEstados = INICIO;

enum tiempos { HORAS=0, MINUTOS, SEGUNDOS, DIA, MES, ANO };
tiempos opReloj = HORAS;


/*--------------------------------------------------------------------------------------
setup()
Called by the Arduino framework once, before the main loop begins
--------------------------------------------------------------------------------------*/
void setup()
{
  //boton adc input
  Serial.begin(9600);
  pinMode(rel1, OUTPUT);
  pinMode(rel2, OUTPUT);
  pinMode( BOTON_PIN_ADC, INPUT );         //ensure A0 is an input
  rtc.begin();
  //set up the LCD number of columns and rows:
  lcd.begin( 16, 2 );
  //Print some initial text to the LCD.
  lcd.setCursor( 0, 0 );   //top left
  //          1234567890123456
  lcd.print( "Control de Camar" );
  //
  lcd.setCursor( 0, 1 );   //bottom left
  //          1234567890123456
  lcd.print( "    Ambiental   " );
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
   case PROGRAMAR:
     programar();
     break;
   case PARAMETROS:
     parametros();
     break;
   case CORRER:
     programar();
     break;
}
uint8_t botones = leerBotones();
Serial.print(botones);
Serial.print(" ");
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
   reloj();
}

void reloj() {
  opEstados = RELOJ;
  lcd.clear();
  lcd.home();
  //         123456789123456
  lcd.print(" Hora del reloj");
  lcd.setCursor(4,1);
  lcd.print(" :    :");
  fijHoras();
}

void muestraTiempo() {
  uint8_t botones = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  lcd.setCursor(0, 1);
  lcd.print("Fecha: ");
  while (true) {
    uint8_t h = rtc.getHours();
    uint8_t m = rtc.getMinutes();
    uint8_t s = rtc.getSeconds();
    uint8_t d = rtc.getDay();
    uint8_t me = rtc.getMonth();
    uint8_t a = rtc.getYear();
    lcd.setCursor(6,0);
    dosDigitos(h); lcd.print(":"); dosDigitos(m); lcd.print(":"); dosDigitos(s);
    lcd.setCursor(7,1);
    dosDigitos(d); lcd.print("/"); dosDigitos(me); lcd.print("/"); dosDigitos(a);
    botones = leerBotones();
    if (botones == BOTON_SELEC) {
      programar();
    }
    if (botones == BOTON_IZQ) {
      fijDia();
    }
    if (botones == BOTON_DER)
    {
      programar();
    }
  }
}

void dosDigitos(uint8_t val) {
  if (val < 10) {
    lcd.print("0");
    lcd.print(val);
  } else {
    lcd.print(val);
  }
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
      fijDia();
    }
    if (botones == BOTON_IZQ) {
      rtc.setHours(h);
      horas = h;
      inicio();
    }
    if (botones == BOTON_DER) {
      rtc.setHours(h);
      horas = h;
      fijMinutos();
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
      fijDia();
    }
    if (botones == BOTON_IZQ) {
      rtc.setMinutes(m);
      minutos = m;
      fijHoras();
    }
    if (botones == BOTON_DER) {
      rtc.setMinutes(m);
      minutos = m;
      fijSegundos();
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
      fijDia();
    }
    if (botones == BOTON_IZQ) {
      rtc.setSeconds(s);
      segundos = s;
      fijMinutos();
    }
    if (botones == BOTON_DER) {
      rtc.setSeconds(s);
      segundos = s;
      fijDia();
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
      delay(200);
      rtc.setDay(d);
      dia = d;
      muestraTiempo();
    }
    if (botones == BOTON_IZQ) {
      delay(200);
      rtc.setDay(d);
      dia = d;
      reloj();
    }
    if (botones == BOTON_DER) {
      delay(200);
      rtc.setDay(d);
      dia = d;
      fijMes();
    }
    if (botones == BOTON_ARR) {
      d += incremento;
      if (d > 31) {
        d = 1;
      }
      delay(200);
    }
    if (botones == BOTON_ABJ) {
      d -= incremento;
      if (d < 1) {
        d = 31;
      }
      delay(200);
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
      delay(200);
      rtc.setMonth(me);
      mes = me;
      muestraTiempo();
    }
    if (botones == BOTON_IZQ) {
      delay(200);
      rtc.setMonth(me);
      mes = me;
      fijDia();
    }
    if (botones == BOTON_DER) {
      delay(200);
      rtc.setMonth(me);
      mes = me;
      fijAno();
    }
    if (botones == BOTON_ARR) {
      me += incremento;
      if (me > 12 ) {
        me = 1;
      }
      delay(200);
    }
    if (botones == BOTON_ABJ) {
      me -= incremento;
      if (me < 1) {
        me = 12;
      }
      delay(200);
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
      delay(200);
      rtc.setYear(a);
      ano = a;
      muestraTiempo();
    }
    if (botones == BOTON_IZQ) {
      delay(200);
      rtc.setYear(a);
      ano = a;
      fijMes();
    }
    if (botones == BOTON_DER) {
      delay(200);
      rtc.setYear(a);
      ano = a;
      muestraTiempo();
    }
    if (botones == BOTON_ARR) {
      a += incremento;
      if (a > 99) {
        a = 0;
      }
      delay(200);
    }
    if (botones == BOTON_ABJ) {
      a -= incremento;
      if (a < 0) {
        a = 99;
      }
      delay(200);
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

void programar() {
  uint8_t botones = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  /*         123567890123456*/
  lcd.print("Cuantas");
  lcd.setCursor(0,1);
  lcd.print("etapas?");
  while(true)
  {
     botones = leerBotones();
     uint8_t incremento = 1;
     if (botones == BOTON_SELEC) {
       parametros();
     }
     if (botones == BOTON_IZQ) {
       muestraTiempo();
     }
     if (botones == BOTON_DER) {
       parametros();
     }
     if (botones == BOTON_ARR) {
       etapas += incremento;
       if (etapas > 10)
       {
         etapas = 1;
       }
     }
     if (botones == BOTON_ABJ) {
       etapas -= incremento;
       if (etapas < 1)
       {
         etapas = 10;
       }
     }
     lcd.setCursor(10,1);
     lcd.print(etapas);
  }
}

void parametros() {
  lcd.clear();
  lcd.home();
  lcd.print("Etapa ");
  for (uint8_t i = 1; i <= etapas; i++) {
    /*         123567890123456*/
    lcd.setCursor(6, 0); lcd.print(i);
    lcd.setCursor(0, 1);
    uint8_t valHum, valTem;
    uint8_t valLlu, valLuz;
    int valDur;
    valHum = defHumedad();
    valTem = defTemperatura();
    valLuz = defLuz();
    valLlu = defRocio();
    valDur = defDuracion();
    condiciones condicionesX(valHum, valTem, valLlu, valLuz, valDur);
    pasos.push_back(condicionesX);
  }
  Serial.print(pasos[0].usaT());
  Serial.print(pasos[1].usaT()); //Solo para verificar que los datos sean correctos
  uint8_t botones = 0;
  for (size_t i = 1; i < etapas; i++) {
    lcd.clear();
    lcd.home();
    lcd.print("Etapa: "); lcd.print(i);
    pasos[i].muestra();
    while (botones != BOTON_DER) {
      botones = leerBotones();
      if (botones == BOTON_IZQ) {
        i--;
        if (i <= 0) {
          i = 1;
        }
        break;
      }
    };
  }
  lcd.clear();
  lcd.home(); lcd.print("Iniciar");
  lcd.setCursor(0, 1); lcd.print("Presiona Selec");
  botones = 0;
  while (botones == BOTON_SELEC ) {
    opEstados = CORRER;
  }
}


uint8_t defHumedad() {
  lcd.setCursor(0, 1);
           //123456789123456
  lcd.print("HR       %     ");
  uint8_t botones = 0;
  uint8_t valHum = 50;
  while (true) {
   botones = leerBotones();
   if (botones == BOTON_SELEC)
   {
     return valHum;
   }
   if (botones == BOTON_IZQ) {
     programar();
   }
   if (botones == BOTON_ARR) {
     valHum = increm(valHum, 99, 10, 1);
   }
   if (botones == BOTON_ABJ) {
     valHum = decrem(valHum, 10, 99, 1);
   }
   if (botones == BOTON_DER) {
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
     return valTem;
   }
   if (botones == BOTON_IZQ) {
     defHumedad();
     return valTem;
   }
   if (botones == BOTON_DER) {
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

/* Función para fijar la presencia/ausencia de luz en el ciclo*/
uint8_t defLuz() {
  lcd.setCursor(0, 1);
  lcd.print("Con luz?        ");
  uint8_t botones = 0;
  uint8_t valLuz = 1;
  while (true) {
    botones = leerBotones();
    if (botones == BOTON_IZQ) {
      defTemperatura();
      return valLuz;
    }
    if (botones == BOTON_DER) {
      return valLuz;
    }
    if (botones == BOTON_ARR) {
      lcd.setCursor(10, 1);
      lcd.print("Si");
      valLuz = 1;
    }
    if (botones == BOTON_ABJ) {
      lcd.setCursor(10, 1);
      lcd.print("No");
      valLuz = 0;
    }
    if (botones == BOTON_SELEC) {
      return valLuz;
    }
  };
}

uint8_t defRocio() {
  lcd.setCursor(0, 1);
  lcd.print("Con lluvia?     ");
  uint8_t valLlu = 1;
  uint8_t botones = 0;
  while (true) {
    botones = leerBotones();
    if (botones == BOTON_IZQ) {
      defLuz();
      return valLlu;
    }
    if (botones == BOTON_DER) {
      return valLlu;
    }
    if (botones == BOTON_ARR) {
      lcd.setCursor(13, 1);
      lcd.print("Si");
      valLlu = 1;
    }
    if (botones == BOTON_ABJ) {
      lcd.setCursor(13, 1);
      lcd.print("No");
      valLlu = 0;
    }
    if (botones == BOTON_SELEC) {
      return valLlu;
    }
  }
}

int defDuracion() {
  lcd.setCursor(0, 1);
  lcd.print("Duracion     min");
  int valDur = 60;
  uint8_t botones = 0;
  while (true) {
   botones = leerBotones();
   if (botones == BOTON_SELEC) {
     return valDur;
   }
   if (botones == BOTON_IZQ) {
     defRocio();
     return valDur;
   }
   if (botones == BOTON_DER) {
    return valDur;
   }
   if (botones == BOTON_ARR) {
     valDur = increm(valDur, 720, 1, 1);
   }
   if (botones == BOTON_ABJ) {
     valDur = decrem(valDur, 1, 720, 1);
   }
   lcd.setCursor(10, 1);
   dosDigitos(valDur);
  };
}


void correr() {

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
/* función de control
void correr() {
  digitalWrite(rel1, LOW);
  digitalWrite(rel2, LOW);
  for(size_t i = 0; i < etapas + 1; i++) {
    unsigned long tiempoPrevio = 0;
    if(pasos[i].luz == 1){
      digitalWrite(rel1, HIGH);
    }
    if(pasos[i].lluvia == 1){
      digitalWrite(rel2, HIGH);
    }
    unsigned long tiempoActual = millis();
    if(tiempoActual - tiempoPrevio >= (pasos[i].duracion)* 1000 ){
      tiempoPrevio = tiempoActual;
    } else {
    }
  }
}
--*/
/*--------------------------------------------------------------------------------------
loop()
Arduino main loop
-------------------------------

void programar() {
int pasos = 1;
pasos();
detParametros();
}

int pasos() {
lcd.clear();
lcd.print("Numero de pasos:");
uint8_t botones = 0;
while(true)
{
 botones = leerBotones();
 uint8_t incremento = 1;
 if (botones & BOTON_SEL) {
   incremento *= 10;
 }
 if (botones & BOTON_IZQ) {
   opEstados = PASO;
 }
 if (botones & BOTON_DER) {
   opEstados = PARAMETROS;
 }
 if (botones & BOTON_ARR) {
   pasos += incremento;
   delay(200);
 }
 if (botones & BOTON_ABJ) {
   pasos -= incremento;
   delay(200);
 }
 lcd.setCursor(0,1);
 lcd.print(pasos);
}
return pasos;
}

void detParametros() {
for (size_t i = 1; i =< pasos; i++) {
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print('Parametros')
 lcd.setCursor(0,1);
 lcd.print('Ciclo');
 lcd.print(i);



byte idenBoton()
 lluvia();
}
}
*/

byte leerBotones() {
  uint8_t botones;
  if (millis() > tiempoAnt + tiempoRef ) {
    voltAnt = botonVolt;
    botonVolt = analogRead(BOTON_PIN_ADC);
  }
  if (botonVolt <= voltAnt + HISTERESIS && botonVolt >= voltAnt - HISTERESIS) {
    cambio = false;
    botones = botonEdo;
  } else {
    cambio = true;
    botonEdo = botones;
  }
  if( botonVolt <= ( DER_ADC + HISTERESIS ) && botonVolt >= (DER_ADC - HISTERESIS))
  {
    botones = BOTON_DER;
  }
  else if(   botonVolt >= ( ARR_ADC - HISTERESIS )
          && botonVolt <= ( ARR_ADC + HISTERESIS ) )
  {
    botones = BOTON_ARR;
  }
  else if(   botonVolt >= ( ABJ_ADC - HISTERESIS )
          && botonVolt <= ( ABJ_ADC + HISTERESIS ) )
  {
    botones = BOTON_ABJ;
  }
  else if(   botonVolt >= ( IZQ_ADC - HISTERESIS )
          && botonVolt <= ( IZQ_ADC + HISTERESIS ) )
  {
    botones = BOTON_IZQ;
  }
  else if(   botonVolt >= ( SELEC_ADC - HISTERESIS )
          && botonVolt <= ( SELEC_ADC + HISTERESIS ) )
  {
    botones = BOTON_SELEC;
  }
  if (cambio) {
    return( botones );
  } else return ESPERA;
  tiempoAnt = millis();
};
