# ATmega128 과제 및 프로젝트 템플릿

> **광운대학교 로봇학부**  
> **작성자:** 김민찬  
> **제출일:** 2026-08-03

---

## 1. 개요 (Overview)
본 과제는 ATmega128의 UART0 통신과 타이머 기능을 활용하여, 시리얼 터미널에서 입력한 목표 각도에 따라 서보모터를 제어하는 시스템을 구현하는 것을 목표로 함.

### 핵심 목표
* UART 시리얼 통신을 통한 목표 각도(정수 문자열) 입력 처리
* 서보모터를 제어하기 위한 PWM 신호 구현
* 시스템 초기화 시 서보모터의 원점 복귀
* 유효 범위(0~180도)를 벗어난 입력값에 대한 경고 메시지 출력

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, SG90 서보모터, UART-USB 시리얼 모듈 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PB7 (OC1B 출력용 GPIO) ----->   서보모터 신호선 (PWM 제어)
 PE0 (RXD0) / PE1       ----->   UART 시리얼 통신 (PC 터미널)
```

### 주요 회로 특징
* **전원:** 5V DC 안정화 전원 공급 (서보모터는 별도 전원 권장)


---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day00_Task00/
│   └── main.c # 메인 제어 루프, UART/Timer 초기화, 서보 각도 제어
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 전체 구현 코드 (`main.c`)
```c
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
```

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 시스템 전원 인가 시 UART0 및 타이머를 초기화하고, PB7을 LOW로 설정하여 서보모터를 초기 위치로 이동
2. 시리얼 터미널에서 목표 각도(0~180도)를 정수 문자열로 입력
3. 줄바꿈(`\r` 또는 `\n`) 입력 시까지 버퍼에 수신 데이터를 저장한 후, `atoi()`로 정수 변환하여 `Servo_SetAngle()` 호출
4. Timer1의 OCR1B 값을 조정하여 1ms~2ms 범위의 펄스폭을 생성, 20ms 주기로 서보모터 PWM 신호 출력
5. 0도 미만 또는 180도 초과 값 입력 시 "잘못된 범위입니다." 경고 메시지를 시리얼로 출력하고 모터 동작은 유지(갱신하지 않음)

### 동작 영상

| 정면 동작 모습 | 시리얼 입력 및 응답 |
https://drive.google.com/file/d/1DXKLAMccz80qP1h9pV59yrKWrIdmi4Fp/view?usp=sharing
---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 코드 디버깅 & 자료 수집 | - 오작동 오류 원인 분석<br>- PWM 신호를 생성하기 위한 레지스터 사용법 정리 |

### AI 활용 및 검증 원칙
1. **코드 검증:** AI가 생성한 레지스터 설정 및 함수 코드는 데이터시트와 비교 검증한 후 시리얼 모니터링을 거쳐 직접 수정 및 테스트하였습니다.
2. **학습 주도성:** 코드의 핵심 제어 로직 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.
