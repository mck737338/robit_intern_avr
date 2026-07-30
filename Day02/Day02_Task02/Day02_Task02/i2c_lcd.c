#include "i2c_lcd.h"
#include "twi.h"
#include <util/delay.h>

/* PCF8574 pin mapping to LCD (common backpack wiring) */
#define LCD_BACKLIGHT_MASK 0x08
#define LCD_EN             0x04
#define LCD_RW             0x02
#define LCD_RS             0x01

static uint8_t backlightState = LCD_BACKLIGHT_MASK;

/* --- low level I2C byte send to PCF8574 --- */
static void I2C_LCD_WriteByte(uint8_t data)
{
	TWI_Start();
	TWI_Write((LCD_ADDR << 1) | 0); // write mode
	TWI_Write(data);
	TWI_Stop();
}

static void LCD_PulseEnable(uint8_t data)
{
	I2C_LCD_WriteByte(data | LCD_EN);
	_delay_us(1);
	I2C_LCD_WriteByte(data & ~LCD_EN);
	_delay_us(50);
}

static void LCD_Write4Bits(uint8_t nibble)
{
	uint8_t data = nibble | backlightState;
	I2C_LCD_WriteByte(data);
	LCD_PulseEnable(data);
}

static void LCD_SendByte(uint8_t value, uint8_t mode)
{
	uint8_t highNibble = value & 0xF0;
	uint8_t lowNibble  = (value << 4) & 0xF0;

	LCD_Write4Bits(highNibble | mode);
	LCD_Write4Bits(lowNibble  | mode);
}

void LCD_Command(uint8_t cmd)
{
	LCD_SendByte(cmd, 0);
}

void LCD_Data(uint8_t data)
{
	LCD_SendByte(data, LCD_RS);
}

void LCD_Init(void)
{
	_delay_ms(50);

	LCD_Write4Bits(0x30);
	_delay_ms(5);
	LCD_Write4Bits(0x30);
	_delay_us(150);
	LCD_Write4Bits(0x30);
	_delay_us(150);
	LCD_Write4Bits(0x20); // set 4-bit mode

	LCD_Command(0x28); // 4-bit, 2-line, 5x8 font
	LCD_Command(0x0C); // display ON, cursor OFF, blink OFF
	LCD_Command(0x06); // entry mode set
	LCD_Command(0x01); // clear display
	_delay_ms(2);
}

void LCD_Clear(void)
{
	LCD_Command(0x01);
	_delay_ms(2);
}

void LCD_Home(void)
{
	LCD_Command(0x02);
	_delay_ms(2);
}

void LCD_SetCursor(uint8_t col, uint8_t row)
{
	static const uint8_t rowOffsets[] = {0x00, 0x40, 0x14, 0x54};
	if (row > 3) row = 3;
	LCD_Command(0x80 | (col + rowOffsets[row]));
}

void LCD_PrintChar(char c)
{
	LCD_Data((uint8_t)c);
}

void LCD_Print(const char *str)
{
	while (*str)
	{
		LCD_Data((uint8_t)*str);
		str++;
	}
}

void LCD_BacklightOn(void)
{
	backlightState = LCD_BACKLIGHT_MASK;
	I2C_LCD_WriteByte(backlightState);
}

void LCD_BacklightOff(void)
{
	backlightState = 0x00;
	I2C_LCD_WriteByte(backlightState);
}