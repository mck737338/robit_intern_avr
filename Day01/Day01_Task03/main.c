/*
 * Day01_Task03.c
 *
 * Created: 2026-07-30 오후 5:44:26
 * Author : USER
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	
	DDRA = 0xff;
	DDRD = 0x00;
	DDRE = 0x00;
	
	PORTA = 0x00;
	
	EIMSK = 0x3c;
	EICRA = 0xa0;
	EICRB = 0x0a;
	
	sei(); 
	
	//int count = 0;
	
    /* Replace with your application code */
    while (1) 
    {
		uint8_t count = ~PINA;
		
		count++;
		if(count == 256){
			count = 0;
		}
		PORTA = ~((uint8_t)count);
		_delay_ms(1000);
	}
}

ISR(INT2_vect){
	uint8_t a = PINA;
	PORTA = 0xfe;
	for(int i = 0; i < 8; i++){
		PORTA = ~((~PORTA) << 1);
		_delay_ms(1000);
	}
	PORTA = 0x7f;
	for(int i = 0; i < 8; i++){
		PORTA = ~(((~PORTA)>>1) & 0x7f);
		_delay_ms(1000);
	}
	PORTA = a;
}

ISR(INT3_vect){
	PORTA = 0xff;
}

ISR(INT4_vect){
	uint8_t a = PINA;
	for(int i = 0; i < 2; i++){
		PORTA = 0x1f;
		for(int j = 0; j < 8; j++){
			_delay_ms(1000);
			PORTA = ~(((~PINA) >> 1) & 0x7f);
			
		}
	}
	PORTA = a;
}

ISR(INT5_vect){
	uint8_t a = PINA;
	for(int i = 0; i < 2; i++){
		PORTA = 0xf8;
		for(int j = 0; j < 8; j++){
			_delay_ms(1000);
			PORTA = ~((~PINA) << 1);
		}
	}
	PORTA = a;
}