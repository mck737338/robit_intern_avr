# ATmega128 과제 및 프로젝트 템플릿

> **광운대학교 로봇학부**  
> **작성자:** 김민찬
> **제출일:** 2026-08-03

---

## 1. 개요 (Overview)
본 과제는 ATmega128 마이크로컨트롤러와 가변저항(ADC), 외부 인터럽트 스위치(SW1, SW2), I2C LCD를 활용하여 **날짜/시간을 설정하고 실시간으로 흐르게 하는 디지털 달력 겸 시계**를 구현하는 것을 목표로 함.

### 핵심 목표
* 가변저항(ADC) 값을 이용해 연/월/일/시/분/초/밀리초 값을 순차적으로 세팅
* SW1(INT4)로 세팅 항목 전환, SW2(INT5)로 시간 흐름 시작/정지 제어
* Timer0 오버플로우 인터럽트를 이용한 1ms 단위 시간 갱신
* 윤년/월별 일수 등 날짜 예외처리
* I2C(TWI) 통신 기반 LCD에 `연-월-일 시:분:초.밀리초` 형태로 출력

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz, F_CPU 16000000UL) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, 가변저항(Potentiometer), 택트 스위치, I2C LCD |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PORTD (PD0, PD1)     ----->   I2C(TWI) - LCD 통신 (SDA/SCL)
 PE4 (INT4)           ----->   SW1 - 세팅 항목 변경
 PE5 (INT5)           ----->   SW2 - 시계 시작/정지 (Toggle)
 PF1 (ADC Channel 1)  ----->   가변저항(Potentiometer) 아날로그 입력
```

### 주요 회로 특징
* **전원:** 5V DC 안정화 전원 공급
* **인터럽트 트리거:** `EICRB = 0x0a` 설정으로 INT4, INT5 모두 하강 에지(Falling Edge) 트리거 사용
* **ADC 기준전압:** AVCC 기준, 분주비 128 (`ADCSRA = 0x87`)
* **주의사항:** ISP 다운로드 시 SPI 핀 타겟 전원 및 리셋 회로 간섭 주의

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day00_Task00/
│   ├── main.c   # 메인 제어 루프, 인터럽트 서비스 루틴, 시간 출력
│   ├── twi.c    # I2C(TWI) 통신 라이브러리
│   └── i2c_lcd.c # I2C 기반 LCD 제어 라이브러리
├── include/
│   ├── twi.h
│   └── i2c_lcd.h
└── README.md
```

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 초기화 및 메인 루프 (`main.c`)
```c
DDRD = 0x03;
DDRE  = 0x00;

EIMSK = 0x30;   // INT4, INT5 인터럽트 허용
EICRB = 0x0a;   // INT4, INT5 하강 에지 트리거

TCNT0 = TCNT0_PRELOAD;              // 카운터 초기값 설정
TCCR0 = (1 << CS01) | (1 << CS00);  // 분주비 64
TIMSK |= (1 << TOIE0);              // Timer0 오버플로우 인터럽트 허용

DDRF = 0x00;        // PF1을 입력으로 설정
ADMUX = 0x40;        // AVCC 기준전압, 채널 1(PF1) 선택
ADCSRA = 0x87;       // ADC enable, 분주비 128

TWI_Init();
LCD_Init();
sei();
```

### 항목별 ADC 값 매핑 (연/월/일/시/분/초/밀리초 세팅)
`index` 변수(SW1으로 증가, 0~6 순환)에 따라 ADC 값을 각기 다른 범위로 매핑하여 연/월/일/시/분/초/밀리초를 설정함. 특히 `day` 세팅 시에는 해당 `month`와 윤년 여부를 판단해 최대 일수(`tday`)를 계산한 뒤 매핑함.

### 스위치 인터럽트 서비스 루틴
```c
ISR(INT4_vect){        // SW1: 세팅 항목 변경
    index++;
    index %= 7;
}

ISR(INT5_vect)          // SW2: 시계 시작/정지 토글
{
    if(runState == 0) runState = 1;
    else runState = 0;
    TCNT0 = TCNT0_PRELOAD;
}
```

### 1ms 타이머 인터럽트 및 시간 환산 (`TIMER0_OVF_vect`)
`runState == 1`(시계 동작 중)일 때만 1ms마다 `ms`를 증가시키고, 밀리초→초→분→시→일→월→연 순으로 자리올림을 처리함. 월별 일수 및 윤년 여부를 판별하여 `day`, `month`를 환산함.

### LCD 출력 (`print_time`)
`itoa()`를 이용해 각 시간 요소를 문자열로 변환한 뒤, LCD 커서를 이동시키며 `연-월-일  시:분:초.밀리초` 형태로 출력함.

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 전원 인가 시 TWI/LCD, Timer0, ADC, 외부 인터럽트(INT4, INT5)를 초기화함
2. `runState == 0`(정지 상태)일 때 가변저항 값을 읽어 `index`가 가리키는 항목(연/월/일/시/분/초/밀리초)을 실시간으로 세팅함
3. SW1을 누르면 세팅 항목이 순차적으로 전환됨 (연 → 월 → 일 → 시 → 분 → 초 → 밀리초 → 연 …)
4. SW2를 누르면 `runState`가 1로 전환되며 Timer0 인터럽트를 통해 시간이 흐르기 시작함
5. 다시 SW2를 누르면 `runState`가 0으로 전환되어 시간이 정지하고, 재세팅이 가능한 상태로 복귀함
6. 시간 값은 매 루프마다 LCD에 `연-월-일 시:분:초.밀리초` 형태로 갱신 출력됨

### 예외처리
* 윤년 판단: `year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)`
* 월별 최대 일수(28/29/30/31일)에 따른 `day` 자리올림 처리
* `month`, `year` 자리올림 시 `%= 12`, `%= 10000` 처리로 범위 초과 방지

### 동작 영상
https://drive.google.com/file/d/1Y_7yVZFHw4sOEfODJXSocy75V0DEtJve/view?usp=sharing

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 코드 디버깅 | - 오작동 원인 분석 |

### AI 활용 및 검증 원칙
1. **학습 주도성:** 코드의 핵심 제어 로직(날짜 예외처리, 시간 환산 로직 등) 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.
