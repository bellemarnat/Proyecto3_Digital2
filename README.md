# Proyecto de Monitoreo de Temperatura con ESP32

Este proyecto implementa un sistema de monitoreo de temperatura en tiempo real utilizando un microcontrolador ESP32. El sistema está diseñado para medir la temperatura ambiente y mostrarla de manera visual en un display de 7 segmentos y controlar un servo motor o LEDs indicadores de acuerdo con los valores de temperatura. 

## Contenido

- [Circuitos Utilizados](#circuitos-utilizados)
- [Funcionamiento](#funcionamiento)
- [Configuración](#configuración)
- [Notas](#notas)

## Circuitos Utilizados

El proyecto consta de los siguientes elementos:

- Implementar un data logger de un sensor utilizando el almacenamiento SD.
- Crear una interfaz gráfica para mostrar los resultados de las mediciones en la pantalla.
- Conectar un sensor al microcontrolador ESP32 y comunicarse con él utilizando el protocolo de comunicación I2C.
- Enviar la información del sensor tanto al microcontrolador TIVA C como a la computadora mediante comunicación UART.
- Utilizar dos botones en el microcontrolador TIVA C para seleccionar las tareas a realizar.
- Incluir un indicador visual mediante un led RGB NeoPixel.

## Funcionamiento

El proyecto consiste en un bucle principal donde se leen los valores del sensor de temperatura y se realizan las acciones correspondientes. El sistema verifica si el botón ha sido presionado, aplicando el principio de debouncing para evitar lecturas incorrectas. Una vez que se detecta una pulsación, se activa la función `presionBoton()`.

En la función `presionBoton()`, se adquiere la lectura cruda del sensor LM35 y se convierte en una temperatura en grados Celsius. Esta temperatura se utiliza para controlar el servo motor o los LEDs indicadores, dependiendo del rango en el que se encuentre. Además, se muestra la temperatura en el display de siete segmentos.

## Configuración


## Notas



Este código y proyecto fueron desarrollados por Isabelle Marnat como parte de mi proyecto de Electronica Digital 2. 
