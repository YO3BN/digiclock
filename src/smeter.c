
#include <avr/io.h>
#include "lcd.h"

struct adc_value {
	uint16_t adc_min;
	uint16_t adc_max;
} adc_value;


struct adc_value smeter_table[] = {
	/* min, max */

	{0, 0},			/* S0 		*/
	{60, 69},		/* S1		*/
	{70, 79},		/* S2		*/
	{80, 89},		/* S3		*/
	{90, 93},		/* S4		*/
	{93, 94},		/* S5		*/
	{95, 96},		/* S6		*/
	{97, 110},		/* S7		*/
	{111, 140},		/* S8		*/
	{1016, 1017},		/* S9		*/
	{1012, 1015},		/* S9 + 3	*/
	{1005, 1011},		/* S9 + 5	*/
	{980, 1004},		/* S9 + 7	*/
	{920, 979},		/* S9 + 9	*/
	{850, 919},		/* S9 + 10	*/
	{750, 849},		/* S9 + 11	*/
	{700, 749},		/* S9 + 12	*/
	{640, 699},		/* S9 + 13	*/
	{500, 639},		/* S9 + 15	*/
	{360, 499},		/* S9 + 17	*/
	{345, 359},		/* S9 + 21	*/
	{260, 344},		/* S9 + 24	*/
	{240, 259},		/* S9 + 26	*/
	{223, 239},		/* S9 + 28	*/
	{215, 222},		/* S9 + 30	*/
	{203, 214},		/* S9 + 31	*/
	{200, 202},		/* S9 + 32	*/
	{190, 199},		/* S9 + 33	*/
	{185, 189},		/* S9 + 34	*/
	{180, 185},		/* S9 + 35	*/
	{176, 179},		/* S9 + 36	*/
	{175, 175},		/* S9 + 37	*/
	{174, 174},		/* S9 + 42	*/
	{173, 173},		/* S9 + 50	*/
	{170, 172},		/* S9 + 60	*/

	{0, 0}				/* None		*/
};

void show_smeter(uint8_t s)
{
	static int8_t last_s = -1;

	if (last_s == s)
		return;
	
	last_s = s;


	lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);


	switch(s) {
		case 0:
		lcd_print("S0      ");
		break;

		case 1:
		lcd_print("S1      ");
		break;

		case 2:
		lcd_print("S2      ");
		break;

		case 3:
		lcd_print("S3      ");
		break;

		case 4:
		lcd_print("S4      ");
		break;

		case 5:
		lcd_print("S5      ");
		break;

		case 6:
		lcd_print("S6      ");
		break;

		case 7:
		lcd_print("S7      ");
		break;

		case 8:
		lcd_print("S8      ");
		break;

		case 9:
		lcd_print("S9      ");
		break;

		case 10:
		lcd_print("S9 + 3  ");
		break;

		case 11:
		lcd_print("S9 + 5  ");
		break;

		case 12:
		lcd_print("S9 + 7   ");
		break;

		case 13:
		lcd_print("S9 + 9   ");
		break;

		case 14:
		lcd_print("S9 + 10  ");
		break;

		case 15:
		lcd_print("S9 + 11  ");
		break;

		case 16:
		lcd_print("S9 + 12   ");
		break;


		case 17:
		lcd_print("S9 + 13   ");
		break;

		case 18:
		lcd_print("S9 + 15   ");
		break;

		case 19:
		lcd_print("S9 + 17   ");
		break;

		case 20:
		lcd_print("S9 + 21   ");
		break;

		case 21:
		lcd_print("S9 + 24   ");
		break;

		case 22:
		lcd_print("S9 + 26   ");
		break;

		case 23:
		lcd_print("S9 + 28   ");
		break;

		case 24:
		lcd_print("S9 + 30   ");
		break;

		case 25:
		lcd_print("S9 + 31   ");
		break;

		case 26:
		lcd_print("S9 + 32   ");
		break;

		case 27:
		lcd_print("S9 + 33   ");
		break;

		case 28:
		lcd_print("S9 + 34   ");
		break;

		case 29:
		lcd_print("S9 + 35   ");
		break;

		case 30:
		lcd_print("S9 + 36   ");
		break;

		case 31:
		lcd_print("S9 + 37   ");
		break;

		case 32:
		lcd_print("S9 + 42   ");
		break;

		case 33:
		lcd_print("S9 + 50   ");
		break;

		case 34:
		lcd_print("S9 + 60   ");
		break;

	}
}



void process_smeter(void)
{
	uint16_t rssi_adc_val = 0;
	uint16_t agc_adc_val = 0;
	uint8_t	i;
	static uint8_t last_smeter_value = 0;
	static uint8_t delay = 10;
	uint16_t adc_value;

	rssi_adc_val = adc_start_conversion(PA1, &adc_value); // <--- replace PA0
	agc_adc_val = adc_start_conversion(PA2, &adc_value); // <--- replace PA0

	/* Bitx hack */
	if (agc_adc_val >= 1020) {
		adc_value = rssi_adc_val;
	} else adc_value = agc_adc_val;

#if 0
	char buffer[100];
	static char last_buffer[100];
	sprintf(buffer, "%d   %d  ", rssi_adc_val, agc_adc_val);
	/**	Don't write it to LCD if
	 *	it is same with the last one.
	 */
	if (strcmp(buffer, last_buffer)) {
		lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
		lcd_print(buffer);
		strcpy(last_buffer, buffer);
	}
	return;

#endif
	// FIXME: 35 replaced by sizeof struct
	for (i = 0; i <= 34; i++) {
		if (adc_value >= smeter_table[i].adc_min && adc_value <= smeter_table[i].adc_max)
			break;
	}

	/* do not erase the biggest value for a while */
	if (i <= last_smeter_value) {
		if (delay) {
			delay--;
			return;
		} else {
			delay = 10;
			last_smeter_value = i;
		}
	} else {
		last_smeter_value = i;
	}

	show_smeter(i);
	return;
}

