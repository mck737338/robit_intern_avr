#include "twi.h"

void TWI_Init(void)
{
	TWSR = 0x00; // prescaler = 1
	TWBR = ((F_CPU / TWI_SCL_FREQ) - 16) / 2;
	TWCR = (1 << TWEN);
}

void TWI_Start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void TWI_Stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	while (TWCR & (1 << TWSTO));
}

void TWI_Write(uint8_t data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

uint8_t TWI_GetStatus(void)
{
	uint8_t status = TWSR;
	status &= 0xF8; // mask prescaler bits
	return status;
}