# Sistema de control y monitoreo para cámara de intemperismo acelerado

Este es un sistema desarrollado en Arduino para la tarjeta [Adafruit Feather M0 Datalogger](https://www.adafruit.com/product/2796) que controla una cuatro relevadores para controlar las condiciones dentro de una cámara de intemperismo acelerado y al mismo tiempo registra la humedad relativa, temperatura, luz y UV dentro de la cámara. Las condiciones se generan por medio de cuatro actuadores independientes que solo se relacionan por los relevadores y son un humidificador ultrasonico en el relevador 1, una serie de lamparas de IR en el relevador 2, una bomba para rociar agua en el relevador 3, y lamparas LED de luz UV en el relevador 4.

Los datos son almacenados en la tarjeta SD cada 15 minutos para ciclos de 8 horas de duración.
## Estado del sistema
El sistema esta en uso en el Laboratorio CODICE de la CNCPC-INAH y es operativo con los siguientes problemas:
+ Desconfiguración aleatoria provocada aparentemente por PEM generados cuando se activa alguno de los actuadores
+ Desprogramación aleatoria que solo se resuelve al conectar una fuente de poder USB

## Pendientes
+ Guardado de programaciones en la tarjeta SD
+ Generación de ciclos de actividad preprogramados
+ Incorporación de scripts opcionales para correr pruebas de funcionamiento
