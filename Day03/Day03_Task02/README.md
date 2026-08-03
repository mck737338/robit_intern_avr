
> **광운대학교 로봇학부**  
> **작성자:** 김민찬
> **제출일:** 2026-08-03

---

## 1. 개요 (Overview)
본 과제는 ATmega128 마이크로컨트롤러를 Uart 통신의 구현과 Uart 통신을 이용하여 원하는 LED를 제어할 수 있도록 하는 것을 목표로 함.

### 핵심 목표
* ATmega128의 Uart 관련 레지스터의 사용법 이해
* Uart 통신을 이용하여 값을 읽고 출력 제어

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, LED, TTL - UART 송수신 모듈 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PORTA (PA0 ~ PA7)   ----->   8-Bit LED
 PE0 (RXD0) / PE1    ----->   UART Serial Communication
 INT4 (PE4)          ----->   Extern Interrupt
```

### 주요 회로 특징
* **전원:** 5V DC 전원 공급
* **주의사항:** ISP 선과 UART 선 동시연결 하지 않도록 주의

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day03_Task02/
│   └── main.c # 시스템 초기화 및 UART 통신 값 읽기, LED 켜기
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 시스템 초기화, 통신 값 읽기
```c
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
```

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. Atmega128 전원 인가 시 UART 수신 시작
2. 0~7의 숫자 수신 시 해당 LED 켜기
3. 8 또는 9 수신 시 각각 좌측, 우측으로 LED 이동, LEFT/RIGHT 송신
4. sw1 눌리면 모든 LED 끄고 RESET 송신

### 동작 영상
[text](https://drive.google.com/file/d/1NC53sSe6ts7jvn9i1G6Tm-y5lanbICa5/view?usp=sharing)

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| Claude** | 코드 디버깅 | - 오작동 원인 분석<br> |

### AI 활용 및 검증 원칙
1. **학습 주도성:** 코드의 핵심 제어 로직 설계는 직접 작성하였으며, AI는 보조 도구(디버깅)로만 활용하였습니다.
