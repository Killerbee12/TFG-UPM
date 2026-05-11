/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7789.h"
#include <stdio.h>
#include "i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
float voltaje = 0.00f;
float porcentaje_bateria = 0.00f;
float consumo = 0.00f;
uint16_t capacidad_restante = 0;
float temperatura = 0.00f;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId batterygau_taskHandle;
osThreadId LCD_taskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartGaulge_task(void const * argument);
void StartLCD_task(void const * argument);

/* Funciones para configurar el BQ34Z100-G1 */
void BQ34Z100_Unseal(void);
void BQ34Z100_Seal(void);
void BQ34Z100_WriteFlashBlock(uint8_t subclass, uint8_t offset, uint8_t* data, uint8_t length);
void BQ34Z100_ReadFlashBlock(uint8_t subclass, uint8_t offset, uint8_t* out_data, uint8_t length);
void Configurar_BQ34Z100(void);


extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of batterygau_task */
  osThreadDef(batterygau_task, StartGaulge_task, osPriorityIdle, 0, 128);
  batterygau_taskHandle = osThreadCreate(osThread(batterygau_task), NULL);

  /* definition and creation of LCD_task */
  osThreadDef(LCD_task, StartLCD_task, osPriorityIdle, 0, 128);
  LCD_taskHandle = osThreadCreate(osThread(LCD_task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application_BQ_Config */
void BQ34Z100_Unseal(void) {
    uint8_t data[2];
    data[0] = 0x14; data[1] = 0x04;
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x00, 1, data, 2, 100);
    osDelay(10);
    data[0] = 0x72; data[1] = 0x36;
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x00, 1, data, 2, 100);
    osDelay(10);
}

void BQ34Z100_Seal(void) {
    uint8_t data[2] = {0x20, 0x00};
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x00, 1, data, 2, 100);
}

void BQ34Z100_WriteFlashBlock(uint8_t subclass, uint8_t offset, uint8_t* new_data, uint8_t length) {
    uint8_t block_data[32];
    uint8_t block_cmd = 0x00;
    uint8_t new_checksum = 0;
    
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x61, 1, &block_cmd, 1, 100);
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x3E, 1, &subclass, 1, 100);
    uint8_t block_offset = offset / 32;
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x3F, 1, &block_offset, 1, 100);
    
    HAL_I2C_Mem_Read(&hi2c1, 0xAA, 0x40, 1, block_data, 32, 100);
    
    uint8_t local_offset = offset % 32;
    for(int i=0; i<length; i++) {
        block_data[local_offset + i] = new_data[i];
    }
    
    for(int i = 0; i < 32; i++) {
        new_checksum += block_data[i];
    }
    new_checksum = 255 - new_checksum;
    
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x40, 1, block_data, 32, 100);
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x60, 1, &new_checksum, 1, 100);
    osDelay(100);
}

void BQ34Z100_ReadFlashBlock(uint8_t subclass, uint8_t offset, uint8_t* out_data, uint8_t length) {
    uint8_t block_data[32];
    uint8_t block_cmd = 0x00;
    
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x61, 1, &block_cmd, 1, 100);
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x3E, 1, &subclass, 1, 100);
    uint8_t block_offset = offset / 32;
    HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x3F, 1, &block_offset, 1, 100);
    
    HAL_I2C_Mem_Read(&hi2c1, 0xAA, 0x40, 1, block_data, 32, 100);
    uint8_t local_offset = offset % 32;
    for(int i=0; i<length; i++) {
        out_data[i] = block_data[local_offset + i];
    }
}

void Configurar_BQ34Z100(void) {
    BQ34Z100_Unseal();
    
    // 1. Pack Configuration (Activar bit VOLTSEL)
    uint8_t pack_config[2];
    BQ34Z100_ReadFlashBlock(64, 0, pack_config, 2);
    pack_config[0] |= (1 << 3); // Poner el bit VOLTSEL a 1
    BQ34Z100_WriteFlashBlock(64, 0, pack_config, 2);

    // 2. Number of Series Cells (Configurar a 4 celdas)
    uint8_t cells = 4;
    BQ34Z100_WriteFlashBlock(64, 7, &cells, 1);
    
    // 3. Design Capacity (7500 mAh) -> SUBCLASS 48, Offset 11
    uint8_t capacity[2] = { (7500 >> 8) & 0xFF, 7500 & 0xFF }; 
    BQ34Z100_WriteFlashBlock(48, 11, capacity, 2);

    // 4. Design Energy (96000 mWh) -> SUBCLASS 48, Offset 13
    // El registro es de 2 bytes (máximo 65535). Como 96000 no cabe, hay que escalar entre 10 
    // y activar el Design Energy Scale a 10.
    uint16_t energy = 9600; // 96000 / 10
    uint8_t energy_arr[2] = { (energy >> 8) & 0xFF, energy & 0xFF };
    BQ34Z100_WriteFlashBlock(48, 13, energy_arr, 2);
    
    // Design Energy Scale -> SUBCLASS 48, Offset 30
    uint8_t energy_scale = 10;
    BQ34Z100_WriteFlashBlock(48, 30, &energy_scale, 1);

    // 5. Voltage Divider (Ej: 13500 mV aprox para calibrar)
    uint16_t volt_divider = 13500; 
    uint8_t vd_data[2] = { (volt_divider >> 8) & 0xFF, volt_divider & 0xFF };
    BQ34Z100_WriteFlashBlock(104, 14, vd_data, 2);

    // 6. CC Gain y CC Delta (Para resistencia Shunt de 60mOhm)
    // Valores precalculados de Xemics Floating Point para 60mOhm:
    // CC Gain = 4.768 / 60 = 0.07946 -> 0x7D, 0xA2, 0xBF, 0xAA
    // CC Delta = 5677445 / 60 = 94624.08 -> 0x91, 0xB8, 0xD1, 0xD4
    uint8_t cc_gain[4] = { 0x7D, 0xA2, 0xBF, 0xAA };
    BQ34Z100_WriteFlashBlock(104, 0, cc_gain, 4);

    uint8_t cc_delta[4] = { 0x91, 0xB8, 0xD1, 0xD4 };
    BQ34Z100_WriteFlashBlock(104, 4, cc_delta, 4);

    // NOTA 7: Chemistry ID (ChemID) de LiFePO4 no se puede cambiar con 2 bytes
    // desde el STM32 fácilmente, se mantendrá en Litio-Ion por defecto.

    BQ34Z100_Seal();
}
/* USER CODE END Application_BQ_Config */

