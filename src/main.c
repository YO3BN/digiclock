
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "mcu.h"
#include "spi.h"
#include "keypad.h"


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
#define IF_CENTER 9998250
#define IF_ECART 2500

static eBFO_Mode eBfo = LSB;
static eIF_OPERATION eIFOp = IF_OPER_ADD;
static int16_t IfEcart = IF_ECART;

typedef enum eMENU_ENTRY_tag
{
	MENU_ENTRY_NONE = 0,
	MENU_ENTRY_FREQ_SET,
	MENU_ENTRY_FREQ_CURSOR,
	MENU_ENTRY_IF_ECART,
	MENU_ENTRY_BFO_MODE,
	MENU_ENTRY_BFO_OFFSET,
	MENU_ENTRY_BACKLIGHT,
	MENU_ENTRY_EXIT,
}eMENU_ENTRY;

static eMENU_ENTRY eMenuEntry = MENU_ENTRY_FREQ_SET;


static int32_t offset_freq = 0;


struct frequency
{
	uint32_t hz;
	uint32_t step;
} frequency;


uint8_t freq2selected = 0;

static volatile uint8_t event = 0;
extern volatile uint8_t keypad_event;

#define	DIAL_UP		1
#define DIAL_DOWN	2
#define PUSH_BTN	3
#define KEYPAD		4

#define LCD_FREQ_POSITION	7
#define LCD_SMTR_POSITION	0x40

static volatile uint8_t isr = 0;

volatile char push = 0;

ISR(PCINT3_vect)
{
	isr = ((PIND & 0b01100000) >> 5);
	if ((PIND & 0b00010000) == 0)
	{
		push = 1;
	}
}


void putch_freq(char c, char pos)
{
	lcd_send_instr(LCD_INSTR_SET_DDRAM | pos);
	lcd_send_data(c);
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

		if (!(x % 3))
		{
			putch_freq(',', lcd_freq_pos--);
		}
		p--;
	}

	/* clear the rest of frequency field */
	//TODO: hardcoded 5 is the fifth character on the screen, after voltmeter */
	while (lcd_freq_pos >= 5)
	{
		putch_freq(' ', lcd_freq_pos--);
	}
}



void set_freq(void)
{
	char buffer[16];
	uint32_t vfo_freq = frequency.hz;
	uint32_t bfo_freq = IF_CENTER;

	static uint32_t last_vfo_freq = 0;
	static uint32_t last_bfo_freq = 0;

	/* Adjust value of VFO FREQ to Intermediate Frequency Center */
	switch (eIFOp)
	{
	case IF_OPER_ADD:
		vfo_freq += IF_CENTER;
		break;

	case IF_OPER_SUB:
		vfo_freq = IF_CENTER - vfo_freq;
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
			break;

		case IF_OPER_SUB:
			vfo_freq += IfEcart / 2;
			bfo_freq += IfEcart / 2;
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
			break;

		case IF_OPER_SUB:
			vfo_freq -= IfEcart / 2;
			bfo_freq -= IfEcart / 2;
			break;

		case IF_OPER_NONE:
			break;

		default:
			return; //TODO fail
		}
	}
	else return; //TODO fail

	//TODO: move these
	vfo_freq += offset_freq;
	bfo_freq += offset_freq;
	
	/* Write BFO freq to synthesis chip */
	if (last_bfo_freq != bfo_freq)
	{
		si5351aSetFrequency2(bfo_freq);
		last_bfo_freq = bfo_freq;
	}

	/* Write VFO freq to synthesis chip */
	if (last_vfo_freq != vfo_freq)
	{
		si5351aSetFrequency1(vfo_freq);
		last_vfo_freq = vfo_freq;
	}

	/* Write freq to display */
	sprintf(buffer, "%lu", frequency.hz);
	show_freq(buffer);

	return;
}


