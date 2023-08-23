/*
 * lcd.c
 *
 *  Created on: 12 abr. 2021
 *      Author: Joel
 */

#include "lcd.h"

static void write_4_bits(uint8_t value);
static void lcd_enable(void);
//static void delay_ms(uint32_t cnt);
static void delay_us(uint32_t cnt);

void lcd_send_command(uint8_t cmd)
{
	// RS = 0, for LCD command
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);
	//GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);

	// RW = 0, Writing to LCD
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);
	//GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(cmd >> 4);
	write_4_bits(cmd & 0x0F);
}

void lcd_send_char(uint8_t data)
{
	// RS = 1, for LCD user data
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_SET);
	//GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_SET);

	// RW = 0, Writing to LCD
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);
	//GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(data >> 4);
	write_4_bits(data & 0x0F);
}

void lcd_init(void)
{
	// 1. Configure the GPIO pins which are used for lcd connections


	GPIO_InitTypeDef lcd_signal;

	lcd_signal.Mode = GPIO_MODE_OUTPUT_PP;
	lcd_signal.Pin = LCD_GPIO_RS | LCD_GPIO_RW | LCD_GPIO_EN | LCD_GPIO_D4 | LCD_GPIO_D5 | LCD_GPIO_D6 | LCD_GPIO_D7;
	lcd_signal.Pull = GPIO_NOPULL;
	lcd_signal.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_GPIO_PORT, &lcd_signal);

/*
	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_RW;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_EN;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D4;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D5;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D6;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D7;
	GPIO_Init(&lcd_signal);
*/

	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D7, GPIO_PIN_RESET);

	// 2. Do the LCD initialization

	//delay_ms(40);
	HAL_Delay(40);

	// RS = 0, for LCD command
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);

	// RW = 0, Writing to LCD
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(0x3);

	//delay_ms(5);
	HAL_Delay(5);

	write_4_bits(0x3);

	delay_us(150);

	write_4_bits(0x3);
	write_4_bits(0x2);

	// Function set command
	lcd_send_command(LCD_CMD_4DL_2N_5X8F);

	// Display ON and cursor ON
	lcd_send_command(LCD_CMD_DISP_ON_CUR_ON);

	lcd_display_clear();

	// Entry mode set
	lcd_send_command(LCD_CMD_INC_ADD);
}

void lcd_display_clear(void)
{
	lcd_send_command(LCD_CMD_DISP_CLEAR);
	HAL_Delay(2);
	//delay_ms(2);
}

// Cursor returns home position
void lcd_display_return_home(void)
{
	lcd_send_command(LCD_CMD_DISP_RETURN_HOME);
	HAL_Delay(2);
	//delay_ms(2);
}

void lcd_print_string(char *message)
{
	do
	{
		lcd_send_char((uint8_t) *message++);
	}
	while(*message != '\0');
}

// Set to LCD a specific location given by row and column information
// Row number : 1 or 2
// Column number : 1 to 16 (for a 2x16 characters display)
void lcd_set_cursor(uint8_t row, uint8_t column)
{
	column--;
	switch(row)
	{
		case 1:
			// Set cursor to 1st row address and add index
			lcd_send_command((column |= 0x80));
			break;
		case 2:
			// Set cursor to 2nd row address and add index
			lcd_send_command((column |= 0xC0));
			break;
		default:
			break;
	}
}

// Writes 4 bits data/command on to D4, D5, D6 and D7 lines
static void write_4_bits(uint8_t value)
{
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D4, ((value >> 0) & 0x1));
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D5, ((value >> 1) & 0x1));
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D6, ((value >> 2) & 0x1));
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D7, ((value >> 3) & 0x1));

	lcd_enable();
}

static void lcd_enable(void)
{
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_RESET);
	delay_us(100);		// Execution time > 37 microseconds
}
/*
static void delay_ms(uint32_t cnt)
{
	for(uint32_t i = 0; i < (cnt * 1000); i++);
}
*/
static void delay_us(uint32_t cnt)
{
	for(uint32_t i = 0; i < (cnt * 10); i++);
}
