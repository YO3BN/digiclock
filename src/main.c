
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

//#include "hd44780/lcd.h"
#include "lcd.h"
#include "mcu.h"
#include "keypad.h"
#include "itu_table.h"
#include "si5351a.h"

#include "arch.h"
#include "kernel_api.h"

typedef enum BFO_MODE_tag {
	LSB = 1,
	USB = 2,
} eBFO_Mode;


typedef enum IF_OPERATION_tag {
	IF_OPER_NONE = 0,
	IF_OPER_ADD = 1,
	IF_OPER_SUB = 2,
} eIF_OPERATION;


// TODO these should be stored in eeprom
#define IF_CENTER 10700000
#define IF_ECART 3000

static eBFO_Mode eBfo = LSB;
static eIF_OPERATION eIFOp = IF_OPER_ADD;
static int16_t IfEcart = IF_ECART;
static uint32_t if_center = IF_CENTER;
static uint8_t fine_tune = 0;

extern uint32_t xtalFreq;

typedef enum
{
	SCAN_NONE = 0,
	SCAN_WAIT,
	SCAN_UP,
	SCAN_DOWN,
} eScan;

static volatile eScan scan = SCAN_NONE;
static uint16_t scan_time = 5;

typedef enum eMENU_ENTRY_tag
{
	MENU_ENTRY_NONE = 0,
	MENU_ENTRY_FREQ_SET,
	MENU_ENTRY_FREQ_CURSOR,
	MENU_ENTRY_IF_ECART,
	MENU_ENTRY_BFO_MODE,
	MENU_ENTRY_BFO_OFFSET,
	MENU_ENTRY_BACKLIGHT,
	MENU_ENTRY_SICLK,
	MENU_ENTRY_SCNTIME,
	MENU_ENTRY_IFCENTER,
	MENU_ENTRY_EPROM_SAVE,
	MENU_ENTRY_EPROM_ERASE,
	MENU_ENTRY_EXIT,
}eMENU_ENTRY;

static eMENU_ENTRY eMenuEntry = MENU_ENTRY_FREQ_SET;


static int32_t offset_freq = 0;


struct frequency
{
	uint32_t hz;
	uint32_t step;
} frequency;


struct eprom_data
{
	uint32_t home_freq;
	uint32_t if_center;
	uint32_t si5351_qrtz;
	uint32_t if_width;
	int32_t  if_offset;
	// TODO: add checksum of all elements stored to eprom.
} eprom_data;


static volatile uint8_t event = 0;
extern volatile uint8_t keypad_event;
extern uint32_t xtalFreq;

#define	DIAL_UP		1
#define DIAL_DOWN	2
#define PUSH_BTN	3
#define KEYPAD		4

#define LCD_FREQ_POSITION	7
#define LCD_SMTR_POSITION	0x40


static volatile uint8_t encoder = 0;
static uint8_t show_agc_dB = 0;


/* the index is used as dB count. */
static uint16_t
agc_to_dB_table[] = {
	1023,
	894,
	718,
	650,
	608,
	575,
	549,
	526,
	505,
	487,
	469,
	453,
	435,
	420,
	406,
	393,
	380,
	368,
	356,
	345,
	334,
	323,
	312,
	301,
	293,
	284,
	276,
	268,
	260,
	252,
	245,
	238,
	231,
	224,
	219,
	213,
	208,
	203,
	198,
	194,
	190,
	188,
	185,
	184,
	183,
	182,
	181,
	0
};



