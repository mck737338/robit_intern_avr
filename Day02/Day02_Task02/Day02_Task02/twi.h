#ifndef TWI_H_
#define TWI_H_

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define TWI_SCL_FREQ 100000UL   // 100kHz standard I2C speed

void TWI_Init(void);
void TWI_Start(void);
void TWI_Stop(void);
void TWI_Write(uint8_t data);
uint8_t TWI_GetStatus(void);

#endif /* TWI_H_ */