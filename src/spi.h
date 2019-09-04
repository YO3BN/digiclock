#include "mcu.h"


#define	SPI_CS_DDS_LOW	PORT_SPI &= ~(1 << CS_DDS)
#define	SPI_CS_DDS_HIGH	PORT_SPI |= (1 << CS_DDS)

#define SPI_CS_DAC_LOW	PORT_SPI &= ~(1 << CS_DAC)
#define	SPI_CS_DAC_HIGH	PORT_SPI |= (1 << CS_DAC)

#define SPI_MODE_ZERO	0x00
#define SPI_MODE_ONE	0x01
#define SPI_MODE_TWO	0x02
#define SPI_MODE_THREE	0x03