ISR(PCINT3_vect)
{

static volatile uint8_t a = 0;
static volatile uint8_t isr = 0;
static volatile uint8_t byte = 0;
static volatile uint8_t x = 0;

	isr = ((PIND & 0b00110000) >> 4);
	if ((PIND & 0b00001000) == 0)
	{
		encoder = PUSH_BTN;
	}

	if (isr != a )
	{
		if (isr == 2 || isr == 0 || isr == 1)
		{
			byte |= isr;
			byte <<= 2;
			x++;
		}

		if ((x >= 3) && ((byte == 0x84) || (byte == 0x48)))
		{
			switch (byte)
			{
				case 0x84:
				encoder = DIAL_UP;
				break;

				case 0x48:
				encoder = DIAL_DOWN;
				break;

				default:
				break;
			}
			byte = 0;
			x = 0;
		}
		a = isr;
	}
}


uint8_t encoder_read(void)
{
	uint8_t ret = 0;

	switch (encoder)
	{
		case 1:
		case 2:
		case 3:
			ret = encoder;
			encoder = 0;
			break;

		default:
		break;
	}

	return ret;
}


void putch_freq(char c, char pos)
{
	lcd_command(LCD_SETDDRAMADDR | pos);
	lcd_write(c);
}

void show_freq(const char *s)
{
	char *p;
	char x = 0;
	char lcd_freq_pos = 15;

	/* p points to the end of the string */
	p = s + strlen(s) - 1;

	while (p >= s)
	{
		putch_freq(*p, lcd_freq_pos--);
		x++;

		if (!(x % 3) && (p - s > 0))
		{
			putch_freq(',', lcd_freq_pos--);
		}
		p--;
	}

	/* clear the rest of frequency field */
	//TODO: hardcoded 5 is the fifth character on the screen, after voltmeter */
	while (lcd_freq_pos >= 9)
	{
		putch_freq(' ', lcd_freq_pos--);
	}

	/* Fix for FineTune: point back to the end of row 1. */
	lcd_command(LCD_SETDDRAMADDR | 15);
}



void set_freq(char force)
{
	char buffer[16];
	uint32_t vfo_freq = frequency.hz;
	uint32_t bfo_freq = if_center;

	static uint32_t last_vfo_freq = 0;
	static uint32_t last_bfo_freq = 0;

	if (force)
	{
		last_vfo_freq = 0;
		last_bfo_freq = 0;
	}

	/* Adjust value of VFO FREQ to Intermediate Frequency Center */
	switch (eIFOp)
	{
	case IF_OPER_ADD:
		vfo_freq += if_center;
		break;

	case IF_OPER_SUB:
		vfo_freq = if_center - vfo_freq;
		break;

	case IF_OPER_NONE:
		break;

	default:
		return; //TODO fail
	}


	/* Adjust value of VFO FREQ regarding the LSB or USB mode */
	if (eBfo == LSB)
	{
		switch (eIFOp)
		{
		case IF_OPER_ADD:
			vfo_freq -= IfEcart / 2;
			bfo_freq -= IfEcart / 2;
		  vfo_freq += offset_freq;
		  bfo_freq += offset_freq;
			break;

		case IF_OPER_SUB:
			vfo_freq += IfEcart / 2;
			bfo_freq += IfEcart / 2;
		  vfo_freq -= offset_freq;
		  bfo_freq -= offset_freq;
			break;

		case IF_OPER_NONE:
			break;

		default:
			return; //TODO fail
		}
	}
	else if (eBfo == USB)
	{
		switch (eIFOp)
		{
		case IF_OPER_ADD:
			vfo_freq += IfEcart / 2;
			bfo_freq += IfEcart / 2;
		  vfo_freq -= offset_freq;
		  bfo_freq -= offset_freq;
			break;

		case IF_OPER_SUB:
			vfo_freq -= IfEcart / 2;
			bfo_freq -= IfEcart / 2;
		  vfo_freq += offset_freq;
		  bfo_freq += offset_freq;
			break;

		case IF_OPER_NONE:
			break;

		default:
			return; //TODO fail
		}
	}
	else return; //TODO fail

	/* Write BFO freq to synthesis chip */
	if (last_bfo_freq != bfo_freq)
	{
		si5351aSetFrequency_CLK2(bfo_freq);
		last_bfo_freq = bfo_freq;
	}

	/* Write VFO freq to synthesis chip */
	if (last_vfo_freq != vfo_freq)
	{
		si5351aSetFrequency_CLK0(vfo_freq);
		last_vfo_freq = vfo_freq;
	}

	/* Write freq to display in KHz units */
	sprintf(buffer, "%lu", frequency.hz / 1000);
	show_freq(buffer);

	if (!show_agc_dB)
	{
		show_itu(frequency.hz / 1000);
	}

	return;
}


