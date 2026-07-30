#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "twi.h"
#include "i2c_lcd.h"

int a = 1;	//A, B 초기값 설정
int b = 1;
char opr[4] = {'+', '-', '*', '/'};		//연산자
int index = 0;		//현재 연산자 인덱스

char buffer[4];

int main(){
	DDRD = 0x03;	//입출력 설정
	DDRE = 0x00;
	
	PORTD = 0x0C;
	PORTE = 0x30;
	
	EIMSK = 0x3c;	//인터럽트 설정
	EICRA = 0xa0;
	EICRB = 0x0a;
	
	sei();
	
	TWI_Init();		//LCD 설정
	LCD_Init();
	
	LCD_SetCursor(0, 0);		//초기 화면 출력
	itoa(a, buffer, 10);
	LCD_Print(buffer);
	LCD_SetCursor(5, 0);
	LCD_PrintChar(opr[index]);
	LCD_SetCursor(7, 0);
	itoa(b, buffer, 10);
	LCD_Print(buffer);
	
	while(1){
		
	}
	
}

ISR(INT4_vect){		//A 증가 후 화면 출력
	a++;
	LCD_SetCursor(0, 0);
	LCD_Print("    ");
	LCD_SetCursor(0, 0);
	itoa(a, buffer, 10);
	LCD_Print(buffer);
	
}

ISR(INT5_vect){	// 연산자 변경 후 화면 출력
	index = (index + 1) % 4;
	LCD_SetCursor(5, 0);
	LCD_PrintChar(opr[index]);
}

ISR(INT2_vect){	//B 증가 후 화면 출력
	b++;
	LCD_SetCursor(7, 0);
	LCD_Print("    ");
	LCD_SetCursor(7, 0);
	itoa(b, buffer, 10);
	LCD_Print(buffer);
}

ISR(INT3_vect){	//연산 후 '=', 결과값 출력
	float result = 0;
		
	switch(opr[index]){
		case '+': 
			result = (float)(a + b);
			break;
		case '-':
			result = (float)(a - b);
			break;
		case '*':
			result = (float)(a * b);
			break;
		case '/':
			result = (float)a / (float)b;
			break;
		default:;
	}
		
	LCD_SetCursor(13, 0);
	LCD_Print("    ");
	LCD_SetCursor(11, 0);
	LCD_Print("= ");
		
	int decimals = (opr[index] == '/') ? 2 : 0;
	dtostrf(result, 0, decimals, buffer);
	LCD_Print(buffer);
}