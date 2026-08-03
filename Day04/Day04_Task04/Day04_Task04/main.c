#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>

#define MIN 20.0		//Sharp 2y0a02 스펙, 감지거리 20~150cm
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
	
	unsigned int dataset[10] = {0};
	int num = 0;
	float v;
	float distance;
	float raw;
	float filtered = 0;
	char buffer[10];
	
	unsigned char channel = 0x01;
	
	for(int i = 0; i < 9; i++){	//초기 데이터 9개 누적
		ADMUX = 0x40 | channel;
		ADCSRA |= 0x40;
		while((ADCSRA&0x10)==0);
		dataset[i] = ADC;
		filtered += dataset[i]/10.0;
		_delay_ms(100);
	}
	
	while(1)
	{
		
		ADMUX = 0x40 | channel;
		ADCSRA |= 0x40;
		while((ADCSRA&0x10)==0);
		raw = ADC;
		filtered += raw/10.0;
		dataset[9] = raw;	//데이터 10개 누적
		
		
		Uart_transmit_string("RAW: ");		//Raw, Filtered 출력
		dtostrf(raw, 0, 2, buffer);
		Uart_transmit_string(buffer);
		Uart_transmit_string(" | FILTERED: ");;
		dtostrf(filtered, 0, 2, buffer);
		Uart_transmit_string(buffer);
		
		v = filtered * 5 /1024.0;			//Distance 계산, 출력
		distance = 60.374 * pow(v, -1.16);
		Uart_transmit_string(" | DISTANCE: ");
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
		
		filtered -= dataset[0]/10.0;		//첫번째 데이터 지우고 정렬
		for(int i = 0; i < 9; i++){
			dataset[i] = dataset[i+1];
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