/*
 * Task_handles.c
 *
 *  Created on: 31 may. 2021
 *      Author: Joel
 */

#include "main.h"

static void process_command(void);
static int extract_command(command_t *cmd);
static void activar_botones(void);
static void desactivar_botones(void);
void cambio_giro_motor(void);
void read_send_datos(void);
void active_pwm_pot(void);
void conf_pwm_ext(command_t *cmd);
void send_data_inv(void);
static void data_to_main_plus(char *msg_menu, char *msg_menu_2, cadena_lcd_t *msg);
void change_display_state(char *msg_menu, char *msg_menu_2, cadena_lcd_t *msg, state_t display);

// Tarea para imprimir en LCD
void vprint_LCD(void* parameters)
{
	cadena_lcd_t *msg;

	while(1)
	{
		// Leemos los datos recibidos en la queue
		xQueueReceive(q_print_lcd, &msg, portMAX_DELAY);

		// Averoguamos que función se está solicitando
		if(msg->function == LCD_CLEAR)
		{
			// Limpiar pantalla
			lcd_display_clear();
		}else if(msg->function == LCD_POSITION)
		{
			// Posicionar el cursor
			lcd_set_cursor(msg->fila, msg->columna);
		}else if(msg->function == LCD_WRITE)
		{
			// Escribir en el LCD
			lcd_set_cursor(msg->fila, msg->columna);
			lcd_print_string(msg->msg);
		}else
		{
			// Función errónea
			lcd_print_string("Error");
		}

		// Entregamos el semaforo, permitiendo un nuevo envío de datos a la queue
		xSemaphoreGive(sema_lcd);
	}
}

// Tarea para decodificar los datos recibidos en el puerto UART
void vuart_deco(void* parameters)
{
	while(1)
	{
		// Espera una notificación
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		// Process the user data(command) stored in input data queue
		process_command();
	}
}

// Tarea para transmitir por el puerto UART
void vuart_out(void* parameters)
{
	char *msg_uart;

	while(1)
	{
		// Leemos los datos recibidos en la queue y los enviamos por el puerto UART
		xQueueReceive(q_print_uart, &msg_uart, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg_uart, strlen((char*)msg_uart), HAL_MAX_DELAY);
	}
}

