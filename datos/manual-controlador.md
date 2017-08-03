Sensor de humedad y temperatura

Rango de 0 a 100% de HR con precisión de 2-5%

Rango de -40 a 80 °C con precisión de ±0.5 °C

Velocidad de muestreo de 0.5 Hz (cada 2 s)

Sensor de luz

Sensor de dos diodos con detección espectral de 400 a 1100 nm.

Rango dinámico de sensibilidad de 600M:1 respuesta espectral similar a
la del ojo humano.

Sensor de UV

Rango de detección espectral de 240 a 370 nm.

Manual de operación.

Conecte antes los equipos que serán controlados en el siguiente orden:

\* Contacto 1, HUMIDIFICADOR o el dispositivo que modifique la humedad
ambiental.

\* Contacto 2, IR u otro dispositivo que modifique la temperatura, como
resistencias o mantas electricas.

\* Contacto 3, las LUCES que la prueba requiera.

\* Contacto 4, la BOMBA o el dispositivo que controle el rocío, goteo u
otra forma de presencia de agua liquida en la prueba.

Conecte el equipo a un contacto aterrizado de 120 V.

Encienda el equipo presionando el interruptor 1 en la posición I.

Si lo requiere, conecte la batería accionando el interruptor en la
posición I. La batería almacena energía de respaldo para el procesador
en el caso de una interrupción del suministro, si bien el equipo seguirá
funcionando, los aparatos conectados a este NO, ya que requieren mucha
más energía que la que la batería interna puede proporcionar. Si le
interesa un funcionamiento sin interrupciones deberá usar un no-break
como fuente de energía

Al accionar el botón de encendido deberá aparecer un mensaje de
bienvenida en pantalla.

Verifique que la tarjeta SD esta correctamente conectada. En la pantalla
deberá aparecer la leyenda “Tarjeta lista”.

El equipo estará en modo de espera hasta que se pulse el botón derecho.

Al pulsar el botón derecho se inicia la configuración del equipo
iniciando por el reloj de tiempo real.

Configuración del reloj

Cuando no esta conectada la batería, el reloj debe ser configurado cada
vez que se use el equipo.

1.  Primero configure la hora, modificando su valor con los botones de
    arriba y abajo. Cuando este en el valor deseado, presione el botón
    derecho para configurar los minutos.
2.  Siga el mismo procedimiento para configurar los segundos. Al
    finalizar, presione el botón “Select” para configurar la fecha.
3.  Para la fecha, siga el mismo procedimiento, comenzando con el día,
    el mes y el año. Al finalizar, presione “Select” y verifique en la
    pantalla que los valores ingresados sean los correctos.
4.  En caso de necesitar corregir los datos, presione el botón
    izquierdo.
5.  Cuando esté conforme con los valores de hora y fecha, presione el
    botón de “Select” para iniciar la configuración de las condiciones
    de operación.

Configuración de las condiciones de operación.

Las condiciones que deberá controlar el equipo se configuran mediante
ciclos de operación. Cada ciclo comprende una serie de etapas, las
cuales tienen condiciones y duración particular. Cada etapa puede
incluir la presencia de humedad, calor, luz o rocío y una duración
independiente del resto de las etapas. Para configurar los valores siga
el siguiente procedimiento:

1.  Después de configurar el reloj, la pantalla le preguntará el numero
    de etapas por ciclo que desea. Defina tantas etapas como variaciones
    planeé con los botones de arriba y abajo. Presione “Select” o
    derecha para continuar con la configuración de las condiciones de
    cada etapa.
2.  Defina primero si la etapa incluirá la presencia de humedad. Si su
    prueba no incluye humedad continué con la siguiente condición.
    Presione “Select” o derecha para continuar.
3.  Fije si la etapa contará con elementos de calentamiento o no.
    Presione derecha o “Select” para continuar.
4.  Establezca si la etapa tendrá activadas las luces especiales o no
    con los botones de arriba y abajo. Presione “Select” o derecha para
    continuar.
5.  Establezca si la etapa tendrá rocío, u alguna otra actividad con
    agua liquida o no con los botones de arriba y abajo. Presione
    “Select” o derecha para continuar.
