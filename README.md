# Sistema de Alimentación Portátil para Montura de Telescopio

**Trabajo Fin de Grado** — Grado en Ingeniería Electrónica de Comunicaciones  
ETSIST · Universidad Politécnica de Madrid (UPM)

---

## De qué va este proyecto

Cualquiera que haya hecho astrofotografía o astronomía visual en campo abierto sabe que la energía es un problema. Las monturas motorizadas, las cámaras y los accesorios necesitan alimentación estable durante horas, normalmente lejos de cualquier enchufe. La solución clásica es cargar con una batería de plomo-ácido, que pesa, ocupa y no te dice cuánta carga le queda realmente.

Este TFG propone una alternativa: un **sistema de alimentación portátil basado en baterías LiFePO4** (12.8V, 7.5Ah), más ligeras y seguras que el plomo, con electrónica dedicada para monitorizar el estado de carga real, gestionar la recarga y entregar una tensión de salida estable a la montura.

## Qué hace el sistema

- **Mide la carga real de la batería** usando un fuel gauge (BQ34Z100-G1) con tecnología Impedance Track, que compensa el envejecimiento de las celdas y mide temperatura con un termistor NTC.
- **Gestiona la recarga** con un cargador buck-boost BQ25792, compatible con USB-PD 3.0.
- **Estabiliza la tensión de salida** mediante un convertidor buck-boost TPS551892-Q1, preparado para los picos de corriente que generan los motores de la montura.
- **Coordina todo** desde un microcontrolador STM32F411 programado con FreeRTOS, que lee la telemetría por I2C y gestiona las tareas del sistema de forma concurrente.

## Diseño hardware

El hardware está diseñado en **KiCad** y separado en PCBs independientes para aislar ruidos electromagnéticos y facilitar la depuración:

1. **Battery Gauge (BMS)** — Circuito del BQ34Z100-G1 con divisor de tensión, shunt de corriente y sensor térmico.
2. **Battery Charger** — Circuito del BQ25792 con la electrónica de carga.
3. **Bloque de control** — MCU STM32F411, buses I2C y lectura de telemetría.

## Estructura del repositorio

```
├── Anteproyecto/          Documentos del anteproyecto (borradores, versión firmada, ejemplos y revisiones del tutor)
├── Archivos/              Datasheets de referencia consultados durante el diseño
├── Inventario/            Datasheets organizados por tipo de componente:
│   ├── Bateria/               Pack LiFePO4 12.8V 7.5Ah
│   ├── Battery_Charger/       BQ25792, BQ25690, LTC4020, etc.
│   ├── Battery_Gauge/         BQ34Z100-G1, BQ34110, BQ78350, etc.
│   ├── DC-DC_BuckBoost/       TPS551892-Q1, TPS55285, TPS63070, etc.
│   ├── Microcontrolador/      STM32F411 Discovery
│   ├── Bobina/
│   ├── Conectores/
│   ├── Diodos-zener/
│   ├── Resistencias/
│   └── Transistores/
├── Notas/                 Notas y documentación del proyecto (memoria, apuntes)
├── PCB/                   Diseños de PCB en KiCad
│   ├── Prototipo_2.0/        Prototipo inicial del battery gauge
│   └── Proyecto/              Diseños finales
│       ├── PCB_BatteryCharger/
│       └── PCB_BatteryGaulge/
├── STM32_project/         Firmware en C (STM32CubeIDE + FreeRTOS)
└── README.md
```

## Herramientas utilizadas

| Área | Herramienta |
|---|---|
| Esquemáticos y PCB | KiCad |
| Firmware | STM32CubeIDE (HAL + FreeRTOS) |
| Microcontrolador | STM32F411 Discovery |
| Documentación | Word / PDF |

## Estado del proyecto

Este repositorio recoge todo el proceso de desarrollo del TFG: desde los primeros borradores del anteproyecto hasta los diseños de PCB y el firmware. Es un proyecto en progreso.

---

*Nizar El Azeouzi Amine — ETSIST, Universidad Politécnica de Madrid*
