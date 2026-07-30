#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include <avr/io.h>
#include <stdint.h>

/* Change this if your PCF8574 module uses a different address (common: 0x27, 0x3F) */
#define LCD_ADDR 0x27

void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_Clear(void);
void LCD_Home(void);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_Print(const char *str);
void LCD_PrintChar(char c);
void LCD_BacklightOn(void);
void LCD_BacklightOff(void);

#endif /* I2C_LCD_H_ */