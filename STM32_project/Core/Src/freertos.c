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
float voltaje;
float porcentaje_bateria;
float consumo;
uint16_t capacidad_restante = 0;
float temperatura;
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
uint16_t BQ34Z100_GetDeviceType(void);


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
  osThreadDef(batterygau_task, StartGaulge_task, osPriorityIdle, 0, 512);
  batterygau_taskHandle = osThreadCreate(osThread(batterygau_task), NULL);

  /* definition and creation of LCD_task */
  osThreadDef(LCD_task, StartLCD_task, osPriorityIdle, 0, 512);
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
uint16_t BQ34Z100_GetDeviceType(void) {
    uint8_t data[2];
    data[0] = 0x01; 
    data[1] = 0x00;

    // 1. Comprobar si el dispositivo responde a su dirección
    HAL_StatusTypeDef ready = HAL_I2C_IsDeviceReady(&hi2c1, 0xAA, 3, 100);
    if (ready != HAL_OK) {
        printf("I2C DEBUG: El BQ34Z100 no hace ACK en la direccion 0xAA (HAL Status: %d)\r\n", ready);
        return 0; // Si no responde, salir directamente
    }

    // 2. Enviar el comando para pedir el Device Type
    HAL_StatusTypeDef w_status = HAL_I2C_Mem_Write(&hi2c1, 0xAA, 0x00, 1, data, 2, 100);
    if (w_status != HAL_OK) {
        printf("I2C DEBUG: Fallo al escribir el comando (HAL Status: %d)\r\n", w_status);
        return 0;
    }
    
    osDelay(10);
    
    // 3. Leer la respuesta
    HAL_StatusTypeDef r_status = HAL_I2C_Mem_Read(&hi2c1, 0xAA, 0x00, 1, data, 2, 100);
    if (r_status == HAL_OK) {
        return (data[1] << 8) | data[0];
    } else {
        printf("I2C DEBUG: Fallo al leer la respuesta (HAL Status: %d)\r\n", r_status);
        return 0;
    }
}

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
    
    uint8_t pack_config[2];
    BQ34Z100_ReadFlashBlock(64, 0, pack_config, 2);
    pack_config[0] |= (1 << 3);
    BQ34Z100_WriteFlashBlock(64, 0, pack_config, 2);

    uint8_t cells = 4;
    BQ34Z100_WriteFlashBlock(64, 7, &cells, 1);
    
    uint8_t capacity[2] = { (7500 >> 8) & 0xFF, 7500 & 0xFF }; 
    BQ34Z100_WriteFlashBlock(48, 11, capacity, 2);

    uint16_t energy = 9600;
    uint8_t energy_arr[2] = { (energy >> 8) & 0xFF, energy & 0xFF };
    BQ34Z100_WriteFlashBlock(48, 13, energy_arr, 2);
    
    uint8_t energy_scale = 10;
    BQ34Z100_WriteFlashBlock(48, 30, &energy_scale, 1);

    uint16_t volt_divider = 13500; 
    uint8_t vd_data[2] = { (volt_divider >> 8) & 0xFF, volt_divider & 0xFF };
    BQ34Z100_WriteFlashBlock(104, 14, vd_data, 2);

    uint8_t cc_gain[4] = { 0x7D, 0xA2, 0xBF, 0xAA };
    BQ34Z100_WriteFlashBlock(104, 0, cc_gain, 4);

    uint8_t cc_delta[4] = { 0x91, 0xB8, 0xD1, 0xD4 };
    BQ34Z100_WriteFlashBlock(104, 4, cc_delta, 4);

    BQ34Z100_Seal();
}
/* USER CODE END Application_BQ_Config */

void Reparar_BQ34Z100(void) {
    printf("\r\n--- INICIANDO REPARACION DE FLASH ---\r\n");
    // 1. Desbloquear el chip para poder escribir
    BQ34Z100_Unseal();
    osDelay(100);

    // 2. Valores seguros de fabrica (Asumen un Shunt de 1 mili-Ohm)
    uint8_t cc_gain_default[4] = { 0x82, 0x98, 0x96, 0xA2 };
    BQ34Z100_WriteFlashBlock(104, 0, cc_gain_default, 4);
    
    uint8_t cc_delta_default[4] = { 0x82, 0xA8, 0x5D, 0xE3 };
    BQ34Z100_WriteFlashBlock(104, 4, cc_delta_default, 4);

    // NUEVO: Borrar cualquier "Tara" (Offset) corrupta que este sumando 30 Amperios
    uint8_t zero_offset[2] = { 0x00, 0x00 };
    BQ34Z100_WriteFlashBlock(104, 8, zero_offset, 2);  // Board Offset
    BQ34Z100_WriteFlashBlock(104, 12, zero_offset, 2); // CC Offset

    // 3. Divisor de voltaje para bateria de 12.8V (12800 mV = 0x3200 HEX)
    uint8_t v_div[2] = { 0x32, 0x00 };
    BQ34Z100_WriteFlashBlock(104, 14, v_div, 2);

    // 4. Volver a bloquear
    BQ34Z100_Seal();
    printf("--- REPARACION COMPLETADA ---\r\n\r\n");
}

