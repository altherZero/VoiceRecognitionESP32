# Adaptacion Academica de VoiceRecognitionV3 para ESP32

## Referencia al repositorio original

Este trabajo se basa en la libreria original **VoiceRecognitionV3** desarrollada por Elechouse, disponible en:

https://github.com/elechouse/VoiceRecognitionV3

La libreria original esta diseñada para entornos Arduino tradicionales y utiliza `SoftwareSerial` para comunicacion serie con el modulo de reconocimiento de voz V3 :contentReference[oaicite:1]{index=1}.

---

## Resumen

Este repositorio contiene una adaptacion academica no oficial de la libreria **VoiceRecognitionV3 de Elechouse**, modificada para permitir su uso en plataformas basadas en **ESP32**.

La adaptacion responde a limitaciones tecnicas en ESP32 relacionadas con el uso de `SoftwareSerial`. En esta version se emplea `HardwareSerial`, aprovechando los puertos UART por hardware disponibles en el ESP32.

Este trabajo tiene fines academicos, educativos y de investigacion y no representa una version oficial ni certificada por Elechouse.

---

## Objetivo

El objetivo de esta adaptacion es permitir la integracion del modulo de reconocimiento de voz V3 con microcontroladores ESP32 sin dependencia de `SoftwareSerial`.

Los objetivos especificos incluyen:

- Eliminar dependencias de `SoftwareSerial`
- Utilizar UART por hardware en ESP32
- Mantener la logica funcional original de la libreria Elechouse
- Facilitar pruebas y desarrollo en entornos de investigacion

---

## Alcance

- Version compatible exclusivamente con **ESP32**
- No compatible con placas basadas en AVR (Arduino UNO, Nano, Mega, etc.)
- No constituye un producto oficial ni soportado por Elechouse
- Dedicado a uso academico y experimental

---

## Descripcion de las modificaciones

Las modificaciones principales respecto al repositorio original son:

- Eliminacion de todo uso de `SoftwareSerial`
- Sustitucion por `HardwareSerial` para comunicacion UART
- Ajustes en la inicializacion de la comunicacion serial
- Conservacion de la estructura funcional de comandos y clases
- Adaptacion de ejemplos para uso con ESP32

---

## Metodologia

El proceso de adaptacion incluyo:

1. Analisis del codigo fuente original disponible en el repositorio de Elechouse.
2. Identificacion de las dependencias a `SoftwareSerial`.
3. Implementacion de equivalentes utilizando `HardwareSerial` del ESP32.
4. Modificaciones de ejemplos y pruebas de comunicacion UART.
5. Pruebas basicas de reconocimiento y entrenamiento de comandos en hardware ESP32.

---

## Limitaciones

- El codigo modificado puede contener errores no detectados en pruebas iniciales.
- La comunicacion serial depende de la configuracion de pines UART seleccionada por el usuario.
- No se garantiza el mismo nivel de estabilidad que en entornos oficiales.
- Se recomienda realizar pruebas exhaustivas antes de su uso en aplicaciones criticas.

---

## Consideraciones de responsabilidad

Este material se proporciona con fines estrictamente academicos y de investigacion.

Los autores del presente repositorio no asumen responsabilidad por:

- Fallos de software
- Daños en hardware
- Perdida de datos
- Resultados no deseados durante el uso

La responsabilidad del uso recae exclusivamente en el usuario final.

---

## Licencia

Esta adaptacion se distribuye bajo una licencia de tipo **MIT-style**, similar a la utilizada en el repositorio original, y establece que:

El software se proporciona "tal cual", sin garantias expresas o implicitas, incluidas pero no limitadas a garantias de comercializacion, idoneidad para un proposito especifico o no infraccion.

En ningun caso los autores o colaboradores seran responsables por reclamaciones, daños u otras responsabilidades derivadas del uso del software.

---

## Creditos

- Elechouse, VoiceRecognitionV3 Library — repositorio original de referencia para la funcionalidad basica del modulo de voz. :contentReference[oaicite:2]{index=2}
- Documentacion oficial de ESP32
- Materiales y recursos de investigacion relacionados con sistemas embebidos y comunicacion serial

---

## Nota final

Esta adaptacion puede utilizarse como referencia en proyectos academicos, tesis o trabajos de investigacion, siempre que se cite adecuadamente tanto al repositorio original de Elechouse como a esta adaptacion.

