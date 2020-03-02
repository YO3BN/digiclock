
#include <avr/io.h>
//#include "mcu.h"

extern void inline
encoder_init(void)
{
	DDRD &= 0b11000111;
	PORTD |= 0b00111000;

	PCICR |= (1 << PCIE3);
	PCMSK3 = (1 << PCINT29) | (1 << PCINT28) | (1 << PCINT27);
}
