# ATmega128 과제 및 프로젝트 템플릿

> **광운대학교 로봇학부**  
> **작성자:** 김민찬
> **제출일:** 2026-08-03

---

## 1. 개요 (Overview)
본 과제는 ATmega128 마이크로컨트롤러와 PSD(Position Sensitive Detector) 센서를 활용하여 거리 데이터를 측정하고, 이를 UART 통신을 통해 PC 시리얼 터미널로 출력하는 PSD 기반 거리 측정 시스템을 구현하는 것을 목표로 함.

### 핵심 목표
* PORTF(ADC)를 이용한 PSD 센서 아날로그 값 수신
* ADC 변환 값을 센서 특성에 맞는 거리(cm) 단위로 환산
* 계산된 거리 데이터를 UART로 PC 시리얼 터미널에 출력
* 측정 주기 설정 및 비정상 센서 데이터(측정 범위 이탈) 예외처리

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, PSD 센서, USB-Serial 통신 모듈 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 ADMUX channel 1 (PF1)  ----->   PSD 센서 아날로그 출력 (ADC 입력)
 UART0 (UBRR0/UCSR0)    ----->   UART 시리얼 통신 (PC 터미널 출력)
 DDRD (PD0, PD1)        ----->   출력 설정
 DDRF                   ----->   입력 설정 (ADC 채널용)
```

### 주요 회로 특징
* **전원:** 5V DC 안정화 전원 공급
* **주의사항:** ISP 다운로드 시 SPI 핀 타겟 전원 및 리셋 회로 간섭 주의
* PSD 센서는 비선형 출력 특성을 가지므로, ADC 값을 거리(cm)로 환산 시 별도의 변환식 적용 필요

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day00_Task00/
│   ├── main.c # 메인 제어 루프, UART/ADC 초기화, 거리 계산 및 출력
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### PSD 센서 ADC 수신 및 거리 계산, UART 출력 (`main.c`)
```c
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
```

**동작 요약:**
* UART0을 통신 속도에 맞게 초기화 (`UBRR0`, `UCSR0A/B/C`)
* ADC를 Free Running이 아닌 단발 변환 방식으로 설정하고 채널 1(`0x01`)을 사용
* 초기 4개 샘플을 먼저 누적한 뒤, 매 루프마다 5개 샘플의 이동 평균(`average`)을 갱신
* 평균 ADC 값을 전압(`v`)으로 변환한 후, `distance = 60.374 * v^(-1.16)` 식으로 PSD 센서 특성에 맞는 거리(cm) 산출
* 산출된 거리가 `MIN(20cm)~MAX(150cm)` 범위를 벗어나면 각각 `"too close to detect"` / `"too far to detect"` 메시지를 출력하고, 범위 내에 있으면 `dtostrf`로 문자열 변환 후 `"cm"` 단위와 함께 UART로 출력
* `_delay_ms(100)`으로 측정 주기를 100ms로 설정

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 시스템 전원 인가 시 ATmega128 UART0, ADC 초기화함
2. PSD 센서로부터 ADC 채널 1을 통해 아날로그 값을 5개 단위 이동 평균으로 수신함
3. 이동 평균 값을 전압으로 환산 후 비선형 변환식을 통해 거리(cm) 계산함
4. 계산된 거리가 20cm~150cm 범위 내이면 UART로 거리 값을 출력하고, 범위를 벗어나면 예외 메시지("too close to detect" / "too far to detect")를 출력함
5. 100ms 주기로 측정을 반복함

### 동작 영상
https://drive.google.com/file/d/1prMMMdMCP9T0mKZRkf904OXqrf7sAEoi/view?usp=sharing

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 코드 디버깅 | - 논리 오류, 오작동 원인 분석 |

### AI 활용 및 검증 원칙
1. **코드 검증:** AI가 생성한 레지스터 설정 및 함수 코드는 데이터시트(ATmega128 Datasheet)와 비교 검증한 후 실제 시리얼 모니터링을 거쳐 직접 수정 및 테스트하였습니다.
2. **학습 주도성:** 코드의 핵심 제어 로직 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.
