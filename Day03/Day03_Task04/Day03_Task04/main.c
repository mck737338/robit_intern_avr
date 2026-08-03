#define F_CPU 16000000UL      // 실제 보드 클럭에 맞게 수정
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BIT_DELAY_US 104       // 1/9600 ≈ 104.17us


int main(void)
{
	DDRD = 0x08;		//RX를 입력, TX를 출력으로 설정
    PORTD |= (1 << PD3);    //TX 핀 HIGH 설정

    while (1)
    {
        Uart_transmit_string("HelloWorld!");
        _delay_ms(1000);   // 1초 멈춤
    }
}

void Uart_Putch(unsigned char PutData)
{
    //시작 비트 보내기
    PORTD &= ~(1 << PD3);
    _delay_us(BIT_DELAY_US);

    //데이터 보내기
    for (unsigned char i = 0; i < 8; i++)
    {
        if (PutData & 0x01)
            PORTD |= (1 << PD3);
        else
            PORTD &= ~(1 << PD3);

        PutData >>= 1;
        _delay_us(BIT_DELAY_US);
    }

    //종료 비트 보내기
    PORTD |= (1 << PD3);
    _delay_us(BIT_DELAY_US);
}


void Uart_transmit_string(char *str)
{
    while (*str != '\0')
    {
        Uart_Putch(*str);
        str++;
    }
}