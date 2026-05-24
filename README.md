# 🔭 Sistema de Alimentación Portátil para Montura de Telescopio

Este repositorio contiene todo el desarrollo de hardware, firmware y documentación del **Trabajo Fin de Grado / Máster** realizado en la **Universidad Politécnica de Madrid (UPM)**. El objetivo principal es el diseño y prototipado de un sistema electrónico modular y robusto capaz de gestionar la energía y alimentar de forma portátil la electrónica y los motores de una montura de telescopio.

## 🚀 Características Principales

* **Gestión Inteligente de Batería (BMS):** Monitorización en tiempo real mediante el integrado de potencia **BQ34Z100-G1** (Impedance Track™) sobre una batería LiFePO4 de 12.8V.
* **Regulación de Potencia:** Arquitectura de conversión Buck-Boost eficiente para estabilizar las líneas de tensión frente a los picos de carga de los motores.
* **Controlador Central:** Firmware desarrollado sobre un microcontrolador **STM32F411** en entorno STM32CubeIDE y FreeRTOS para la gestión de tareas.
* **Diseño de PCB Modular:** Placas de circuito impreso diseñadas de forma modular en **KiCad**, incluyendo tiras de pines para testeo en laboratorio y jumpers de selección de niveles lógicos.

## 🛠️ Arquitectura del Sistema

El proyecto se divide en tres bloques físicos independientes para facilitar el prototipado y aislamiento de fallos:
1.  **Bloque de Gestión de Batería (BMS):** Centrado en el circuito del BQ34Z100-G1, sensor de temperatura NTC acoplado a las celdas y filtrado RC para la resistencia Shunt.
2.  **Bloque de Regulación y Potencia:** Convertidores de tensión apantallados con bobinas robustas para mitigar el ruido electromagnético.
3.  **Bloque de Control:** Placa de desarrollo de la STM32F411 encargada de leer la telemetría por bus I2C y procesar los estados.

## 📂 Estructura del Repositorio

* `/Hardware`: Esquemas de conexión, ficheros de diseño de PCB y footprints utilizados en **KiCad**.
* `/Firmware`: Código fuente en C/C++ desarrollado para la STM32 (`Core/Src`, `Core/Inc`, configuración de FreeRTOS).
* `/Documentación`: Hojas de características (datasheets), notas de aplicación de Texas Instruments y esquemas del anteproyecto.

## 🔧 Configuración del Laboratorio (Debug)

Para la depuración del bus I2C y monitorización de registros en tiempo real:
* **Líneas de Pull-Up:** Configurables mediante jumper físico a `3.3V` (Línea del microcontrolador para garantizar inmunidad al ruido y niveles lógicos correctos).
* **Puntos de Test (Pins):** Espadines de 2.54 mm accesibles para osciloscopio o analizador lógico en las señales `SDA`, `SCL`, `REG25` (2.5V de lógica interna) y `BAT`.

---
*Desarrollado por Nizar El Amine - Escuela Técnica Superior de Ingeniería de Sistemas de Telecomunicación (ETSIST - UPM).*
