#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "i2c.h"

#define I2C_START 0x08
#define I2C_START_RPT 0x10
#define I2C_SLA_W_ACK 0x18
#define I2C_SLA_R_ACK 0x40
#define I2C_DATA_ACK 0x28
#define I2C_WRITE 0b11000000
#define I2C_READ  0b11000001

uint8_t i2cStart()
{
	uint8_t safety = 0;
	
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	while (!(TWCR & (1<<TWINT)))
	{
		safety++;
		if (safety >= 200)
		{
			return 0;
		}
	}

	return (TWSR & 0xF8);
}

void i2cStop()
{
	uint8_t safety = 0;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

	while ((TWCR & (1<<TWSTO)))
	{
		safety++;
		if (safety >= 200)
		{
			break;
		}
	}
}

uint8_t i2cByteSend(uint8_t data)
{
	uint8_t safety = 0;
	TWDR = data;

	TWCR = (1<<TWINT) | (1<<TWEN);

	while (!(TWCR & (1<<TWINT)))
	{
		safety++;
		if (safety >= 200)
		{
			return 0;
		}
	}

	return (TWSR & 0xF8);
}

uint8_t i2cByteRead()
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	uint8_t safety = 0;
	
	while (!(TWCR & (1<<TWINT)))
	{
		safety++;
		if (safety >= 200)
		{
			return 0;
		}
	}

	return (TWDR);
}

uint8_t i2cSendRegister(uint8_t reg, uint8_t data)
{
	uint8_t stts;
	
	stts = i2cStart();
	if (stts != I2C_START) return 1;

	stts = i2cByteSend(I2C_WRITE);
	if (stts != I2C_SLA_W_ACK) return 2;

	stts = i2cByteSend(reg);
	if (stts != I2C_DATA_ACK) return 3;

	stts = i2cByteSend(data);
	if (stts != I2C_DATA_ACK) return 4;

	i2cStop();

	return 0;
}

uint8_t i2cReadRegister(uint8_t reg, uint8_t *data)
{
	uint8_t stts;
	
	stts = i2cStart();
	if (stts != I2C_START) return 1;

	stts = i2cByteSend(I2C_WRITE);
	if (stts != I2C_SLA_W_ACK) return 2;
 
	stts = i2cByteSend(reg);
	if (stts != I2C_DATA_ACK) return 3;

	stts = i2cStart();
	if (stts != I2C_START_RPT) return 4;

	stts = i2cByteSend(I2C_READ);
	if (stts != I2C_SLA_R_ACK) return 5;

	*data = i2cByteRead();

	i2cStop();

	return 0;
}

// Init TWI (I2C)
//
void i2cInit()
{
	TWCR = 0;
	TWBR = 10;						
	TWSR = 0;
	//TWDR = 0xFF;
	
	TWCR = 0x04;
	//PRR = 0; // removed by cristian ionita
}
