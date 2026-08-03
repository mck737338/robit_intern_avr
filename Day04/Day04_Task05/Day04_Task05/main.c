#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

unsigned char Uart_Getch(void);
void Uart_Putch(unsigned char PutData);
void Uart_transmit_string(char *str);
void Servo_SetAngle(uint8_t angle);


int main(void)
{

	UBRR0L = 16;		//Uart 통신 설정
	UBRR0H = 0;
	UCSR0A = 0x20;
	UCSR0B = 0x18;
	UCSR0C = 0x06;

	//PORTB 출력 설정
	DDRB |= (1 << PB7);
	PORTB &= ~(1 << PB7);

	//타이머1 설정
	TCCR1A = 0x00;
	TCCR1B = (1 << WGM12) | (1 << CS11);   // CTC, 분주비 8 -> 1tick = 0.5us

	OCR1A = 39999;     // 20ms 주기 (40000 tick * 0.5us = 20ms)
	OCR1B = 3000;

	TIMSK = (1 << OCIE1A) | (1 << OCIE1B);
	sei();

	unsigned char buf[8];
	unsigned char idx = 0;
	unsigned char recvData;

	Uart_transmit_string("Servo Ready (0~180)\r\n");

	while (1)
	{
		recvData = Uart_Getch();

		if (recvData == '\r' || recvData == '\n')	//줄바꿈까지 읽기
		{
			if (idx > 0)
			{
				buf[idx] = '\0';
				int angle = atoi((char*)buf);	//입력값 정수로 변환

				Servo_SetAngle((uint8_t)angle);	//서보 동작
				Uart_transmit_string((char*)buf);
				Uart_transmit_string("\r\n");
				idx = 0;
			}
		}
		else
		{
			if (idx < sizeof(buf) - 1)
			{
				buf[idx++] = recvData;
				Uart_Putch(recvData);
			}
		}
	}
}

// 주기 시작 시점: PB7 High
ISR(TIMER1_COMPA_vect)
{
	PORTB |= (1 << PB7);
}

// 펄스폭 만큼 지난 시점: PB7 Low
ISR(TIMER1_COMPB_vect)
{
	PORTB &= ~(1 << PB7);
}

unsigned char Uart_Getch(void){
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

void Servo_SetAngle(uint8_t angle){
	if (angle > 180 || angle < 0){		//잘못된 수치는 경고 출력
		Uart_transmit_string("잘못된 범위입니다.\r");
	}
	else{
		uint32_t ticks = 2000 + ((uint32_t)angle * 2000UL) / 180UL; // 1ms~2ms
		OCR1B = (uint16_t)ticks;
	}
}