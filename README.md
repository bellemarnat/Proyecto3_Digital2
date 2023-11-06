
# Proyecto #3 – I2C y NeoPixel

Este proyecto tiene como objetivo que los estudiantes experimenten con los microcontroladores TIVA C y ESP32 para desarrollar una aplicación que utilice la comunicación I2C y el led RGB NeoPixel.

## Contenido

- [Descripción del Proyecto](#descripción-del-proyecto)
- [Circuitos Utilizados](#circuitos-utilizados)
- [Implementación del Proyecto](#implementación-del-proyecto)
  - [Parte 1 ESP32](#parte-1-esp32)
  - [Parte 2 Comunicación UART](#parte-2-comunicación-uart)
  - [Parte 3 Despliegue de datos](#parte-3-despliegue-de-datos)
  - [Parte 4 Almacenamiento de la SD](#parte-4-almacenamiento-de-la-sd)
  - [Parte 5 Indicador Visual](#parte-5-indicador-visual)

## Descripción del Proyecto

El proyecto busca que los estudiantes experimenten con los microcontroladores TIVA C y ESP32 para desarrollar una aplicación que utilice la comunicación I2C y el led RGB NeoPixel. Las tareas principales del proyecto incluyen:

- Implementar un data logger de un sensor utilizando el almacenamiento SD.
- Crear una interfaz gráfica para mostrar los resultados de las mediciones en una pantalla.
- Conectar un sensor al microcontrolador ESP32 y comunicarse con él mediante el protocolo I2C.
- Enviar la información del sensor tanto al microcontrolador TIVA C como a la computadora a través de la comunicación UART.
- Incorporar dos botones en el microcontrolador TIVA C para seleccionar la tarea deseada.
- Utilizar un indicador visual con un led RGB NeoPixel.

## Circuitos Utilizados

Este proyecto hace uso de varios componentes electrónicos clave:

- **Microcontrolador ESP32:** Se utiliza para la comunicación y adquisición de datos del sensor.
- **Sensor I2C:** Conectado al ESP32 para la adquisición de datos.
- **Microcontrolador TIVA C:** Controla la selección de tareas y la comunicación UART.
- **Pantalla TFT:** Utilizada para mostrar los resultados de las mediciones.
- **Tarjeta SD:** Utilizada para almacenar los datos obtenidos.
- **Led RGB NeoPixel:** Sirve como indicador visual del estado del dispositivo.

## Implementación del Proyecto

### Parte 1 ESP32

Diseñe e implemente una rutina en la que el ESP32 se comunique con un sensor de su preferencia utilizando el protocolo de comunicación I2C.

### Parte 2 Comunicación UART

Diseñe e implemente una rutina en el microcontrolador TIVA C que permita utilizar uno de los botones para preguntar el valor del sensor al microcontrolador ESP32. Además, cuando se le pida el resultado del sensor, este deberá ser enviado tanto al TIVA C como a la computadora utilizando la comunicación UART.

### Parte 3 Despliegue de Datos

Diseñe e implemente una rutina para que en la pantalla TFT se pueda desplegar el resultado de la medición del sensor. Se anima a ser creativo en la implementación de la interfaz gráfica.

### Parte 4 Almacenamiento de la SD

Diseñe e implemente la comunicación con la tarjeta SD para que se pueda almacenar el dato obtenido utilizando el segundo botón.

### Parte 5 Indicador Visual

Implemente un indicador visual utilizando un led RGB NeoPixel para indicar con colores el estado del dispositivo (encendido, midiendo, guardando datos, etc).

Asegúrese de incluir las librerías necesarias en sus códigos para garantizar el correcto funcionamiento del proyecto.

Este proyecto es una oportunidad para que los estudiantes adquieran experiencia en el uso de microcontroladores, comunicación I2C y UART, diseño de interfaces gráficas y gestión de almacenamiento en tarjetas SD. ¡Buena suerte con tu proyecto!
