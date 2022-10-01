#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#include "../lcd/lcd.h"


#define LCD_VOLTAGE_POSITION		0
#define ADC_TO_BAT_VOLTAGE(x)		(((2.56 * (float) (x)) / 1023) * 7.8)


extern void
show_voltage(const int16_t adc_val)
{
	char buffer[8];
	static char last_buffer[8] = "";

	/*	Make it human readable	*/
	sprintf(buffer, "%4.1fV", ADC_TO_BAT_VOLTAGE(adc_val));
	/**	Don't write it to LCD if
	 *	it is same with the last one.
	 */
	if (strcmp(buffer, last_buffer)) {
		lcd_command(LCD_SETDDRAMADDR | LCD_VOLTAGE_POSITION);
		lcd_printf(buffer);
		strcpy(last_buffer, buffer);
	}
}
