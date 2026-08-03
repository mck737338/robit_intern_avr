# ATmega128 과제 및 프로젝트 템플릿

> **광운대학교 로봇학부**  
> **작성자:** 김민찬
> **제출일:** 2026-08-03

---

## 1. 개요 (Overview)
본 과제는 ATmega128 마이크로컨트롤러의 ADC를 이용하여 Sharp 2Y0A02 적외선 거리 센서(PSD)의 아날로그 출력을 수신하고, 이동 평균 필터(Moving Average Filter)를 적용하여 노이즈를 저감한 뒤, UART를 통해 원시(RAW) 데이터와 필터링(FILTERED) 데이터, 그리고 환산된 거리(DISTANCE) 값을 동시에 출력하는 시스템을 구현하는 것을 목표로 함.

### 핵심 목표
* ATmega128 ADC 레지스터 설정을 통한 아날로그 센서 값 수신
* 10개 샘플 기반 이동 평균 필터를 통한 데이터 노이즈 제거
* USART를 통한 RAW / FILTERED / DISTANCE 값의 실시간 시리얼 출력
* Sharp 2Y0A02 특성식을 이용한 전압-거리 변환 및 유효 감지 범위(20~150cm) 처리

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal, F_CPU = 16000000UL) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, Sharp 2Y0A02 PSD 거리 센서 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PORTD (PD0, PD1)    ----->   출력 설정 (DDRD = 0x03)
 PE0 (RXD0) / PE1    ----->   UART 시리얼 통신 (Baud rate: UBRR0L=16)
 PORTF (전체 입력)    ----->   ADC 입력 (DDRF = 0x00)
 ADC1 (PF1)          ----->   Sharp 2Y0A02 PSD 센서 아날로그 출력
```

### 주요 회로 특징
* **전원:** 5V DC 안정화 전원 공급
* **ADC 기준전압:** AVCC 기준 (ADMUX = 0x40), 채널 1(PF1) 사용
* **주의사항:** ISP 다운로드 시 SPI 핀 타겟 전원 및 리셋 회로 간섭 주의

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c)만 단일 파일로 구성됨.
```text
├── Day00_Task00/
│   └── main.c   # ADC 수신, 이동 평균 필터, 거리 변환, UART 출력을 포함한 메인 제어 루프
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### UART / ADC 초기화 (`main.c`)
```c
UBRR0L = 16;		//Uart 통신 설정
UBRR0H = 0;
UCSR0A = 0x20;
UCSR0B = 0x18;
UCSR0C = 0x06;

DDRD = 0x03;
DDRF = 0x00;

ADMUX = 0x40;	//ADC 설정
ADCSRA = 0x87;
```

### 이동 평균 필터 초기화 (초기 9개 샘플 누적)
```c
for(int i = 0; i < 9; i++){	//초기 데이터 9개 누적
    ADMUX = 0x40 | channel;
    ADCSRA |= 0x40;
    while((ADCSRA&0x10)==0);
    dataset[i] = ADC;
    filtered += dataset[i]/10.0;
    _delay_ms(100);
}
```

### 메인 루프 내 필터 갱신 및 거리 계산
```c
raw = ADC;
filtered += raw/10.0;
dataset[9] = raw;	//데이터 10개 누적

v = filtered * 5 /1024.0;			//Distance 계산, 출력
distance = 60.374 * pow(v, -1.16);

filtered -= dataset[0]/10.0;		//첫번째 데이터 지우고 정렬
for(int i = 0; i < 9; i++){
    dataset[i] = dataset[i+1];
}
```

### UART 출력 포맷
```c
Uart_transmit_string("RAW: ");
dtostrf(raw, 0, 2, buffer);
Uart_transmit_string(buffer);
Uart_transmit_string(" | FILTERED: ");
dtostrf(filtered, 0, 2, buffer);
Uart_transmit_string(buffer);
```

---

## 6. 동작 설명 및 결과 (Results)

### 필터링 개념 및 특징
* **적용 필터:** 이동 평균 필터 (Moving Average Filter, 윈도우 크기 = 10)
* **동작 방식:** 매 루프마다 새로운 ADC 샘플을 누적하고, 가장 오래된 샘플을 제거하여 최근 10개 샘플의 평균을 유지함
* **효과:** 센서 노이즈로 인한 순간적인 값 튐(spike)을 완화하여 거리 값의 변동성을 감소시킴
* **미적용 값 대비 변화:** RAW 값은 매 샘플마다 진동 폭이 크게 나타나는 반면, FILTERED 값은 상대적으로 완만하게 변화하며 DISTANCE 계산의 안정성을 높임

### 동작 시나리오
1. 시스템 전원 인가 시 UART, ADC 초기화 및 초기 9개 샘플 누적 수행
2. 매 100ms마다 ADC 채널 1(PF1)에서 새로운 샘플 수신
3. 이동 평균 필터로 RAW 값 갱신 후 UART로 RAW / FILTERED 값 출력
4. Sharp 2Y0A02 특성식(`distance = 60.374 * v^-1.16`)을 이용해 전압을 거리(cm)로 환산
5. 유효 감지 범위(20~150cm)를 벗어나면 "too far to detect" 또는 "too close to detect" 출력, 범위 내면 거리(cm) 출력


### 동작 영상
https://drive.google.com/file/d/1UNoTLjLaH_9Ev585vCAeTXHCDzRIQCvi/view?usp=sharing

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 코드 디버깅 | - 오작동 오류 원인 분석 |

### AI 활용 및 검증 원칙
1. **학습 주도성:** 코드의 핵심 제어 로직(이동 평균 필터, 거리 환산식) 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.