static void inline eprom_save(void)
{
	eprom_data.home_freq = frequency.hz;
	eprom_data.si5351_qrtz = xtalFreq;
	eprom_data.if_center = if_center;
	eprom_data.if_width = IfEcart;
	eprom_data.if_offset = offset_freq;
	eeprom_write_block(&eprom_data, 0, sizeof(eprom_data));
}

static void process_keypad(char c)
{
	static char buffer[16];
	static char *ptr = buffer;
	static uint32_t step_bkp = 0;

	/* Check for special keys */
	switch (c)
	{

	case 'A':
		// 50 Hz fine tune
		if (!fine_tune)
		{
			fine_tune = 1;
			step_bkp = frequency.step;
			frequency.step = 50; // 50 Hz step

			/* set and show the cursor on the display. */
			lcd_enable_cursor();
			lcd_set_cursor(15, 0);
			return;
		}
		else
		{
			fine_tune = 0;
			frequency.step = step_bkp;

			/* remove the cursor from the display. */
			lcd_disable_cursor();
			return;
		}
		break;

	case 'B':
		//TODO optimize
		if (eBfo != USB)
		{
			eBfo = USB;
			set_freq(0);
			show_lsb_usb();
			return;
		}

		if (eBfo != LSB)
		{
			eBfo = LSB;
			set_freq(0);
			show_lsb_usb();
			return;
		}
		break;

	case 'C':
		if (scan == SCAN_NONE)
		{
			scan = SCAN_WAIT;
			lcd_enable_blinking();
			lcd_set_cursor(15, 0);
		}
		else
		{
			scan = SCAN_NONE;
			lcd_disable_blinking();
		}

		return;
		break;

	case 'D':
		// TODO
		if (!show_agc_dB)
		{
			show_agc_dB = 1;
		}
		else
		{
			show_agc_dB = 0;

			sprintf(buffer, "                   ");
			lcd_command(LCD_SETDDRAMADDR | 0x40);
			lcd_printf(buffer);
		}
		return;
		break;

	case '#':
		//TODO beautify/optimize this
		if (atol(buffer))
		{
			/* freq enetered in KHz units */
			frequency.hz = atol(buffer) * 1000;
			set_freq(0);
			memset(buffer, 0, sizeof(buffer));
			ptr = buffer;
			return;
		}
		break;

	case '*':
		//TODO
		if (strlen(buffer) > 0)
		{
			buffer[strlen(buffer) - 1] = '\0';
			ptr--;
		}
		break;

	default:
		if (isdigit(c))
		{
			*ptr = c;
			ptr++;

			/* check overflow */
			if (ptr == &buffer[sizeof(buffer)-1])
			{
				ptr = buffer;
				*ptr = c;
				ptr++;
			}
			*ptr = '\0';
		}
		break;
	}

	show_freq(buffer);
}


static void inline eprom_erase(void)
{
	memset(&eprom_data, 0xff, sizeof(eprom_data));
	eeprom_write_block(&eprom_data, 0, sizeof(eprom_data));
}


static void inline
clear_events(void)
{
	event = 0;
}


