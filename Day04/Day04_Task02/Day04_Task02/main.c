#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "twi.h"
#include "i2c_lcd.h"

#define TCNT0_PRELOAD 6   // 16MHz, prescaler 64 기준, 1ms 주기

int year = 2026;		//시간 초기값 설정
int month = 1;
int day = 1;
int hour = 0;
int min = 0;
int sec = 0;
int ms = 0;

int index = 0;
int runState = 0;

unsigned int Read_ADC();
void print_time();

int main()
{
	DDRD = 0x03;
	DDRE  = 0x00;

	EIMSK = 0x30;
	EICRB = 0x0a;
	
	TCNT0 = TCNT0_PRELOAD;      // 카운터 초기값 설정
	TCCR0 = (1 << CS01) | (1 << CS00); // 분주비 64 설정 (CS02:0 = 011)
	TIMSK |= (1 << TOIE0);      // Timer0 오버플로우 인터럽트 허용
	
	
	DDRF = 0x00;        // PF1을 입력으로 설정
	ADMUX = 0x40; // AVCC 기준전압, 채널 1(PF1) 선택
	ADCSRA = 0x87; // ADC enable, 분주비 128
	
	TWI_Init();		//I2C, LCD 설정
	LCD_Init();

	sei();  // 전역 인터럽트 허용
	
	LCD_Print('a');

	while (1)
	{
		if(runState == 0){
			unsigned long adcValue;
			ADMUX = 0x40;
			ADCSRA |= 0x40;
			while((ADCSRA&0x10)==0);
			adcValue = ADC;
			int tday;
			switch(index){		//저항값에 따라 각 항목 수정
				case 0:
					year = adcValue *(2100 - 2026)/1024.0 + 2026;
					break;
				case 1:
					month = adcValue*12/1024.0 + 1;
					break;
				case 2:
					if((month%2 == 1 && month < 8) || (month%2 == 0 && month > 7)){		//월별  총 날짜 계산
						tday = 31;
					}
					else if(month == 2){
						if(year%4 == 0 && (year%100 != 0 || year%400 == 0)){		//윤년 판단
							tday = 29;
						}
						else {
							tday = 28;
						}
					}
					else {
						tday = 30;
					}
					day = adcValue*tday/1024.0 + 1;
					break;
				case 3:
					hour = adcValue*24/1024.0;
					break;
				case 4:
					min = adcValue*60/1024.0;
					break;
				case 5:
					sec = adcValue*60/1024.0;
					break;
				case 6:
					ms = adcValue*1000/1024.0;
					break;
			}
		}
		print_time();
		
	}
}

ISR(INT4_vect){		//sw1 눌리면 항목 변경
	index++;
	index %= 7;
}

ISR(INT5_vect)		//sw2 눌리면 시작, 다시 누르면 중지
{
	if(runState == 0)
	{
		runState = 1;
	}
	else
	{
		runState = 0;
	}
	TCNT0 = TCNT0_PRELOAD;  // 버튼 눌리면 타이머 카운트 리셋
}

// Timer0 오버플로우 인터럽트 서비스 루틴, 1ms마다 ms 증가
ISR(TIMER0_OVF_vect)
{
	if(runState == 1)		//시계 실행 중일 경우만 시간 변경
	{
		TCNT0 = TCNT0_PRELOAD;  // 다음 1ms를 위해 다시 프리로드
		ms++;
		sec += ms/1000;	ms %= 1000;		//밀리초, 초, 분, 시, 일, 월 환산
		min += sec/60;	sec %= 60;
		hour += min/60;	min %= 60;
		day += hour/24;	hour %= 24;
		if((month%2 == 1 && month < 8) || (month%2 == 0 && month > 7)){		//총 일자 판단
			month += day/31;
			day %= 31;
		}
		else if(month == 2){
			if(year%4 == 0 && (year%100 != 0 || year%400 == 0)){
				month += day/29;
				day %= 29;
			}
			else {
				month += day/28;
				day %= 28;
			}
		}
		else {
			month += day/30;
			day %= 30;
		}
		year += month/12;	month %= 12;
		year %= 10000;
	}
}

void print_time(){		//시간 출력
	char buffer[6];
	LCD_SetCursor(0,0);
	itoa(year, buffer, 10);
	LCD_Print(buffer);
	
	LCD_SetCursor(6,0);
	LCD_Print("  ");
	LCD_SetCursor(6,0);
	itoa(month, buffer, 10);
	LCD_Print(buffer);
	
	LCD_SetCursor(9,0);
	LCD_Print("  ");
	LCD_SetCursor(9,0);
	itoa(day, buffer, 10);
	LCD_Print(buffer);
	
	LCD_SetCursor(0,1);
	LCD_Print("  ");
	LCD_SetCursor(0,1);
	itoa(hour, buffer, 10);
	LCD_Print(buffer);
	
	LCD_SetCursor(3,1);
	LCD_Print("  ");
	LCD_SetCursor(3,1);
	itoa(min, buffer, 10);
	LCD_Print(buffer);
	
	LCD_SetCursor(6,1);
	LCD_Print("  ");
	LCD_SetCursor(6,1);
	itoa(sec, buffer, 10);
	LCD_Print(buffer);
	
	LCD_SetCursor(9,1);
	LCD_Print("   ");
	LCD_SetCursor(9,1);
	itoa(ms, buffer, 10);
	LCD_Print(buffer);
}