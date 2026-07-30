/*
 * ATmega128 I2C (TWI) LCD Display Example
 * SCL -> PD0, SDA -> PD1 (hardware TWI pins)
 * LCD via PCF8574 I2C backpack
 * Build in Atmel Studio (AVR/GNU C Compiler)
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c_lcd.h"
#include "twi.h"
#include <stdlib.h>

int main()
{
	DDRA = 0xff;
	DDRD = 0x03;
	DDRE = 0x00;
	DDRF = 0x00;
	
	PORTA = 0xff;
	
	ADMUX = 0x40;
	ADCSRA = 0x87;
	
	TWI_Init();
	LCD_Init();
	
	sei();
	
	while(1)
	{
		unsigned int adcValue = 0;
		unsigned char channel = 0x00;
		ADMUX = 0x40 | channel;
		ADCSRA |= 0x40;
		while((ADCSRA&0x10)==0);
		adcValue = ADC;
		PORTA = ~(adcValue/4);
		
		char buffer[6];
		
		
		
		LCD_Clear();
		
		LCD_SetCursor(0,0);
		LCD_Print("21th KMC");
		
		LCD_SetCursor(0,1);
		itoa(adcValue, buffer, 10);
		LCD_Print(buffer);
		
		LCD_SetCursor(6, 1);
		dtostrf((adcValue*5.0/1024), 0, 2, buffer);
		LCD_Print(buffer);
		_delay_ms(100);
	}
	
}