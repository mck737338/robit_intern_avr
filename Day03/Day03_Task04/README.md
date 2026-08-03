# ATmega128 과제 및 프로젝트 템플릿

> **광운대학교 (학부명)**  
> **작성자:** (이름)
> **제출일:** (날짜)

---

## 1. 개요 (Overview)
본 과제는 ATmega128의 USART 관련 레지스터(UBRR, UCSR 등)를 전혀 사용하지 않고,
오직 **PORTD, DDRD 레지스터만을 이용한 소프트웨어 방식(비트뱅잉, Bit-Banging)**으로
UART 데이터 송신을 구현하는 것을 목표로 함.

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
| **주요 부품** | ATmega128 개발보드, USB-Serial 변환 모듈(FTDI 등), 오실로스코프/시리얼 터미널 |

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
  - 레벨 변환 없이 5V TTL로 직접 연결 시 상대 장치의 입력 허용 전압 확인 필요
  - PD0/PD1은 TWI(SCL/SDA) 하드웨어 핀과 겹치므로 본 과제에서는 사용하지 않음

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day00_Task00/
│   ├── main.c         # 메인 루프 및 1초 주기 송신 제어
│   ├── soft_uart.c     # PORTD/DDRD 기반 소프트웨어 UART 드라이버
│   └── soft_uart.h
├── include/
│   └── soft_uart.h
├── docs/
│   └── schematic.pdf   # 회로도 파일
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 소프트웨어 UART 송신 구현 (`soft_uart.c`)
```c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define TX_PIN        PD2
#define BIT_DELAY_US  104   // 9600bps -> 1비트 주기 약 104.17us

// TX 핀 초기화 (Idle 상태 = High)
void UART_Init(void)
{
    DDRD  |= (1 << TX_PIN);
    PORTD |= (1 << TX_PIN);
}

// 1비트 전송 (레벨 설정 + 비트 시간만큼 대기)
static void UART_TxBit(unsigned char bit)
{
    if (bit)
        PORTD |= (1 << TX_PIN);
    else
        PORTD &= ~(1 << TX_PIN);

    _delay_us(BIT_DELAY_US);
}

// 1바이트 전송: Start bit -> Data 8bit(LSB first) -> Stop bit
void UART_TxChar(unsigned char data)
{
    UART_TxBit(0);   // Start bit

    for (unsigned char i = 0; i < 8; i++)
    {
        UART_TxBit(data & 0x01);
        data >>= 1;
    }

    UART_TxBit(1);   // Stop bit
}

// 문자열 전송
void UART_TxString(const char *str)
{
    while (*str)
    {
        UART_TxChar(*str++);
    }
}
```

### 메인 루프 (`main.c`)
```c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "soft_uart.h"

int main(void)
{
    UART_Init();

    while (1)
    {
        UART_TxString("HelloWorld!\r\n");
        _delay_ms(1000);
    }
}
```

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 시스템 전원 인가 시 `DDRD`로 PD2를 출력으로 설정하고 Idle 상태(High)로 유지함
2. 1초마다 `UART_TxString()`을 호출하여 "HelloWorld!" 문자열을 1비트씩 GPIO 토글로 송신함
3. 각 비트는 Start bit(Low) → Data 8bit(LSB first) → Stop bit(High) 순서로 전송하며,
   `_delay_us(104)`로 9600bps 타이밍을 근사 구현함

### 한계 및 보정 사항
* 함수 호출/분기 오버헤드로 인해 실제 비트 주기가 이론값(104.17us)보다 다소 길어질 수 있음
  → 오실로스코프로 실측 후 `BIT_DELAY_US` 값을 미세 조정 필요
* 인터럽트를 사용하지 않는 순수 블로킹(Blocking) 방식이므로, 송신 중 다른 인터럽트 처리가 지연될 수 있음

### 동작 사진 / 영상

| 오실로스코프 파형 | 시리얼 터미널 출력 |
| :---: | :---: |
| ![Waveform](개인_구글드라이브_링크_첨부) | ![Serial Monitor](개인_구글드라이브_링크_첨부) |

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **ChatGPT / Claude** | 코드 디버깅 & 리팩토링 | - 소프트웨어 UART 비트 타이밍 로직 검토<br>- 레지스터 설정 주석 작성 및 가독성 개선 |
| **Gemini** | 개념 정리 & 모듈 설계 | - Bit-Banging 방식 UART 동작 원리 검토<br>- 프로젝트 파일/디렉토리 구조 설계 참고 |

### AI 활용 및 검증 원칙
1. **코드 검증:** AI가 생성한 타이밍 로직 및 함수 코드는 데이터시트(ATmega128 Datasheet)와 비교 검증한 후 실제 오실로스코프/시리얼 모니터링을 거쳐 직접 수정 및 테스트하였습니다.
2. **학습 주도성:** 코드의 핵심 제어 로직(비트 타이밍, 송신 시퀀스) 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.
