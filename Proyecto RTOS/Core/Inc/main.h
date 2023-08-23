/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd.h"
#include "timers.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

typedef struct
{
	char msg[17];
	uint8_t fila;
	uint8_t columna;
	uint8_t function;
}cadena_lcd_t;

typedef struct
{
	float x;
	float y;
	float z;
}A_M_ejes_t;

typedef struct
{
	float x;
	float y;
	float z;
	float temp;
}G_ejes_t;

typedef struct
{
	A_M_ejes_t *Acel_ejes;
	G_ejes_t *Gyro_ejes;
	A_M_ejes_t *Magne_ejes;
}AGM_t;

typedef struct
{
	uint8_t payload[12];		// Store data bytes of the command
	uint32_t len;				// command length
}command_t;

typedef enum
{
	Menu_1 = 0,
	Menu_2,
	Data_1,
	Data_2,
	Data_3,
	Data_4,
	Data_5,
	Configuration_1,
	Configuration_2,
	AGM_1,
	AGM_2,
	ADC_1,
	ADC_2
}state_t;

typedef enum
{
	Externo = 0,
	Interno_ADC
}control_pwm_t;

extern TaskHandle_t print_LCD;
extern TaskHandle_t uart_deco;
extern TaskHandle_t uart_out;/*
extern TaskHandle_t main_handler;
extern TaskHandle_t conf_handler;
extern TaskHandle_t data_handler;
extern TaskHandle_t conf_agm_handler;
extern TaskHandle_t conf_adc_handler;*/
extern TaskHandle_t all_menus_handler;

extern QueueHandle_t q_print_lcd;
extern QueueHandle_t q_print_uart;
extern QueueHandle_t q_data_uart;

extern SemaphoreHandle_t sema_lcd;
extern SemaphoreHandle_t sema_i2c_rx_end;
extern SemaphoreHandle_t sema_adc;

extern state_t display_state;

extern control_pwm_t ctrl_pwm;

extern FlagStatus state_agm;
extern FlagStatus state_adc_pwm;

extern A_M_ejes_t Acel_ejes, Magne_ejes;
extern G_ejes_t Gyro_ejes;
extern AGM_t AGM_ejes;
extern AGM_t *pAGM_ejes;

extern TimerHandle_t AGM_ADC_timer;
extern TimerHandle_t Buttons_timer[4];

extern uint8_t cont_timer;
extern uint32_t valor_adc;
extern uint8_t send_agm_uart;
extern uint8_t send_adc_uart;
extern uint8_t valor_pwm_ext;

extern I2C_HandleTypeDef hi2c1;

extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart2;

extern ADC_HandleTypeDef hadc1;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define LCD_CLEAR		1
#define LCD_WRITE		2
#define LCD_POSITION	3

#define BUTTON_UP	1
#define BUTTON_DOWN	2
#define ENTER		3
#define BACK		4

#define MY_ADDR			0x61
#define ACELER_ADDR		0xA6		//0x53 sin desplazamiento
#define GYRO_ADDR		0xD0		//0x68
#define MAG_ADDR		0x3C		//0x1E

#define ACEL_POWER_CTL	0x2D
#define ACEL_DATA		0x32

#define GYRO_SMPLRT_DIV 	0x15
#define GYRO_DLPF_FS 		0x16
#define GYRO_INT_CFG 		0x17
#define GYRO_PWR_MGM 		0x3E
#define GYRO_DATA			0x1B
#define GYRO_WHO_AM_I		0x00

#define MAG_CF_REG_A		0x00	// 0x18
#define MAG_CF_REG_B		0x01	//
#define MAG_MOD_REG			0x02	// 0x00
#define MAG_DATA			0x03

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void vprint_LCD(void* parameters);
void vuart_deco(void* parameters);
void vuart_out(void* parameters);
void vall_menus_handler(void* parameters);

void Acel_Init(void);
void Gyro_Init(void);
void Mag_Init(void);
void Acel_read_data(A_M_ejes_t *A_eje);
void Gyro_read_data(G_ejes_t *G_eje);
void Mag_read_data(A_M_ejes_t *M_eje);

void send_to_lcd(uint8_t fila, uint8_t columna, uint8_t function, char *data, cadena_lcd_t *msg);

float conv_adc_volt(void);
float read_sensors(void);
void send_data_uart(float valor_volt);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define USER_Btn_EXTI_IRQn EXTI15_10_IRQn
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define Pin_Motor_1_Pin GPIO_PIN_8
#define Pin_Motor_1_GPIO_Port GPIOC
#define Pin_Motor_2_Pin GPIO_PIN_9
#define Pin_Motor_2_GPIO_Port GPIOC
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
