
#include <avr/io.h>
#include <util/delay.h>

#include "../hd44780_obsolete/lcd.h"

void main(void)
{
	lcd_init(LCD_SET_TWO_LINES);

	/* Print on line one */
	lcd_print("Hello World!!");
	_delay_ms(5000);

	/* Clear the display */
	lcd_send_instr(LCD_INSTR_CLEAR_DISPLAY);
	_delay_ms(1000);

	/* Now print on two lines */
	lcd_print("This is line one");
	_delay_ms(2000);
	/* Shift to line two */
	lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
	lcd_print("This is line two");
	_delay_ms(2000);

	for (;;);
}