void vall_menus_handler(void* parameters)
{
	char *msg_welcome = "Bienvenido";
	char *msg_menu = "o Mostrar data";
	char *msg_menu_2 = "  Configuracion";
	char *msg_menu_3 = "  Mostrar data";
	char *msg_menu_4 = "o Configuracion";

	char *msg_conf = "o AGM";
	char *msg_conf_2 = "  ADC - PWM";
	char *msg_conf_3 = "  AGM";
	char *msg_conf_4 = "o ADC - PWM";

	char *msg_conf_agm_adc = "o Activar";
	char *msg_conf_agm_adc_2 = "  Desactivar";
	char *msg_conf_agm_adc_3 = "  Activar";
	char *msg_conf_agm_adc_4 = "o Desactivar";

	char *msg_conf_agm_act = "AGM activado";
	char *msg_conf_agm_des = "AGM desactivado";

	char *msg_conf_adc_act = "ADC activado";
	char *msg_conf_adc_des = "ADC desactivado";

	cadena_lcd_t msg_lcd;
	cadena_lcd_t *msg = &msg_lcd;
	uint32_t cmd_addr;

	Mag_Init();
	Gyro_Init();
	Acel_Init();

	// Imprimimos las pantallas iniciales al encender el micro
	send_to_lcd(1, 1, LCD_WRITE, msg_welcome, msg);

	vTaskDelay(pdMS_TO_TICKS(3000));

	send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
	send_to_lcd(1, 1, LCD_WRITE, msg_menu, msg);
	send_to_lcd(2, 1, LCD_WRITE, msg_menu_2, msg);

	while(1)
	{
		// Esperamos una notificación por presionar un botón
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);

		// Averiguamos en que pantalla estamos
		if(display_state == Menu_1)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_DOWN)
			{
				change_display_state(msg_menu_3, msg_menu_4, msg, Menu_2);
			}else if(cmd_addr == ENTER)
			{
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				// Inicializa el timer AGM_ADC_timer
				xTimerStart(AGM_ADC_timer, portMAX_DELAY);
				display_state = Data_1;
			}
		}else if(display_state == Menu_2)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
			{
				change_display_state(msg_menu, msg_menu_2, msg, Menu_1);
			}else if(cmd_addr == ENTER)
			{
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
			}
		}else if(display_state == Configuration_1)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_DOWN)
			{
				change_display_state(msg_conf_3, msg_conf_4, msg, Configuration_2);
			}else if(cmd_addr == ENTER)
			{
				change_display_state(msg_conf_agm_adc, msg_conf_agm_adc_2, msg, AGM_1);
			}else if(cmd_addr == BACK)
			{
				change_display_state(msg_menu, msg_menu_2, msg, Menu_1);
			}
		}else if(display_state == Configuration_2)
		{
			if(cmd_addr == BUTTON_UP)
			{
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
			}else if(cmd_addr == ENTER)
			{
				change_display_state(msg_conf_agm_adc, msg_conf_agm_adc_2, msg, ADC_1);
			}else if(cmd_addr == BACK)
			{
				change_display_state(msg_menu, msg_menu_2, msg, Menu_1);
			}
		}else if(display_state == Data_1)
		{
			// Preguntamos si los sensores están activados o desactivados
			if((state_adc_pwm == RESET) && (state_agm == RESET))
			{
				// Ambos sensores están desactivados, solo hay una pantalla
				// Si se presiona el botón BACK
				if(cmd_addr == BACK)
				{
					// Retorno a main_handler
					data_to_main_plus(msg_menu, msg_menu_2, msg);
				}
			}else if((state_adc_pwm == SET) && (state_agm == RESET))
			{
				// ADC: activado, AGM: desactivado, solo hay una pantalla
				// Si se presiona el botón BACK
				if(cmd_addr == BACK)
				{
					// Retorno a main_handler
					data_to_main_plus(msg_menu, msg_menu_2, msg);
				}
			}else
			{
				// AGM: activado, hay 5 pantallas
				// Averiguamos que botón fue presionado y actualizamos el estado de la pantalla
				if(cmd_addr == BUTTON_DOWN)
					display_state = Data_2;
				else if(cmd_addr == BACK)
				{
					// Retorno a main_handler
					data_to_main_plus(msg_menu, msg_menu_2, msg);
				}
			}
		}else if(display_state == Data_2)
		{
			// Averiguamos que botón fue presionado y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
				display_state = Data_1;
			else if(cmd_addr == BUTTON_DOWN)
				display_state = Data_3;
			else if(cmd_addr == BACK)
			{
				// Retorno a main_handler
				data_to_main_plus(msg_menu, msg_menu_2, msg);
			}
		}else if(display_state == Data_3)
		{
			// Averiguamos que botón fue presionado y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
				display_state = Data_2;
			else if(cmd_addr == BUTTON_DOWN)
				display_state = Data_4;
			else if(cmd_addr == BACK)
			{
				// Retorno a main_handler
				data_to_main_plus(msg_menu, msg_menu_2, msg);
			}
		}else if(display_state == Data_4)
		{
			// Averiguamos que botón fue presionado y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
				display_state = Data_3;
			else if(cmd_addr == BUTTON_DOWN)
				display_state = Data_5;
			else if(cmd_addr == BACK)
			{
				// Retorno a main_handler
				data_to_main_plus(msg_menu, msg_menu_2, msg);
			}
		}else if(display_state == Data_5)
		{
			// Averiguamos que botón fue presionado y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
				display_state = Data_4;
			else if(cmd_addr == BACK)
			{
				// Retorno a main_handler
				data_to_main_plus(msg_menu, msg_menu_2, msg);
			}
		}else if(display_state == AGM_1)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_DOWN)
			{
				change_display_state(msg_conf_agm_adc_3, msg_conf_agm_adc_4, msg, AGM_2);
			}else if(cmd_addr == ENTER)
			{
				// Desactivamos los botones mientras mostramos el mensaje de activado
				desactivar_botones();
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				send_to_lcd(1, 1, LCD_WRITE, msg_conf_agm_act, msg);
				// Actualizamos el estado AGM a activado
				state_agm = SET;
				// Delay de 2s
				vTaskDelay(pdMS_TO_TICKS(2000));
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
				// Volvemos a activar los botones
				activar_botones();
			}else if(cmd_addr == BACK)
			{
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
			}
		}else if(display_state == AGM_2)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
			{
				change_display_state(msg_conf_agm_adc, msg_conf_agm_adc_2, msg, AGM_1);
			}else if(cmd_addr == ENTER)
			{
				// Desactivamos los botones mientras mostramos el mensaje de desactivado
				desactivar_botones();
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				send_to_lcd(1, 1, LCD_WRITE, msg_conf_agm_des, msg);
				// Actualizamos el estado AGM a desactivado
				state_agm = RESET;
				// Delay de 2s
				vTaskDelay(pdMS_TO_TICKS(2000));
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
				// Volvemos a activar los botones
				activar_botones();
			}else if(cmd_addr == BACK)
			{
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
			}
		}else if(display_state == ADC_1)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_DOWN)
			{
				change_display_state(msg_conf_agm_adc_3, msg_conf_agm_adc_4, msg, ADC_2);
			}else if(cmd_addr == ENTER)
			{
				// Desactivamos los botones mientras mostramos el mensaje de activado
				desactivar_botones();
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				send_to_lcd(1, 1, LCD_WRITE, msg_conf_adc_act, msg);
				// Actualizamos el estado adc
				state_adc_pwm = SET;
				// Activamos el motor y el PWM
				HAL_GPIO_WritePin(Pin_Motor_1_GPIO_Port, Pin_Motor_1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(Pin_Motor_1_GPIO_Port, Pin_Motor_2_Pin, GPIO_PIN_RESET);
				HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
				// Delay de 2s
				vTaskDelay(pdMS_TO_TICKS(2000));
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
				// Volvemos a activar los botones
				activar_botones();
			}else if(cmd_addr == BACK)
			{
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
			}
		}else if(display_state == ADC_2)
		{
			// Averiguamos que botón fue presionado, luego imprimimos la pantalla correspondiente
			// y actualizamos el estado de la pantalla
			if(cmd_addr == BUTTON_UP)
			{
				change_display_state(msg_conf_agm_adc, msg_conf_agm_adc_2, msg, ADC_1);
			}else if(cmd_addr == ENTER)
			{
				// Desactivamos los botones mientras mostramos el mensaje de desactivado
				desactivar_botones();
				send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
				send_to_lcd(1, 1, LCD_WRITE, msg_conf_adc_des, msg);
				// Actualizamos el estado adc
				state_adc_pwm = RESET;
				// Desactivamos el motor y el PWM
				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
				HAL_GPIO_WritePin(Pin_Motor_1_GPIO_Port, Pin_Motor_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(Pin_Motor_1_GPIO_Port, Pin_Motor_2_Pin, GPIO_PIN_RESET);
				// Delay de 2s
				vTaskDelay(pdMS_TO_TICKS(2000));
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
				// Volvemos a activar los botones
				activar_botones();
			}else if(cmd_addr == BACK)
			{
				change_display_state(msg_conf, msg_conf_2, msg, Configuration_1);
			}
		}
	}
}


