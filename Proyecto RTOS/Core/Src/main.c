/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

#define DWT_CTRL	(*(volatile uint32_t*)0xE0001000)

TaskHandle_t print_LCD;
TaskHandle_t uart_deco;
TaskHandle_t uart_out;
TaskHandle_t all_menus_handler;

QueueHandle_t q_print_lcd;
QueueHandle_t q_print_uart;
QueueHandle_t q_data_uart;

volatile uint8_t uart_data;
uint8_t cont_timer = 0;
uint32_t valor_adc;
uint8_t send_agm_uart = 0;
uint8_t send_adc_uart = 0;
uint8_t valor_pwm_ext;

FlagStatus state_agm = RESET;
FlagStatus state_adc_pwm = RESET;

SemaphoreHandle_t sema_lcd;
SemaphoreHandle_t sema_i2c_rx_end;
SemaphoreHandle_t sema_adc;

state_t display_state = Menu_1;

control_pwm_t ctrl_pwm = Interno_ADC;

A_M_ejes_t Acel_ejes, Magne_ejes;
G_ejes_t Gyro_ejes;
AGM_t AGM_ejes;
AGM_t *pAGM_ejes = &AGM_ejes;

TimerHandle_t AGM_ADC_timer;
TimerHandle_t Buttons_timer[4];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

extern void SEGGER_UART_init(U32 baud);

void AGM_ADC_timer_callback(TimerHandle_t xTimer);
void Buttons_timer_callback(TimerHandle_t xTimer);