void StartGaulge_task(void const * argument)
{
  /* USER CODE BEGIN StartGaulge_task */
  uint8_t buffer[2];
  uint16_t val_u16;
  int16_t val_i16;
  const uint16_t BQ34Z100_ADDR = 0xAA;

  // Configurar_BQ34Z100();

  /* Infinite loop */
  for(;;)
  {
    // 0x08 Voltaje (mV)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x08, I2C_MEMADD_SIZE_8BIT, buffer, 2, 1000) == HAL_OK) {
        val_u16 = (buffer[1] << 8) | buffer[0];
        voltaje = val_u16 / 1000.0f; // Convertir a Voltios
    }

    // 0x0A Corriente (mA)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x0A, I2C_MEMADD_SIZE_8BIT, buffer, 2, 1000) == HAL_OK) {
        val_i16 = (int16_t)((buffer[1] << 8) | buffer[0]);
        consumo = val_i16 / 1000.0f; // Convertir a Amperios
    }

    // 0x02 State of Charge (%)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x02, I2C_MEMADD_SIZE_8BIT, buffer, 2, 1000) == HAL_OK) {
        val_u16 = (buffer[1] << 8) | buffer[0];
        porcentaje_bateria = (float)val_u16;
    }

    // 0x04 Remaining Capacity (mAh)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x04, I2C_MEMADD_SIZE_8BIT, buffer, 2, 1000) == HAL_OK) {
        val_u16 = (buffer[1] << 8) | buffer[0];
        capacidad_restante = val_u16;
    }

    // 0x06 Temperature (0.1°K)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x06, I2C_MEMADD_SIZE_8BIT, buffer, 2, 1000) == HAL_OK) {
        val_u16 = (buffer[1] << 8) | buffer[0];
        temperatura = (val_u16 - 2731) / 10.0f; // Convertir a Celsius
    }

    // Imprimir por el puerto serie (ITM/SWV o UART según tengas configurado el _write)
    printf("BQ34Z100 -> V: %5.2fV | I: %5.2fA | SOC: %5.1f%% | RemCap: %d mAh | Temp: %5.1f C\r\n", 
           voltaje, consumo, porcentaje_bateria, capacidad_restante, temperatura);

    // Refrescamos cada 2 segundos (2000 ms)
    osDelay(2000);
  }
  /* USER CODE END StartGaulge_task */
}

/* USER CODE BEGIN Header_StartLCD_task */
/**
* @brief Function implementing the LCD_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLCD_task */
void StartLCD_task(void const * argument)
{
  /* USER CODE BEGIN StartLCD_task */
  char str_buffer[32];
  ST7789_Fill_Color(BLACK);
  ST7789_WriteString(43, 15, "ESTADO BATERIA", Font_11x18, YELLOW, BLACK);
  ST7789_DrawLine(10, 40, 230, 40, WHITE);

  /* Infinite loop */
  for(;;)
  { 
    sprintf(str_buffer, "Voltaje: %5.2f V", voltaje);
    ST7789_WriteString(15, 70, str_buffer, Font_11x18, WHITE, BLACK);
    
    sprintf(str_buffer, "Consumo: %5.2f A", consumo);
    ST7789_WriteString(15, 120, str_buffer, Font_11x18, RED, BLACK);
    
    sprintf(str_buffer, "Bateria: %5.2f %%", porcentaje_bateria);
    ST7789_WriteString(15, 170, str_buffer, Font_11x18, GREEN, BLACK);

    osDelay(500); // Pausamos la tarea 500ms para no saturar el bus SPI
  }
  /* USER CODE END StartLCD_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