// Función para enviar datos de una tarea a la tarea print_LCD
void send_to_lcd(uint8_t fila, uint8_t columna, uint8_t function, char *data, cadena_lcd_t *msg)
{
	// Tomamos el semaforo
	xSemaphoreTake(sema_lcd, portMAX_DELAY);

	// Averiguamos que función se solicita y de acuerdo a eso enviamos los datos necesarios por la queue
	if(function == LCD_CLEAR)
	{
		msg->function = LCD_CLEAR;
		xQueueSend(q_print_lcd, &msg, portMAX_DELAY);
	}else if(function == LCD_POSITION)
	{
		msg->fila = fila;
		msg->columna = columna;
		msg->function = LCD_POSITION;
		xQueueSend(q_print_lcd, &msg, portMAX_DELAY);
	}else if(function == LCD_WRITE)
	{
		msg->fila = fila;
		msg->columna = columna;
		msg->function = LCD_WRITE;
		sprintf(msg->msg, "%s", data);
		xQueueSend(q_print_lcd, &msg, portMAX_DELAY);
	}else
	{
		// Si la función es erronea, devolvemos el semaforo
		xSemaphoreGive(sema_lcd);
	}
}

// Función para procesar los comandos recibidos por el puerto UART
static void process_command(void)
{
	command_t cmd;
	char pwm_char[12];

	// Extraemos el comando en la estructura cmd
	extract_command(&cmd);
	// Extraemos los 1ros 8 caracteres en la cadena pwm_char
	snprintf(pwm_char, 8, "%s", cmd.payload);
	pwm_char[8] = '\0';

	// Averiguamos si coincide con uno de los comandos permitidos y procesamos de acuerdo a ello
	if(! strcmp((char*)cmd.payload, "CG"))
		cambio_giro_motor();
	else if(! strcmp((char*)cmd.payload, "LEER DATOS"))
		read_send_datos();
	else if(! strcmp((char*)cmd.payload, "PWM POT"))
		active_pwm_pot();
	else if(! strcmp((char*)pwm_char, "PWM EXT"))
		conf_pwm_ext(&cmd);
	else
		send_data_inv();
}

