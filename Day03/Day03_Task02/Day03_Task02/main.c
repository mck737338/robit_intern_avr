#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char Uart_Getch();	
void Uart_Putch(unsigned char PutData);	
void Uart_transmit_string(char *str);
void moveLeft();
void moveRight();

int main(void)
{
    UBRR0L = 16;		//Uart 통신 설정
	UBRR0H = 0;
	UCSR0A = 0x20;
	UCSR0B = 0x18;
	UCSR0C = 0x06;
	
	DDRA = 0xff;
	DDRE = 0x02;
	
	PORTA = 0xff;
	
	EIMSK = 0x10;
	EICRB = 0x02;

	sei();
    while (1) 
    {
		
		char recvData = Uart_Getch();
		if(recvData >= '0' && recvData <= '7'){		//0~7 일 때 해당 LED 켜기
			PORTA = PINA & (~(1 << (recvData - '0')));
			Uart_Putch(recvData);
			Uart_transmit_string(" LED on\r");
		}
		if(recvData == '8'){
			moveLeft();
		}
		if(recvData == '9'){
			moveRight();
		}
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

void moveLeft(){
	PORTA = 0xfe;
	for(int i = 0; i < 8; i++){	//좌측 이동
		_delay_ms(1000);
		PORTA = (PINA<<1) | 0x01;
		Uart_transmit_string("LEFT\r");		//LEFT 출력
	}
	_delay_ms(1000);
}

void moveRight(){
	PORTA = 0x7f;
	for(int i = 0; i < 8; i++){	//우측 이동
		_delay_ms(1000);
		PORTA = (PINA>>1) | 0x80;
		Uart_transmit_string("RIGHT\r");		//RIGHT 출력
	}
	_delay_ms(1000);
	
}

ISR(INT4_vect){
	PORTA = 0xff;		//LED 끄기
	Uart_transmit_string("RESET\r");
}

