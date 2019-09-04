
#include <avr/io.h>
//#include "mcu.h"

extern void inline
encoder_init(void)
{
	DDRD &= 0b10001111;
	PORTD |= 0b01110000;

	PCICR |= (1 << PCIE3);
	PCMSK3 = (1 << PCINT30) | (1 << PCINT29) | (1 << PCINT28);
}