static void process_keypad(char c)
{
	static char buffer[16];
	static char *ptr = buffer;

	/* Check for special keys */
	switch (c)
	{

	case 'A':
		//TODO
		break;

	case 'B':
		//TODO
		break;

	case 'C':
		//TODO
		break;

	case 'D':
		//TODO
		break;

	case '#':
		//TODO beautify/optimize this
		if (atol(buffer))
		{
			frequency.hz = atol(buffer);
			set_freq();
			memset(buffer, 0, sizeof(buffer));
			ptr = buffer;
			return;
		}
		break;

	case '*':
		//TODO
		break;

	default:
		if (isdigit(c))
		{
			*ptr = c;
			ptr++;

			/* check overflow */
			if (ptr == buffer[sizeof(buffer)-1])
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

static void inline
clear_events(void)
{
	event = 0;
}


static void inline
process_event(void)
{
	if (!event) return;

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
				set_freq();
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
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "STEP: %lu       ", frequency.step);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_IF_ECART:
				IfEcart += 100;
				set_freq();
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "IFWIDTH: %d     ", IfEcart);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BFO_MODE:
				eBfo = USB;
				set_freq();
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "BFO: USB     ");
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BFO_OFFSET:
				offset_freq += 100;
				set_freq();
				sprintf(buffer, "BFO OFFSET: %li     ", offset_freq);
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BACKLIGHT:
				//TODO beautify this
				DDRB = (DDRB & 0xFE) | 0x01;
				PORTB = (PORTB & 0xFE) | 0x01;

				sprintf(buffer, "LIGHT: ON    ");
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;

			default:
				sprintf(buffer, "NOT IMPLEMENTED     ");
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;
			}


		break;

		case DIAL_DOWN:
			switch (eMenuEntry)
			{
			case MENU_ENTRY_FREQ_SET:
				frequency.hz -= frequency.step;
				set_freq();
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
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "STEP: %lu       ", frequency.step);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_IF_ECART:
				IfEcart -= 100;
				set_freq();
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "IFWIDTH: %d     ", IfEcart);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BFO_MODE:
				eBfo = LSB;
				set_freq();
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "BFO: LSB     ");
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BFO_OFFSET:
				offset_freq -= 100;
				set_freq();
				sprintf(buffer, "BFO OFFSET: %li     ", offset_freq);
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BACKLIGHT:
				//TODO beautify this
				DDRB = DDRB & 0xFE;
				PORTB = PORTB & 0xFE;

				sprintf(buffer, "LIGHT: OFF         ");
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;

			default:
				sprintf(buffer, "NOT IMPLEMENTED     ");
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
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
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "STEP: %lu       ", frequency.step);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_IF_ECART:
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "IFWIDTH: %d     ", IfEcart);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BFO_MODE:
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				sprintf(buffer, "BFO: %s            ", eBfo == USB ? "USB" : "LSB");
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BFO_OFFSET:
				sprintf(buffer, "BFO OFFSET: %li     ", offset_freq);
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;

			case MENU_ENTRY_BACKLIGHT:
				sprintf(buffer, "LIGHT: ON/OFF     ");
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;

			default:
				sprintf(buffer, "                   ");
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print(buffer);
				break;
			}

			if (eMenuEntry == MENU_ENTRY_EXIT)
			{
				eMenuEntry = MENU_ENTRY_NONE;
			}
			break;
	}
}


static void inline
frequency_init(void)
{
	frequency.hz = 3705000;
	frequency.step = 1000; // TODO: cannot be 50000 for example
}

int main(void)
{
	char buffer[16];
	uint8_t a = 0;
	uint16_t zzz = 0;
	uint16_t last_push = 0;

	// turn on the backlight
	DDRB = 0b00000001;
	PORTB = 0b00000001;

//	spi_init();
//	extern void fnRFPlatformInit(void);
//	fnRFPlatformInit();

	lcd_init(LCD_SET_TWO_LINES);
	frequency_init();
	i2cInit();
	set_freq();
	keypad_init();
	adc_init();
	encoder_init();

	sei();

	uint8_t byte = 0;
	char x = 0;
	int16_t adc_value = 0;

	for (;;)
	{
		if (zzz++ == 65535) {
			zzz = 0;
			adc_value = adc_get_value();
			if (adc_value != -1) {
				show_voltage(adc_value);
			}
			adc_start_conversion(PA0);
		}

		
		if (!event && keypad_event)
		{
			event = KEYPAD;
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
					event = DIAL_DOWN;
					break;

					case 0x48:
					event = DIAL_UP;
					break;

					default:
					sprintf(buffer, "%#x ", byte);
					lcd_send_instr(LCD_INSTR_SET_DDRAM | 0);
					lcd_print(buffer);
					break;
				}
				byte = 0;
				x = 0;
			}
			a = isr;
		}
		
		/*
		 * Poor man's delayer for push button events.
		 * If pressed very fast, for example.
		 * Maybe fixed by hardware using a cap over push contact ?
		 */
		if (last_push == 0)
		{
			if (push)
			{
				event = PUSH_BTN;
				push = 0;
				last_push = 6000;
			}
		} else {
			/* if the time is not expired, then clear the PUSH event */
			last_push--;
			push = 0;
		}

		process_event();
		clear_events();
	}

	return -1;
}

