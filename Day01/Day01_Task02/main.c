/*
 * Day01_Task02.c
 *
 * Created: 2026-07-29 오후 3:59:18
 * Author : USER
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>



int main(void)
{
	DDRA = 0xff;
	DDRD = 0xf3;
	DDRE = 0x00;
	
	EIMSK = 0x0c;
	EICRA = 0xa0;
	
	sei();
	
    /* Replace with your application code */
    while (1) 
    {
		_delay_ms(5000);
		PORTA = ~PORTA;
		if((PINE & (1 << PINE4)) && (PINE & (1 << PINE5))){
			if(PINA){
				PORTA = 0xff;
			}
			else{
				PORTA = 0x00;
			}
		}
		else if(PINE & (1 << PINE5)){
			PORTA = 0xf0;
		}
		else if(PINE & (1 << PINE4)){
			PORTA = 0x0f;
		}
		else{
			PORTA = 0x00;
		}
		
    }
}

ISR(INT2_vect){
	PORTA = 0xff;
	int a = 0x01;
	for(int i = 0; i < 8; i++){
		PORTA = ~a;
		_delay_ms(5000);
		a = (a << 1);
	}
}

ISR(INT3_vect){
	PORTA = 0xff;
	int a = 0x80;
	for(int i = 0; i < 8; i++){
		PORTA = ~a;
		_delay_ms(5000);
		a = (a >> 1);
	}
}