void StartGaulge_task(void const * argument)
{
  /* USER CODE BEGIN StartGaulge_task */
  uint8_t buffer[2];
  uint16_t val_u16;
  int16_t val_i16;
  const uint16_t BQ34Z100_ADDR = 0xAA;

  uint16_t dev_type = BQ34Z100_GetDeviceType();
  printf("Verificando I2C... DEVICE_TYPE: 0x%04X\r\n", dev_type);
  if (dev_type != 0x0100) {
      printf("ERROR de I2C: No se detecta BQ34Z100-G1.\r\n");
  } else {
      printf("BQ34Z100-G1 Detectado correctamente.\r\n");
  }

  // EJECUTAR REPARACION (Solo la dejaremos descomentada para probar 1 vez)
  // Reparar_BQ34Z100();

  /* Infinite loop */
  for(;;)
  {
    uint8_t buf_v[2] = {0};
    uint8_t buf_c[2] = {0};
    uint8_t buf_s[2] = {0};
    uint8_t buf_t[2] = {0};

    // 0x08 Voltaje (mV)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x08, I2C_MEMADD_SIZE_8BIT, buf_v, 2, 1000) == HAL_OK) {
        val_u16 = (buf_v[1] << 8) | buf_v[0];
        voltaje = val_u16 / 1000.0f; 
    }

    // 0x0A Corriente (mA)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x0A, I2C_MEMADD_SIZE_8BIT, buf_c, 2, 1000) == HAL_OK) {
        val_i16 = (int16_t)((buf_c[1] << 8) | buf_c[0]);
        consumo = val_i16 / 1000.0f; 
    }

    // 0x02 State of Charge (%)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x02, I2C_MEMADD_SIZE_8BIT, buf_s, 2, 1000) == HAL_OK) {
        val_u16 = (buf_s[1] << 8) | buf_s[0];
        porcentaje_bateria = (float)val_u16;
    }

    // 0x0C Temperature (0.1°K)
    if (HAL_I2C_Mem_Read(&hi2c1, BQ34Z100_ADDR, 0x0C, I2C_MEMADD_SIZE_8BIT, buf_t, 2, 1000) == HAL_OK) {
        val_u16 = (buf_t[1] << 8) | buf_t[0];
        temperatura = (val_u16 - 2731) / 10.0f; 
    }

    // IMPRESION DE DEBUG PARA VER LOS BYTES CRUDOS QUE LLEGAN
    printf("\r\nRAW BYTES -> V:[%02X %02X] I:[%02X %02X] SOC:[%02X %02X] T:[%02X %02X]\r\n", 
           buf_v[0], buf_v[1], buf_c[0], buf_c[1], buf_s[0], buf_s[1], buf_t[0], buf_t[1]);

    int v_int = (int)voltaje, v_dec = (int)((voltaje - v_int) * 100); if(v_dec < 0) v_dec = -v_dec;
    int c_int = (int)consumo, c_dec = (int)((consumo - c_int) * 100); if(c_dec < 0) c_dec = -c_dec;
    int b_int = (int)porcentaje_bateria, b_dec = (int)((porcentaje_bateria - b_int) * 10); if(b_dec < 0) b_dec = -b_dec;
    int t_int = (int)temperatura, t_dec = (int)((temperatura - t_int) * 10); if(t_dec < 0) t_dec = -t_dec;

    char c_sign[2] = {0};
    if (consumo < 0 && c_int == 0) c_sign[0] = '-';
    char t_sign[2] = {0};
    if (temperatura < 0 && t_int == 0) t_sign[0] = '-';

    printf("BQ34Z100 -> V: %d.%02dV | I: %s%d.%02dA | SOC: %d.%01d%% | Temp: %s%d.%01d C\r\n", 
           v_int, v_dec, c_sign, c_int, c_dec, b_int, b_dec, t_sign, t_int, t_dec);

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
    int v_int = (int)voltaje;
    int v_dec = (int)((voltaje - v_int) * 100);
    if (v_dec < 0) v_dec = -v_dec;
    sprintf(str_buffer, "Voltaje: %d.%02d V  ", v_int, v_dec);
    ST7789_WriteString(15, 70, str_buffer, Font_11x18, WHITE, BLACK);
    
    int c_int = (int)consumo;
    int c_dec = (int)((consumo - c_int) * 100);
    if (c_dec < 0) c_dec = -c_dec;
    char c_sign[2] = {0};
    if (consumo < 0 && c_int == 0) c_sign[0] = '-';
    sprintf(str_buffer, "Consumo: %s%d.%02d A  ", c_sign, c_int, c_dec);
    ST7789_WriteString(15, 120, str_buffer, Font_11x18, RED, BLACK);
    
    int b_int = (int)porcentaje_bateria;
    int b_dec = (int)((porcentaje_bateria - b_int) * 100);
    if (b_dec < 0) b_dec = -b_dec;
    sprintf(str_buffer, "Bateria: %d.%02d %% ", b_int, b_dec);
    ST7789_WriteString(15, 170, str_buffer, Font_11x18, GREEN, BLACK);

    osDelay(500);
  }
  /* USER CODE END StartLCD_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
