#include <avr/io.h>
#include "mcu.h"
#include "spi.h"


extern void inline
spi_init(void)
{
	/* Set CS_DDS, CS_DAC, MOSI, SCK output */
	DDR_SPI = (1 << CS_DDS) | (1 << CS_DAC) | (1 << MOSI) | (1 << SCK);
	/* Set SPI Control Register */
	SPCR = ((0 << SPIE)	| /*	SPI Interrupt Enable	*/
		(1 << SPE)	| /*	SPI Enable		*/
		(0 << DORD)	| /*	Data order 1 = LSB first*/
		(1 << MSTR)	| /*	SPI Master/Slave	*/
		(0 << CPOL)	| /*	Clock Polarity		*/
		(0 << CPHA)	| /*	Clock Phase		*/
		(0 << SPR1)	| /*	SPI Clock Rate select	*/
		(0 << SPR0));	  /*	SPI Clock Rate select	*/

	SPI_CS_DDS_HIGH;
	SPI_CS_DAC_HIGH;
} 


extern void
spi_config_mode(const uint8_t mode)
{
	volatile uint8_t r = SPCR;
	SPCR = 0;

	// FIXME: dummy set
	if (mode == SPI_MODE_TWO) {
	SPCR = ((0 << SPIE)     | /*    SPI Interrupt Enable    */ 
                (1 << SPE)      | /*    SPI Enable              */ 
                (0 << DORD)     | /*    Data order 1 = LSB first*/ 
                (1 << MSTR)     | /*    SPI Master/Slave        */ 
                (1 << CPOL)     | /*    Clock Polarity          */ 
                (0 << CPHA)     | /*    Clock Phase             */ 
                (0 << SPR1)     | /*    SPI Clock Rate select   */ 
                (0 << SPR0));     /*    SPI Clock Rate select   */
	} else {
	SPCR = ((0 << SPIE)     | /*    SPI Interrupt Enable    */ 
                (1 << SPE)      | /*    SPI Enable              */ 
                (0 << DORD)     | /*    Data order 1 = LSB first*/ 
                (1 << MSTR)     | /*    SPI Master/Slave        */ 
                (1 << CPOL)     | /*    Clock Polarity          */ 
                (1 << CPHA)     | /*    Clock Phase             */ 
                (0 << SPR1)     | /*    SPI Clock Rate select   */ 
                (0 << SPR0));     /*    SPI Clock Rate select   */
	}

	
	//SPCR = r & (0b11110011 | (mode << 2));
}


extern void
spi_master_send(const uint8_t byte)
{
	/* Start transmission */
	SPDR = byte;

	// TODO: skip or move at the begining of function
	/* Wait for transmission complete */
	while (!(SPSR & (1 << SPIF)));
}