// Función que extrae los comandos recibidos por el puerto UART, y los guarda en la estructura cmd
static int extract_command(command_t *cmd)
{
	uint8_t item;
	BaseType_t status;

	// Preguntamos si hay un mensaje esperando en la queue, si no hay retornamos
	status = uxQueueMessagesWaiting(q_data_uart);
	if(!status)	return -1;

	// Si hay un mensaje, lo almacenamos en cmd.payload y su longitud en cmd.len
	uint8_t i = 0;
	do
	{
		status = xQueueReceive(q_data_uart, &item, 0);
		if(status == pdTRUE)	cmd->payload[i++] = item;
	}while(item != '\n');

	cmd->payload[i - 1] = '\0';
	cmd->len = i - 1;			// save length of the command excluding null char

	return 0;
}

// Función para inicializar el acelerómetro
void Acel_Init(void)
{
	uint8_t Data_write[2], Data_receive;
	Data_write[0] = ACEL_POWER_CTL;

	// Leemos el bit de medicion
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, ACELER_ADDR, Data_write, 1, I2C_FIRST_FRAME) != HAL_OK);
	while( HAL_I2C_Master_Seq_Receive_IT(&hi2c1, ACELER_ADDR, &Data_receive, 1, I2C_LAST_FRAME) != HAL_OK);

	// Activamos la medicion, measure_bit = 1
	Data_write[1] = (Data_receive | 0x08);
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, ACELER_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);
}

// Función para inicializar el giroscopio
void Gyro_Init(void)
{
	uint8_t Data_write[2];

	// Write in PWR_MGM address
	Data_write[0] = GYRO_PWR_MGM;
	Data_write[1] = 0x00;
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, GYRO_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);

	// Write in SMPLRT_DIV address
	Data_write[0] = GYRO_SMPLRT_DIV;
	Data_write[1] = 0x07;
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, GYRO_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);

	// Write in DLPF_FS address
	Data_write[0] = GYRO_DLPF_FS;
	Data_write[1] = 0x1E;
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, GYRO_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);

	// Write in INT_CFG address
	Data_write[0] = GYRO_INT_CFG;
	Data_write[1] = 0x00;
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, GYRO_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);
}

