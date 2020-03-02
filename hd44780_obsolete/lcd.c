#include "../hd44780_obsolete/lcd.h"

void lcd_print(const char *p)
{
	while (*p) {
		lcd_send_data(*p);
		p++;
	}
}

