#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	DDRA = 0xff;	//A포트 출력 설정
	DDRD = 0x00;	//D포트 입력 설정
	DDRE = 0x00;	//E포트 입력 설정
	PORTA = 0xff;
	
	EIMSK = 0x0c;	//INT 2, 3 인터럽트 설정
	EICRA = 0xa0;	//INT 2, 3 인터럽트 Falling edge로 설정
	
	sei();			//인러텁트 활성화
	
	
    while (1) 
    {
		_delay_ms(5000);;
		if((PINE & (1 << PINE4)) && (PINE & (1 << PINE5))){		//두 버튼 모두 안 눌렸을 때
			if(PINA){		//꺼진 LED가 있으면 모두 켜기
				PORTA = 0x00;
			}
			else{			//아니면 모두 끄기
				PORTA = 0xff;
			}
		}
		else if(PINE & (1 << PINE5)){		//버튼 1만 안 눌렸을 때
			PORTA = 0xf0;
		}
		else if(PINE & (1 << PINE4)){		//버튼 0만 안 눌렸을 때
			PORTA = 0x0f;
		}
		else{								//두 버튼 모두 눌렸을 때
			PORTA = 0x00;
		}
		
    }
}

ISR(INT2_vect){
	PORTA = 0xff;
	int a = 0x01;
	for(int i = 0; i < 8; i++){	//0000 0001을 왼쪽으로 1비트씩 8번 이동
		PORTA = ~a;
		_delay_ms(5000);
		a = (a << 1);
	}
}

ISR(INT3_vect){
	PORTA = 0xff;
	int a = 0x80;
	for(int i = 0; i < 8; i++){	//1000 0000을 오른쪽으로 1비트씩 8번 이동
		PORTA = ~a;
		_delay_ms(5000);
		a = (a >> 1);
	}
}