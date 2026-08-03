#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>

#define MIN 20.0
#define MAX 150.0

unsigned char Uart_Getch();
void Uart_Putch(unsigned char PutData);
void Uart_transmit_string(char *str);

int main()
{
	UBRR0L = 16;		//Uart 통신 설정
	UBRR0H = 0;
	UCSR0A = 0x20;
	UCSR0B = 0x18;
	UCSR0C = 0x06;
	
	DDRD = 0x03;
	DDRF = 0x00;
	
	ADMUX = 0x40;	//ADC 설정
	ADCSRA = 0x87;
	
	sei();
	
	unsigned int latest[5] = {0};
	float average = 0;
	float v;
	float distance;
	char buffer[10];
	
	unsigned char channel = 0x01;
	
	for(int i = 0; i < 4; i++){		//4개 데이터 누적
		ADMUX = 0x40 | channel;
		ADCSRA |= 0x40;
		while((ADCSRA&0x10)==0);
		latest[i] = ADC;
		average += latest[i]/5.0;
	}
	
	while(1)
	{
		
		ADMUX = 0x40 | channel;	
		ADCSRA |= 0x40;
		while((ADCSRA&0x10)==0);
		latest[4] = ADC;		//5번째 데이터 누적
		average += latest[4]/5.0;
		
		v = average * 5 /1024.0;		//Distance 계산, 출력
		distance = 60.374 * pow(v, -1.16);
		if(distance > MAX){
			Uart_transmit_string("too far to detect\r");
		}
		else if(distance < MIN){
			Uart_transmit_string("too close to detect\r");
		}
		else{
			dtostrf(distance, 0, 2, buffer);
			Uart_transmit_string(buffer);
			Uart_transmit_string("cm\r");
		}
		
		average -= latest[0]/5.0;		//첫 번째 데이터 삭제, 정렬
		for(int i = 0; i < 4; i++){
			latest[i] = latest[i+1];
		}
		_delay_ms(100);
	}
	
}

unsigned char Uart_Getch(){
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void Uart_Putch(unsigned char PutData){
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = PutData;
}

void Uart_transmit_string(char *str){
	while(*str != '\0'){
		Uart_Putch(*str);
		str++;
	}
}