void Init_tasks(void);
void Init_semaphores(void);
void Init_queues(void);
void Init_timers(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	AGM_ejes.Acel_ejes = &Acel_ejes;
	AGM_ejes.Gyro_ejes = &Gyro_ejes;
	AGM_ejes.Magne_ejes = &Magne_ejes;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  	lcd_init();

	// Enable the CYCCNT counter
	DWT_CTRL |= (1 << 0);

	SEGGER_UART_init(500000);
	SEGGER_SYSVIEW_Conf();

	Init_tasks();

	Init_queues();

	Init_semaphores();

	Init_timers();

	HAL_UART_Receive_IT(&huart2, (uint8_t*)&uart_data, 1);

	// Start the FreeRTOS scheduler
	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 194;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 420;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 10000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 8000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Pin_Motor_1_Pin|Pin_Motor_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA4 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : STLK_RX_Pin STLK_TX_Pin */
  GPIO_InitStruct.Pin = STLK_RX_Pin|STLK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Pin_Motor_1_Pin Pin_Motor_2_Pin */
  GPIO_InitStruct.Pin = Pin_Motor_1_Pin|Pin_Motor_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void Init_tasks(void)
{
	BaseType_t status;

	status = xTaskCreate(vprint_LCD, "Print LCD", 500, NULL, 2, &print_LCD);
	configASSERT(status == pdPASS);

	status = xTaskCreate(vuart_deco, "UART Deco", 500, NULL, 5, &uart_deco);
	configASSERT(status == pdPASS);

	status = xTaskCreate(vuart_out, "UART out", 500, NULL, 4, &uart_out);
	configASSERT(status == pdPASS);

	status = xTaskCreate(vall_menus_handler, "All_menus_handler", 500, NULL, 2, &all_menus_handler);
	configASSERT(status == pdPASS);
}

void Init_semaphores(void)
{
	sema_lcd = xSemaphoreCreateBinary();
	configASSERT(sema_lcd != NULL);

	sema_i2c_rx_end = xSemaphoreCreateBinary();
	configASSERT(sema_i2c_rx_end != NULL);

	sema_adc = xSemaphoreCreateBinary();
	configASSERT(sema_adc != NULL);

	xSemaphoreGive(sema_lcd);
}

void Init_queues(void)
{
	q_print_lcd = xQueueCreate(1, sizeof(state_t*));
	configASSERT(q_print_lcd != NULL);

	q_print_uart = xQueueCreate(10, sizeof(char*));
	configASSERT(q_print_uart != NULL);

	q_data_uart = xQueueCreate(12, sizeof(char));
	configASSERT(q_data_uart != NULL);
}

void Init_timers(void)
{
	AGM_ADC_timer = xTimerCreate("AGM_ADC_timer", pdMS_TO_TICKS(250), pdTRUE, NULL, AGM_ADC_timer_callback);

	for(int i = 0; i < 4; i++)
		Buttons_timer[i] = xTimerCreate("buttons_timer", pdMS_TO_TICKS(250), pdFALSE, (void*)(i + 1), Buttons_timer_callback);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Indicamos al SEGGER que hemos ingresado a una interrupcion
	traceISR_ENTER();

	if(huart == &huart2)
	{
		uint8_t dummy;

		if(xQueueIsQueueFullFromISR(q_data_uart) == pdFALSE)
		{
			/*Queue is not full */

			/* Enqueue data byte */
			xQueueSendFromISR(q_data_uart, (void*)&uart_data, NULL);
		}else{
			/*Queue is full */

			if(uart_data == '\n')
			{
				/*uart_data = '\n' */

				/* make sure that last data byte of the queue is '\n' */
				xQueueReceiveFromISR(q_data_uart, (void*)&dummy, NULL);
				xQueueSendFromISR(q_data_uart, (void*)&uart_data, NULL);
				//xQueueOverwriteFromISR(q_data, &uart_data, NULL);
			}
		}

		/* send notification to command handling task if uart_data = '\n' */
		if(uart_data == '\n')
		{
			xTaskNotifyFromISR(uart_deco, 0, eNoAction, &xHigherPriorityTaskWoken);
		}

		/*  Enable UART data byte reception again in IT mode */
		HAL_UART_Receive_IT(&huart2, (uint8_t*)&uart_data, 1);
	}

	// Solicitamos el cambio de contexto si pxHigherPriorityTaskWoken = pdTrue
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	// Indicamos al SEGGER que estamos saliendo de la interrupcion
	traceISR_EXIT();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Indicamos al SEGGER que hemos ingresado a una interrupcion
	traceISR_ENTER();

	// Averiguamos que boton fue presionado
	if(GPIO_Pin == GPIO_PIN_0)
	{
		// Se presionó BUTTON_DOWN

		// Deshabilitamos la interrupción a BUTTON_DOWN por 250ms para evitar el rebote
		HAL_NVIC_DisableIRQ(EXTI0_IRQn);
		xTimerStartFromISR(Buttons_timer[0], &xHigherPriorityTaskWoken);

		// Averiguamos en que pantalla estamos y notificamos a la tarea correspondiente
		if((display_state == Menu_1) || (display_state == Configuration_1) || (display_state == Data_1) ||
			(display_state == Data_2) || (display_state == Data_3) || (display_state == Data_4) || (display_state == AGM_1)
			|| (display_state == ADC_1))
			xTaskNotifyFromISR(all_menus_handler, BUTTON_DOWN, eSetValueWithOverwrite, NULL);
	}else if(GPIO_Pin == GPIO_PIN_4)
	{
		// Se presionó BUTTON_UP

		// Deshabilitamos la interrupción a BUTTON_UP por 250ms para evitar el rebote
		HAL_NVIC_DisableIRQ(EXTI4_IRQn);
		xTimerStartFromISR(Buttons_timer[1], &xHigherPriorityTaskWoken);

		// Averiguamos en que pantalla estamos y notificamos a la tarea correspondiente
		if((display_state == Menu_2) || (display_state == Configuration_2) || (display_state == Data_2) ||
			(display_state == Data_3) || (display_state == Data_4) || (display_state == Data_5) || (display_state == AGM_2)
			|| (display_state == ADC_2))
			xTaskNotifyFromISR(all_menus_handler, BUTTON_UP, eSetValueWithOverwrite, NULL);
	}else if(GPIO_Pin == GPIO_PIN_13)
	{
		// Se presionó ENTER

		// Deshabilitamos la interrupción a ENTER por 250ms para evitar el rebote
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
		xTimerStartFromISR(Buttons_timer[2], &xHigherPriorityTaskWoken);

		// Averiguamos en que pantalla estamos y notificamos a la tarea correspondiente
		if((display_state == Menu_1) || (display_state == Menu_2) || (display_state == Configuration_1) || (display_state == Configuration_2)
			|| (display_state == AGM_1) || (display_state == AGM_2) || (display_state == ADC_1)	|| (display_state == ADC_2))
			xTaskNotifyFromISR(all_menus_handler, ENTER, eSetValueWithOverwrite, NULL);
	}else if(GPIO_Pin == GPIO_PIN_6)
	{
		// Se presionó BACK

		// Deshabilitamos la interrupción a BACK por 250ms para evitar el rebote
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		xTimerStartFromISR(Buttons_timer[3], &xHigherPriorityTaskWoken);

		// Averiguamos en que pantalla estamos y notificamos a la tarea correspondiente
		if((display_state == Configuration_1) || (display_state == Configuration_2) || (display_state == Data_1) ||
			(display_state == Data_2) || (display_state == Data_3) || (display_state == Data_4) || (display_state == Data_5)
			|| (display_state == AGM_1) || (display_state == AGM_2) || (display_state == ADC_1)	|| (display_state == ADC_2))
			xTaskNotifyFromISR(all_menus_handler, BACK, eSetValueWithOverwrite, NULL);
	}

	// Solicitamos el cambio de contexto si pxHigherPriorityTaskWoken = pdTrue
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	// Indicamos al SEGGER que estamos saliendo de la interrupcion
	traceISR_EXIT();
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{

}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Indicamos al SEGGER que hemos ingresado a una interrupcion
	traceISR_ENTER();

	// Entregamos el semaforo para indicar que la recepción I2C ha finalizado
	xSemaphoreGiveFromISR(sema_i2c_rx_end, &xHigherPriorityTaskWoken);

	// Solicitamos el cambio de contexto si pxHigherPriorityTaskWoken = pdTrue
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	// Indicamos al SEGGER que estamos saliendo de la interrupcion
	traceISR_EXIT();
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	lcd_print_string("Error I2C");

	// Infinite loop
	while(1);
}

// Función timer de lectura de datos que se ejecuta cada 250ms
void AGM_ADC_timer_callback(TimerHandle_t xTimer)
{
	char *msg_data = "Sensores";
	char *msg_data_2 = "Desactivados";
	char data_lcd[17];

	float valor_volt;

	cont_timer++;

	static cadena_lcd_t msg_lcd;
	static cadena_lcd_t *msg = &msg_lcd;

	uint32_t id;

	// Obtenemos el id del timer activado
	id = (uint32_t)pvTimerGetTimerID(xTimer);
	// Indicamos al SEGGER que hemos ingresado a un timer
	SEGGER_SYSVIEW_RecordEnterTimer(id);

	// Leemos los sensores
	valor_volt = read_sensors();

	// Preguntamos si hay solicitudes de envío de datos
	if((send_agm_uart > 0) || (send_adc_uart > 0))
	{
		// Envíamos los datos por el puerto UART y disminuimos las solicitudes
		send_data_uart(valor_volt);
		send_agm_uart--;
		send_adc_uart--;
	}

	// Preguntamos si los sensores están activados o desactivados
	if((state_adc_pwm == RESET) && (state_agm == RESET) && (cont_timer == 1))
	{
		// Ambos sensores están desactivados
		// Imprimimos en LCD
		send_to_lcd(1, 1, LCD_WRITE, msg_data, msg);
		send_to_lcd(2, 1, LCD_WRITE, msg_data_2, msg);
	}else if((state_adc_pwm == SET) && (state_agm == RESET))
	{
		// ADC: activado, AGM: desactivado
		// Imprimimos en LCD
		sprintf(data_lcd, "Volt: %.2f", valor_volt);
		send_to_lcd(1, 1, LCD_WRITE, data_lcd, msg);
	}else
	{
		if(((cont_timer % 2) == 1) && (state_agm == SET))
		{
			// AGM: activado
			// Averiguamos en que pantalla estamos e imprimimos lo correspondiente
			if(display_state == Data_1)
			{
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				sprintf(data_lcd, "Ax=%5.1f y=%5.1f", AGM_ejes.Acel_ejes->x, AGM_ejes.Acel_ejes->y);
				send_to_lcd(1, 1, LCD_WRITE, data_lcd, msg);
				sprintf(data_lcd, "z=%5.1f Gx=%5.1f", AGM_ejes.Acel_ejes->z, AGM_ejes.Gyro_ejes->x);
				send_to_lcd(2, 1, LCD_WRITE, data_lcd, msg);
			}else if(display_state == Data_2)
			{
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				sprintf(data_lcd, "z=%5.1f Gx=%5.1f", AGM_ejes.Acel_ejes->z, AGM_ejes.Gyro_ejes->x);
				send_to_lcd(1, 1, LCD_WRITE, data_lcd, msg);
				sprintf(data_lcd, "y=%5.1f z=%5.1f", AGM_ejes.Gyro_ejes->y, AGM_ejes.Gyro_ejes->z);
				send_to_lcd(2, 1, LCD_WRITE, data_lcd, msg);
			}else if(display_state == Data_3)
			{
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				sprintf(data_lcd, "y=%5.1f z=%5.1f", AGM_ejes.Gyro_ejes->y, AGM_ejes.Gyro_ejes->z);
				send_to_lcd(1, 1, LCD_WRITE, data_lcd, msg);
				sprintf(data_lcd, "Mx=%5.1f y=%5.1f", AGM_ejes.Magne_ejes->x, AGM_ejes.Magne_ejes->y);
				send_to_lcd(2, 1, LCD_WRITE, data_lcd, msg);
			}else if(display_state == Data_4)
			{
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				sprintf(data_lcd, "Mx=%5.1f y=%5.1f", AGM_ejes.Magne_ejes->x, AGM_ejes.Magne_ejes->y);
				send_to_lcd(1, 1, LCD_WRITE, data_lcd, msg);
				sprintf(data_lcd, "z=%5.1f", AGM_ejes.Magne_ejes->z);
				send_to_lcd(2, 1, LCD_WRITE, data_lcd, msg);
			}else if(display_state == Data_5)
			{
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				sprintf(data_lcd, "z=%5.1f", AGM_ejes.Magne_ejes->z);
				send_to_lcd(1, 1, LCD_WRITE, data_lcd, msg);
			}
		}

		if((state_adc_pwm == SET) && (display_state == Data_5))
		{
			// ADC: activado
			// Convertimos el dato adc a voltaje e imprimimos
			//valor_volt = conv_adc_volt();
			sprintf(data_lcd, "Volt: %.2f", valor_volt);
			send_to_lcd(2, 1, LCD_WRITE, data_lcd, msg);
		}
	}

	// Reiniciamos el contador cuando es 200
	if(cont_timer == 200)
		cont_timer = 0;

	// Indicamos al SEGGER que estamos saliendo del timer
	SEGGER_SYSVIEW_RecordExitTimer();
}

// Función timer que se ejecuta 250ms luego de haber presionado un botón, está función
// vuelve a activar la interrupción del botón correspondiente
void Buttons_timer_callback(TimerHandle_t xTimer)
{
	// Obtenemos el id del timer activado
	uint32_t id;
	id = (uint32_t)pvTimerGetTimerID(xTimer);
	// Indicamos al SEGGER que hemos ingresado a un timer
	SEGGER_SYSVIEW_RecordEnterTimer(id);

	// Limpiamos los flags correspondientes antes de volver a activar la interrupción
	switch(id)
	{
		case 1:
			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
			HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
			HAL_NVIC_EnableIRQ(EXTI0_IRQn);
			break;
		case 2:
			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
			HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
			HAL_NVIC_EnableIRQ(EXTI4_IRQn);
			break;
		case 3:
			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
			HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
			HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
			break;
		case 4:
			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
			HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	}

	// Indicamos al SEGGER que estamos saliendo del timer
	SEGGER_SYSVIEW_RecordExitTimer();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Indicamos al SEGGER que hemos ingresado a una interrupcion
	traceISR_ENTER();

	// Obtenemos el valor adc obtenido
	valor_adc = HAL_ADC_GetValue(&hadc1);
	// Actualizamos el valor del PWM
	TIM4->CCR1 = (10000 * valor_adc) / 4095;

	// Entregamos el semaforo, indicando que la conversión adc está completa
	xSemaphoreGiveFromISR(sema_adc, &xHigherPriorityTaskWoken);

	// Solicitamos el cambio de contexto si pxHigherPriorityTaskWoken = pdTrue
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	// Indicamos al SEGGER que estamos saliendo de la interrupcion
	traceISR_EXIT();
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	while(1);
}

void vApplicationIdleHook( void )
{
	// Ingresamos al modo ahorro de energía, mientras estamos en la idle task.
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