static void inline
process_event(void)
{
	if (!event) return;

	// set scan direction
	if (scan)
	{
		switch (scan)
		{
			case SCAN_WAIT:
				// initiating scan
				switch (event)
				{
				case DIAL_UP:
					scan = SCAN_UP;
					return;
					break;

				case DIAL_DOWN:
					scan = SCAN_DOWN;
					return;
					break;

				default:
					break;
				}
				break;

			case SCAN_DOWN:
				// change scan direction
				if (event == DIAL_UP)
				{
					scan = SCAN_UP;
				}
				break;

			case SCAN_UP:
				// change scan direction
				if (event == DIAL_DOWN)
				{
					scan = SCAN_DOWN;
				}
				break;

			default:
			break;
		}
	}

	char buffer[32];
	switch (event) {
		case KEYPAD:
			*buffer = keypad_get_key();
			if (*buffer)
			{
				process_keypad(*buffer);
			}
			break;

		case DIAL_UP:
			switch (eMenuEntry)
			{
			case MENU_ENTRY_FREQ_SET:
				frequency.hz += frequency.step;
				set_freq(0);
				break;

			case MENU_ENTRY_FREQ_CURSOR:
				//TODO: change the cursor accordingly
				frequency.step *= 10;
				if (frequency.step >= 10000000)
				{
					frequency.step = 10;
					//TODO: change the cursor accordingly
				}
				//TODO remove this
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "STEP: %lu       ", frequency.step);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_IF_ECART:
				IfEcart += 100;
				set_freq(0);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "IFWIDTH: %d     ", IfEcart);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BFO_MODE:
				eBfo = USB;
				set_freq(0);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "BFO: USB     ");
				lcd_printf(buffer);
				show_lsb_usb();
				break;

			case MENU_ENTRY_BFO_OFFSET:
				offset_freq += 100;
				set_freq(0);
				sprintf(buffer, "BFO OFFSET: %li     ", offset_freq);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BACKLIGHT:
				//TODO beautify this
				DDRB = DDRB | 0b10000000;
				PORTB = PORTB | 0b10000000;

				sprintf(buffer, "LIGHT: ON    ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_SICLK:
				xtalFreq += frequency.step;
				set_freq(1);
				sprintf(buffer, "SICLK: %lu", xtalFreq);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_SCNTIME:
				scan_time += 5;
				sprintf(buffer, "SCNTIME: %u    ", scan_time);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_IFCENTER:
				if_center += frequency.step;
				set_freq(1);
				sprintf(buffer, "IFCENTER: %lu    ", if_center);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_EPROM_SAVE:
				sprintf(buffer, "EPROM SAVING ...   ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				eprom_save();
				sprintf(buffer, "EPROM DONE         ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_EPROM_ERASE:
				sprintf(buffer, "EPROM ERASING ...   ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				eprom_erase();
				sprintf(buffer, "EPROM ERASE DONE ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			default:
//				sprintf(buffer, "NOT IMPLEMENTED     ");
//				lcd_command(LCD_SETDDRAMADDR | 0x40);
//				lcd_printf(buffer);
				break;
			}


		break;

		case DIAL_DOWN:
			switch (eMenuEntry)
			{
			case MENU_ENTRY_FREQ_SET:
				frequency.hz -= frequency.step;
				set_freq(0);
				break;

			case MENU_ENTRY_FREQ_CURSOR:
				//TODO: change the cursor accordingly
				frequency.step /= 10;
				if (frequency.step < 10)
				{
					frequency.step = 1000000;
					//TODO: change the cursor accordingly
				}
				//TODO remove this
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "STEP: %lu       ", frequency.step);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_IF_ECART:
				IfEcart -= 100;
				set_freq(0);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "IFWIDTH: %d     ", IfEcart);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BFO_MODE:
				eBfo = LSB;
				set_freq(0);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "BFO: LSB     ");
				lcd_printf(buffer);
				show_lsb_usb();
				break;

			case MENU_ENTRY_BFO_OFFSET:
				offset_freq -= 100;
				set_freq(0);
				sprintf(buffer, "BFO OFFSET: %li     ", offset_freq);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BACKLIGHT:
				//TODO beautify this
				DDRB = DDRB & 0b01111111;
				PORTB = PORTB & 0b01111111;

				sprintf(buffer, "LIGHT: OFF         ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_SICLK:
				xtalFreq -= frequency.step;
				set_freq(1);
				sprintf(buffer, "SICLK: %lu", xtalFreq);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_SCNTIME:
				scan_time -= 5;
				sprintf(buffer, "SCNTIME: %u    ", scan_time);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_IFCENTER:
				if_center -= frequency.step;
				set_freq(1);
				sprintf(buffer, "IFCENTER: %lu    ", if_center);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_EPROM_SAVE:
				sprintf(buffer, "EPROM SKIPPING     ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_EPROM_ERASE:
				sprintf(buffer, "EPROM ERASE SKIP ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			default:
//				sprintf(buffer, "NOT IMPLEMENTED     ");
//				lcd_command(LCD_SETDDRAMADDR | 0x40);
//				lcd_printf(buffer);
				break;
			}
		break;

		case PUSH_BTN:
			eMenuEntry++;
			switch (eMenuEntry)
			{
			case MENU_ENTRY_FREQ_SET:
				//frequency.hz -= frequency.step;
				//set_freq();
				break;

			case MENU_ENTRY_FREQ_CURSOR:
				//TODO remove this
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "STEP: %lu       ", frequency.step);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_IF_ECART:
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "IFWIDTH: %d     ", IfEcart);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BFO_MODE:
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				sprintf(buffer, "BFO: %s            ", eBfo == USB ? "USB" : "LSB");
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BFO_OFFSET:
				sprintf(buffer, "BFO OFFSET: %li     ", offset_freq);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_BACKLIGHT:
				sprintf(buffer, "LIGHT: ON/OFF     ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;
				
			case MENU_ENTRY_SICLK:
				sprintf(buffer, "SICLK: %lu", xtalFreq);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_SCNTIME:
				sprintf(buffer, "SCNTIME: %u     ", scan_time);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_IFCENTER:
				sprintf(buffer, "IFCENTER: %lu    ", if_center);
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_EPROM_SAVE:
				sprintf(buffer, "EPROM SAVE?Y=UP");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			case MENU_ENTRY_EPROM_ERASE:
				sprintf(buffer, "EPROM ERASE? UP  ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;

			default:
				sprintf(buffer, "                   ");
				lcd_command(LCD_SETDDRAMADDR | 0x40);
				lcd_printf(buffer);
				break;
			}

			if (eMenuEntry == MENU_ENTRY_EXIT)
			{
				eMenuEntry = MENU_ENTRY_NONE;
			}
			break;
	}
}


void show_lsb_usb(void)
{
	lcd_command(LCD_SETDDRAMADDR | 6);
	if (eBfo == USB)
	{
		lcd_printf("USB");
	}
	else if (eBfo == LSB)
	{
		lcd_printf("LSB");
	}
}


static void inline
frequency_init(void)
{
	frequency.hz = 3705000;
	frequency.step = 1000; // TODO: cannot be 50000 for example
}


static void
nonvolatile_data_init(void)
{
	memset(&eprom_data, 0, sizeof(eprom_data));
	eeprom_read_block(&eprom_data, 0, sizeof(eprom_data));

	if (eprom_data.home_freq != 0xffffffff)
	{
		frequency.hz = eprom_data.home_freq;
	}

	if (eprom_data.si5351_qrtz != 0xffffffff)
	{
		xtalFreq = eprom_data.si5351_qrtz;
	}

	if (eprom_data.if_center != 0xffffffff)
	{
		if_center = eprom_data.if_center;
	}

	if (eprom_data.if_width != 0xffffffff)
	{
		IfEcart = eprom_data.if_width;
	}

	if (eprom_data.if_offset != 0xffffffff)
	{
		offset_freq = eprom_data.if_offset;
	}
}


void events(void *p)
{
	while (1)
	{
		if (!event)
		{
			if(keypad_event)
			{
				event = KEYPAD;
			}
			else
			{
				event = encoder_read();
			}
		}

		process_event();
		clear_events();
		task_sleep(0, 1);
	}
}


void vbatt(void *v)
{
	int16_t adc_value = 0;

	while (1)
	{
		task_sleep(0, 20);
		if (!show_agc_dB)
		{
			adc_value = adc_get_value();
			if (adc_value != -1) {
				show_voltage(adc_value);
			}
			adc_start_conversion(PA7);
		}
	}
}


float adc_to_dB(uint16_t adc_value)
{
	/* TODO: beautify this function. */
	int x = 0;
	int y = 0;
	int dB = 0;
	float percentage = 0;

	if (adc_value > agc_to_dB_table[0])
	{
		return 0;
	}

	/* Loop thru table till the value is found.
	 * Note: The index is used as decibell count.
	 */
	for (dB = 0; adc_value <= agc_to_dB_table[dB] && agc_to_dB_table[dB] != 0; dB++);

	/* After loop walk the index is one greater, need to decrement it by one. */
	if (dB)
	{
		dB--;
	}

	x = agc_to_dB_table[dB] - adc_value;
	y = agc_to_dB_table[dB] - agc_to_dB_table[dB + 1];

	percentage = x * 100 / y;
	percentage /= 100;

	return (dB + percentage);
}


void agc2dB(void *v)
{
	int16_t adc_value = 0;
	int16_t adc_value_last = 0;
	char buffer[32] = "";
	uint8_t i = 0;

	while (1)
	{
		if (show_agc_dB)
		{
			for (i = 0; i <= 4; i++)
			{
				adc_start_conversion(PA0);
				task_sleep(0, 1);
				adc_value = adc_get_value();

				if (adc_value > adc_value_last)
				{
					adc_value_last = adc_value;
				}
			}

			adc_value = adc_value_last;
			adc_value_last = 0;

			sprintf(buffer, "%0.2f dB    %d  ", adc_to_dB(adc_value), adc_value);
			lcd_set_cursor(0, 1);
			lcd_printf(buffer);
		}
		else
		{
			task_sleep(0, 1);
		}
	}
}


void dummy_scan(void *v)
{
	while (1)
	{
		task_sleep(0, scan_time);
		if (scan != SCAN_NONE)
		{
			// generating UP/DOWN event
			switch (scan)
			{
			case SCAN_UP:
				event = DIAL_UP;
				break;

			case SCAN_DOWN:
				event = DIAL_DOWN;
				break;

			default:
				break;
			}
		}
	}
}


/****************************************************************************
 * Name: main
 *
 * Description:
 *    Main function.
 *    Do initializations, set up task arguments, starting kernel.
 *
 * Input Parameters:
 *    none
 *
 * Returned Value:
 *    none
 *
 * Assumptions:
 *    Should never return.
 *
 ****************************************************************************/

int main(void)
{
  /* Variables. */

  /* Initializations. */

	// turn on the backlight
	DDRB = 0b10000000;
	PORTB = 0b10000000;

    // turn on the display
	DDRD = 0b00000100;
	PORTD = 0b00000100;

	lcd_init();
	lcd_on();
	lcd_clear();

	frequency_init();
	nonvolatile_data_init();
	i2cInit();
	set_freq(0);
	si5351aOutputOff(SI_CLK1_CONTROL);
	keypad_init();
	adc_init();
	encoder_init();

	show_lsb_usb();

  /* Setup task arguments. */

  /* Kernel initialization. */

  kernel_init();

  /* Creating tasks. */

  task_create("Events", &events, NULL, 0);
  task_create("VBatt", &vbatt, NULL, 0);
  task_create("Dscan", &dummy_scan, NULL, 0);
	task_create("AGC2dB", &agc2dB, NULL, 0);

  /* Starting the never-ending kernel loop. */

  kernel_start();

  /* Should not reach here. */

  return 0;
}