// Función para inicializar el magnetómetro
void Mag_Init(void)
{
	uint8_t Data_write[2];

	// Write in CF_REG_A address
	Data_write[0] = MAG_CF_REG_A;
	Data_write[1] = 0x18;
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, MAG_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);

	// Write in MOD_REG address
	Data_write[0] = MAG_MOD_REG;
	Data_write[1] = 0x00;
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, MAG_ADDR, Data_write, 2, I2C_FIRST_AND_LAST_FRAME) != HAL_OK);
}

// Función para leer los datos del acelerómetro
void Acel_read_data(A_M_ejes_t *A_eje)
{
	uint8_t Reg_address = ACEL_DATA;
	uint8_t Data_axes[6];
	uint16_t dummy;

	// Send the first command to the slave
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, ACELER_ADDR, &Reg_address, 1, I2C_FIRST_FRAME) != HAL_OK);

	// Read the data form the slave
	while( HAL_I2C_Master_Seq_Receive_IT(&hi2c1, ACELER_ADDR, Data_axes, 6, I2C_LAST_FRAME) != HAL_OK);

	// Wait till rx completes
	xSemaphoreTake(sema_i2c_rx_end, portMAX_DELAY);

	// Convierte los datos a su valor real y los almacena en la estructura A_eje
	dummy = (((int16_t)Data_axes[0]) << 8) | ((int16_t)Data_axes[1]);
	A_eje->x = (((float)dummy) / 256);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[2]) << 8) | ((int16_t)Data_axes[3]);
	A_eje->y = (((float)dummy) / 256);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[4]) << 8) | ((int16_t)Data_axes[5]);
	A_eje->z = (((float)dummy) / 256);
}

// Función para leer los datos del giroscopio
void Gyro_read_data(G_ejes_t *G_eje)
{
	uint8_t Reg_address = GYRO_DATA;
	uint8_t Data_axes[8];
	uint16_t dummy;

	// Send the first command to the slave
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, GYRO_ADDR, &Reg_address, 1, I2C_FIRST_FRAME) != HAL_OK);

	// Read the data form the slave
	while( HAL_I2C_Master_Seq_Receive_IT(&hi2c1, GYRO_ADDR, Data_axes, 8, I2C_LAST_FRAME) != HAL_OK);

	// Wait till rx completes
	xSemaphoreTake(sema_i2c_rx_end, portMAX_DELAY);

	// Convierte los datos a su valor real y los almacena en la estructura G_eje
	dummy = (((int16_t)Data_axes[0]) << 8) | ((int16_t)Data_axes[1]);
	G_eje->temp = ((((float)dummy) + 13200) / 280);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[2]) << 8) | ((int16_t)Data_axes[3]);
	G_eje->x = (((float)dummy) / 14.375);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[4]) << 8) | ((int16_t)Data_axes[5]);
	G_eje->y = (((float)dummy) / 14.375);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[6]) << 8) | ((int16_t)Data_axes[7]);
	G_eje->z = (((float)dummy) / 14.375);
}