6.  Defina la duración, en horas, de la etapa con los botones de arriba
    y abajo. Presione “Select” para continuar. Si desea corregir alguno
    de los valores anteriores, presione el botón izquierdo el cual le
    permitirá regresar a la configuración inmediata anterior.
7.  Repita el proceso desde el paso 2, para cada etapa.
8.  Al finalizar con la última etapa, el equipo le solicitará la
    configuración del numero de ciclos que se repetirán las etapas. Cada
    ciclo es una sucesión de las etapas definidas previamente. Use los
    botones de arriba y abajo para definir cuantas veces se repetirá la
    sucesión de etapas. Si desea que el equipo repita indefinidamente
    las etapas programadas, establezca el valor de ciclos en 0 (cero).
    Para continuar presione el botón “Select”. NOTA: Al fijar el valor
    el ceros, el equipo entra en un bucle infinito, que sólo se podrá
    detener presionando el botón izquierdo al iniciar la operación.
9.  En la pantalla se desplegaran los valores definidos para cada etapa,
    una por una. Para revisar los datos de la etapa siguiente, presione
    el botón derecho.
10. Al terminar de mostrar los datos de todas las etapas, el equipo le
    pedirá que presione “Select” para confirmar e iniciar el primer
    ciclo.
11. Al iniciar el ciclo de control, la pantalla mostrará en secuencia
    los valores de humedad, temperatura, luminancia, irradiación UV,
    hora, fecha y el tiempo que resta para que la etapa en curso
    concluya.
12. Independiente de los datos que se muestran en pantalla, cada 15
    minutos se registraran en la tarjeta SD los valores de los cuatro
    sensores, y el estado de los cuatro relevadores que señalan que
    dispositivo estaba encendido en ese momento. La información quedara
    contenida en un archivo de texto con formato de valores separados
    por comas (CSV Comma Separated Values) con el nombre REGIST01.TXT,
    el número puede variar dependiendo de el número de registros previos
    que se hayan realizado. Como referencia, el orden en que aparecen
    los datos en el archivo es el siguiente:

Fecha, Hora, Humedad, Temperatura, Luz, Uv, Humidificador, IR, Luces,
Aspersión

01/01/17, 09:12:28, 85.31, 45.01, 430, 55, 1, 1, 1, 1

El formato de los datos para la humedad relativa es porcentaje con dos
decimales. Para la temperatura es en grados Celsius con dos decimales.
La luminancia se expresa en luxes, mientras que la luz UV se mide en
milivatios sobre centímetro cuadrado (mW/cm²). Es importante recordar
que el sensor de UV solo tienen una banda de sensibilidad de 240 a 370
nm por lo que, si los iluminantes UV usados no emiten en dicha banda, el
registro de su potencia puede ser menor al esperado.

1.  Si se desea detener el ciclo de control se deberá presionar el botón
    izquierdo y seguir las instrucciones en pantalla. Esto detendrá la
    programación y reiniciara los parámetros del sistema.
2.  Al finalizar el numero de ciclos programados el aparato se detendrá
    y volverá al punto de programación para que se introduzcan nuevos
    parámetros de control.

### Posibles problemas

Debido a que el procesador del controlador se encuentra confinado junto
con los circuitos de corriente alterna, es posible que estos generen
interferencia electromagnética afectando el funcionamiento del
controlador. Se ha buscado el evitar estos efectos al máximo, pero las
variaciones de voltaje o las descargas podrían provocar que el equipo se
haga inestable, quedando congelada la pantalla o mostrando caracteres
ilegibles o con los actuadores apagados. En dicho caso, accione el
apagador de la batería para desconectarla y desconecte la clavija
principal. Esto reiniciara todo el sistema, sin embargo, los datos de
registro de condiciones ambientales quedaran guardados en la tarjeta SD.

El equipo cuenta con una batería interna de 500 mAh que en caso de una
falla del suministro eléctrico, respaldaría el funcionamiento de los
sensores por al menos 2 horas. Debido a que los actuadores funcionan con
corriente alterna, estos dejarían de funcionar en cualquier caso.
