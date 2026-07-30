#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	
	DDRA = 0xff;	//입출력 핀 설정
	DDRD = 0x00;
	DDRE = 0x00;
	
	PORTA = 0xff;	//LED 모두 끄기
	
	EIMSK = 0x3c;	//인터럽트 설정
	EICRA = 0xa0;	//INT 2, 3 Falling edge 설정
	EICRB = 0x0a;	//INT 4, 5 Falling edge 설정
	
	sei(); 
	
    while (1) 
    {
		
		_delay_ms(1000);
		int count = ~PINA;		//PORTA의 이진 데이터 저장
		
		count++;
		if(count == 256){			//만약 데이터가 1 0000 0000이면 0000 0000으로 저장
			count = 0;
		}
		PORTA = ~(count);		//Active Low방식이므로 비트 반전하여 출력
		
	}
}

ISR(INT2_vect){
	int a = PINA;		//원래 값 저장
	PORTA = 0xfe;
	for(int i = 0; i < 8; i++){		//1111 1110 왼쪽으로 1비트씩 8번 이동
		PORTA = ~((~PORTA) << 1);
		_delay_ms(1000);
	}
	PORTA = 0x7f;
	for(int i = 0; i < 8; i++){		//0111 1111 오른쪽으로 1비트씩 8번 이동
		PORTA = ~(((~PORTA)>>1) & 0x7f);
		_delay_ms(1000);
	}
	PORTA = a;		//원래 값 출력
}

ISR(INT3_vect){
	PORTA = 0xff;	//값 초기화
}

ISR(INT4_vect){
	int a = PINA;
	for(int i = 0; i < 2; i++){	//2회 반복
		PORTA = 0x1f;
		for(int j = 0; j < 8; j++){	//0001 1111 1비트씩 오른쪽으로 8번 이동
			_delay_ms(1000);
			PORTA = ~(((~PINA) >> 1) & 0x7f);
			
		}
	}
	PORTA = a;
}

ISR(INT5_vect){
	uint8_t a = PINA;
	for(int i = 0; i < 2; i++){	//2회 반복
		PORTA = 0xf8;
		for(int j = 0; j < 8; j++){		//1111 1000 1비트씩 왼쪽으로 8번 이동
			_delay_ms(1000);
			PORTA = ~((~PINA) << 1);
		}
	}
	PORTA = a;
}