// Función para leer los datos del magnetómetro
void Mag_read_data(A_M_ejes_t *M_eje)
{
	uint8_t Reg_address = MAG_DATA;
	uint8_t Data_axes[6];
	uint16_t dummy;

	// Send the first command to the slave
	while( HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, MAG_ADDR, &Reg_address, 1, I2C_FIRST_FRAME) != HAL_OK);

	// Read the data form the slave
	while( HAL_I2C_Master_Seq_Receive_IT(&hi2c1, MAG_ADDR, Data_axes, 6, I2C_LAST_FRAME) != HAL_OK);

	// Wait till rx completes
	xSemaphoreTake(sema_i2c_rx_end, portMAX_DELAY);

	// Convierte los datos a su valor real y los almacena en la estructura M_eje
	dummy = (((int16_t)Data_axes[0]) << 8) | ((int16_t)Data_axes[1]);
	M_eje->x = (((float)dummy) / 1090);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[2]) << 8) | ((int16_t)Data_axes[3]);
	M_eje->y = (((float)dummy) / 1090);
	dummy = (uint16_t)dummy;

	dummy = (((int16_t)Data_axes[4]) << 8) | ((int16_t)Data_axes[5]);
	M_eje->z = (((float)dummy) / 1090);
}

// Función que convierte el valor adc a voltios
float conv_adc_volt(void)
{
	float valor_volt;

	valor_volt = (3.3 * ((float)valor_adc)) / 4095;
	valor_adc = (uint32_t)valor_adc;

	return valor_volt;
}

// Función que convierte el valor pwm recibido por el uart (0 a 100) a voltios
float conv_pwm_ext_volt(void)
{
	float valor_volt;

	valor_volt = (3.3 * ((float)valor_pwm_ext)) / 100;
	//valor_pwm_ext = (uint8_t)valor_pwm_ext;

	return valor_volt;
}

// Función para retornar de data_handler a main_handler
static void data_to_main_plus(char *msg_menu, char *msg_menu_2, cadena_lcd_t *msg)
{
	// Detenemos la función timer AGM_ADC_timer
	xTimerStop(AGM_ADC_timer, portMAX_DELAY);
	// reiniciamos el contador y actualizamos el estado de la pantalla
	cont_timer = 0;
	change_display_state(msg_menu, msg_menu_2, msg, Menu_1);
}

// Función para desactivar las interrupciones de todos los botones
static void desactivar_botones(void)
{
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	HAL_NVIC_DisableIRQ(EXTI4_IRQn);
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
}

