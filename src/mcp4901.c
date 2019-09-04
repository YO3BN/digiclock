
#include <stdio.h>
#include <avr/io.h>
#include "mcp4901.h"
#include "spi.h"

static uint16_t dac_config = 0;


extern void
dac_init(void)
{
	dac_config = (
		(0 << ZERO)	|	/*	Always zero		*/
		(1 << BUF)	|	/*	Vref Input Buffer	*/
		(1 << GA)	|	/*	Gain Selection		*/
		(1 << SHDN));		/*	Shutdown Control	*/
}


extern void
dac_write(const uint8_t value)
{
	uint16_t dac_reg = value;
	dac_reg = dac_config | (dac_reg << 4);

	spi_config_mode(SPI_MODE_THREE);

	SPI_CS_DAC_LOW;
	spi_master_send((uint8_t) ((dac_reg & 0xff00) >> 8));
	spi_master_send((uint8_t) (dac_reg & 0x00ff));
	SPI_CS_DAC_HIGH;
}

