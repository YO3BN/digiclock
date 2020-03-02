
/* WARNING !!! These are not accurate. */

#include <util/delay.h>

void delay_1ms(void)
{
	return _delay_ms(1);
}

void delay_ms(unsigned int t)
{
	while (t > 0) {
		delay_1ms();
		t --;
	}
}

