# ATmega128 과제 및 프로젝트 템플릿

> **광운대학교 로봇학부**  
> **작성자:** 김민찬
> **제출일:** 2026-08-03

---

## 1. 개요 (Overview)
본 과제는 ATmega128의 USART 관련 레지스터(UBRR, UCSR 등)를 전혀 사용하지 않고,
오직 **PORTD, DDRD 레지스터만을 이용하여 UART 데이터 송신을 구현하는 것을 목표로 함.

### 핵심 목표
* USART 하드웨어 모듈 없이 GPIO 토글만으로 UART 프로토콜(Start/Data/Stop bit) 구현
* `_delay_us()`를 이용한 9600bps Baudrate 타이밍 제어
* 1초 주기로 "HelloWorld!" 문자열 반복 송신

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, USB-Serial 변환 모듈 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PD2 (Software TX)   ----->   USB-Serial 모듈 RX
 GND                 ----->   USB-Serial 모듈 GND
```

### 주요 회로 특징
* **전원:** 5V DC 안정화 전원 공급
* **주의사항:** 
  - USART 하드웨어(PE0/PE1, RXD0/TXD0)를 사용하지 않으므로 임의의 GPIO 핀(PD2)을 TX로 사용함
  - DDRD를 이용하여 UART 1 핀을 이용하여 통신함
  - 레벨 변환 없이 5V TTL로 직접 연결 시 상대 장치의 입력 허용 전압 확인 필요
  - PD0/PD1은 TWI(SCL/SDA) 하드웨어 핀과 겹치므로 본 과제에서는 사용하지 않음

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day00_Task00/
│   └── main.c         # 메인 루프 및 1초 주기 송신 제어
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 비트 전송하기
```c
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
```

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 시스템 전원 인가 시 `DDRD`로 PD2를 출력으로 설정하고 High로 유지함
2. 1초마다 `Uart_transmit_string()`을 호출하여 "HelloWorld!" 문자열을 1비트씩 송신함
3. 각 비트는 Start bit(Low) → Data 8bit(LSB first) → Stop bit(High) 순서로 전송하며,
   `_delay_us(104)`로 9600bps 타이밍을 근사 구현함

### 동작 사진 / 영상

https://drive.google.com/file/d/10ewRTa1NDAHLSeTzJlIOzuP47DBAEwk0/view?usp=sharing
---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 코드 디버깅 | - 소프트웨어 UART 비트 타이밍 로직 검토 |

### AI 활용 및 검증 원칙
1. **학습 주도성:** 코드의 핵심 제어 로직(비트 타이밍, 송신 과정) 설계는 직접 작성하였으며, AI는 보조 도구(디버깅)로만 활용하였습니다.