// Función para activar las interrupciones de los botones, limpiando los flags antes de activarlos
static void activar_botones(void)
{
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
	HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
	HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// Función para cambiar el giro del motor
void cambio_giro_motor(void)
{
	char *msg_motor = "Giro cambiado\n";
	char *msg_motor_dis = "Motor desactivado\n";

	// Averiguamos el estado del PWM
	if(state_adc_pwm == SET)
	{
		// PWM: activado
		// Cambiamos el giro del motor
		HAL_GPIO_TogglePin(Pin_Motor_1_GPIO_Port, Pin_Motor_1_Pin);
		HAL_GPIO_TogglePin(Pin_Motor_1_GPIO_Port, Pin_Motor_2_Pin);

		// Notificamos el cambio de giro por el UART
		xQueueSend(q_print_uart, &msg_motor, portMAX_DELAY);
		SEGGER_SYSVIEW_PrintfTarget("Giro cambiado");
	}else
	{
		// PWM: desactivado, no se produce el cambio de giro
		// Notificamos que el motor está desactivado por el UART
		xQueueSend(q_print_uart, &msg_motor_dis, portMAX_DELAY);
		SEGGER_SYSVIEW_PrintfTarget("Motor desactivado");
	}
}

// Función para leer datos de los sensores: adc y AGM, de acuerdo a en qué pantalla estemos
void read_send_datos(void)
{
	float valor_volt;

	// Averiguamos si estamos en alguna pantalla de data_handler
	if((display_state != Data_1) && (display_state != Data_2) && (display_state != Data_3) &&
		(display_state != Data_4) && (display_state != Data_5))
	{
		// No estamos en data_handler
		// leemos los datos de los sensores
		valor_volt = read_sensors();

		// Enviamos los datos por el puerto UART
		send_data_uart(valor_volt);
	}else
	{
		// Estamos en data_handler
		// Averiguamos que sensores están activos y de acuerdo a ello habilitamos el envío de datos
		// en el siguiente ciclo de la función timer de lectura de datos
		if(state_agm == SET)
			send_agm_uart++;

		if(state_adc_pwm == SET)
			send_adc_uart++;
	}
}

// Función para leer datos de los sensores: adc y AGM
float read_sensors(void)
{
	float valor_volt;

	// Averiguamos que sensores están activos
	if(state_agm == SET)
	{
		// Sensor AGM: activo, procedemos a leer los datos
		Mag_read_data(&Magne_ejes);
		Gyro_read_data(&Gyro_ejes);
		Acel_read_data(&Acel_ejes);
	}

	if(state_adc_pwm == SET)
	{
		// Sensor adc: activo
		// Averiguamos si el PWM se está controlando internamente (con el adc)
		// o externamente (por el puerto UART)
		if(ctrl_pwm == Interno_ADC)
		{
			// control interno (adc)
			// Leemos el puerto adc
			HAL_ADC_Start_IT(&hadc1);
			xSemaphoreTake(sema_adc, portMAX_DELAY);
			valor_volt = conv_adc_volt();
		}else
		{
			// control externo (puerto uart)
			// Convertimos el valor recibido por uart (0 a 100) a voltios
			valor_volt = conv_pwm_ext_volt();
		}
	}

	return valor_volt;
}

// Función para enviar los datos de una tarea a la tarea data_out
void send_data_uart(float valor_volt)
{
	char *msg_sen_dis = "Sensores desactivados\n";

	static float valor_volt_imp;
	valor_volt_imp = valor_volt;

	static char data_acel[56];
	static char data_gyro[56];
	static char data_mag[56];
	static char data_adc[16];

	static char *pdata_acel = data_acel;
	static char *pdata_gyro = data_gyro;
	static char *pdata_mag = data_mag;
	static char *pdata_adc = data_adc;

	// Preguntamos si el sensor AGM está activo
	if(state_agm == SET)
	{
		// Sensor AGM: activo,
		// enviamos los datos de AGM a la queue de salida de UART
		sprintf(data_acel, "Acelerometro: x = %.2f, y = %.2f, z = %.2f ", AGM_ejes.Acel_ejes->x, AGM_ejes.Acel_ejes->y, AGM_ejes.Acel_ejes->z);
		xQueueSend(q_print_uart, &pdata_acel, portMAX_DELAY);

		sprintf(data_gyro, "Giroscopio: x = %.2f, y = %.2f, z = %.2f ", AGM_ejes.Gyro_ejes->x, AGM_ejes.Gyro_ejes->y, AGM_ejes.Gyro_ejes->z);
		xQueueSend(q_print_uart, &pdata_gyro, portMAX_DELAY);

		// Dependiendo si adc está activo o no, enviamos la cadena con o sin '\n' al final
		if(state_adc_pwm == SET)
		{
			sprintf(data_mag, "Magnetometro: x = %.2f, y = %.2f, z = %.2f ", AGM_ejes.Gyro_ejes->x, AGM_ejes.Gyro_ejes->y, AGM_ejes.Gyro_ejes->z);
			xQueueSend(q_print_uart, &pdata_mag, portMAX_DELAY);
		}else
		{
			sprintf(data_mag, "Magnetometro: x = %.2f, y = %.2f, z = %.2f\n", AGM_ejes.Gyro_ejes->x, AGM_ejes.Gyro_ejes->y, AGM_ejes.Gyro_ejes->z);
			xQueueSend(q_print_uart, &pdata_mag, portMAX_DELAY);
		}

		SEGGER_SYSVIEW_PrintfTarget("Datos AGM enviados");
	}

	// Preguntamos si el sensor adc está activo
	if(state_adc_pwm == SET)
	{
		// Sensor ADC: activo,
		// enviamos los datos de ADC a la queue de salida de UART
		sprintf(data_adc, "Voltaje = %.2f\n", valor_volt_imp);
		xQueueSend(q_print_uart, &pdata_adc, portMAX_DELAY);
		SEGGER_SYSVIEW_PrintfTarget("Datos ADC enviados");
	}

	// Si ambos sensores están desactivados enviamos un mensaje de notificación por el puerto UART
	if((state_agm == RESET) && (state_adc_pwm == RESET))
	{
		xQueueSend(q_print_uart, &msg_sen_dis, portMAX_DELAY);
		SEGGER_SYSVIEW_PrintfTarget("Sensores desactivados");
	}
}

// Función para activar el control del PWM internamente (adc)
void active_pwm_pot(void)
{
	char *msg_pwm_pot = "Potenciometro disponible\n";

	// Actualizamos el control del PWM
	ctrl_pwm = Interno_ADC;

	// Notificamos por el puerto UART
	xQueueSend(q_print_uart, &msg_pwm_pot, portMAX_DELAY);
	SEGGER_SYSVIEW_PrintfTarget("Potenciometro disponible");
}

// Función para activar y configurar el control del PWM externo (puerto uart)
void conf_pwm_ext(command_t *cmd)
{
	char *msg_pwm_inv = "Valor invalido\n";
	char *msg_pwm_val = "Valor configurado\n";

	// Actualizamos el control del PWM
	ctrl_pwm = Externo;

	cmd->payload[8] -= 48;
	cmd->payload[9] -= 48;
	cmd->payload[10] -= 48;

	// Leemos el valor del PWM EXT enviado
	if(cmd->len == 9)
		valor_pwm_ext = (uint8_t)cmd->payload[8];
	else if(cmd->len == 10)
		valor_pwm_ext = (uint8_t)((10 * cmd->payload[8]) + cmd->payload[9]);
	else if(cmd->len == 11)
	{
		if((cmd->payload[8] == 1) && (cmd->payload[9] == 0) && (cmd->payload[10] == 0))
			valor_pwm_ext = (uint8_t)100;
		else
		{
			// si el valor no es valido, lo notificamos por el puerto uart y retornamos
			xQueueSend(q_print_uart, &msg_pwm_inv, portMAX_DELAY);
			SEGGER_SYSVIEW_PrintfTarget("Valor PWM invalido");
			return;
		}
	}else
	{
		// si el valor no es valido, lo notificamos por el puerto uart y retornamos
		xQueueSend(q_print_uart, &msg_pwm_inv, portMAX_DELAY);
		SEGGER_SYSVIEW_PrintfTarget("Valor PWM invalido");
		return;
	}

	// Actualizamos el valor del PWM
	TIM4->CCR1 = (10000 * valor_pwm_ext) / 100;

	// Notificamos por el puerto uart que el PWM se configuró
	xQueueSend(q_print_uart, &msg_pwm_val, portMAX_DELAY);
	SEGGER_SYSVIEW_PrintfTarget("Valor PWM configurado");
}

// Función que notifica que el dato recibido es inválido
void send_data_inv(void)
{
	char *msg_data_inv = "Dato invalido\n";

	xQueueSend(q_print_uart, &msg_data_inv, portMAX_DELAY);
	SEGGER_SYSVIEW_PrintfTarget("Dato invalido");
}

// Función para cambiar el estado de la pantalla e imprimir el nuevo estado en el LCD
void change_display_state(char *msg_menu, char *msg_menu_2, cadena_lcd_t *msg, state_t display)
{
	// Limpiamos e imprimimos la nueva pantalla
	send_to_lcd(0, 0, LCD_CLEAR, NULL, msg);
	send_to_lcd(1, 1, LCD_WRITE, msg_menu, msg);
	send_to_lcd(2, 1, LCD_WRITE, msg_menu_2, msg);
	// Cambiamos el estado de la pantalla
	display_state = display;
}
