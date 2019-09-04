

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "lcd.h"
#include "mcu.h"
#include "spi.h"


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


struct frequency
{
	uint32_t hz;
	uint32_t step;
} frequency;

struct frequency2
{
	uint32_t hz;
	uint32_t step;
} frequency2;

uint8_t freq2selected = 0;

static uint8_t event = 0;
#define	DIAL_UP		1
#define DIAL_DOWN	2
#define PUSH_BTN	3

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
			vfo_freq -= IF_ECART / 2;
			bfo_freq -= IF_ECART / 2;
			break;

		case IF_OPER_SUB:
			vfo_freq += IF_ECART / 2;
			bfo_freq += IF_ECART / 2;
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
			vfo_freq += IF_ECART / 2;
			bfo_freq += IF_ECART / 2;
			break;

		case IF_OPER_SUB:
			vfo_freq -= IF_ECART / 2;
			bfo_freq -= IF_ECART / 2;
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
	//TODO with commas
	if (1)
	{
		sprintf(buffer, "%lu    ", frequency.hz);
		lcd_send_instr(LCD_INSTR_SET_DDRAM | LCD_FREQ_POSITION);
		lcd_print(buffer);
	}

	return;
}

static void inline
clear_events(void)
{
	event = 0;
}


static void
frequency_commas(char *s)
{
	return;
	
	int8_t len = strlen(s);
	int8_t commas;
	int8_t c;
	char *p = s;

	/* Calculate commas number */
	commas = len / 3;
	if ((commas > 0) && !(len % 3))
		commas--;

	/* Process string */
	for (p += len, c = 0; (p > s) && (commas > 0); p--) {
		*(p + commas) = *p;
		/* don't count the string terminator */
		if (*p == '\0') continue;

		c++;
		if (c == 3) {
			commas--;
			*(p + commas) = '.';
			c = 0;
		}
	}
}

static void inline
process_event(void)
{


static uint32_t last_freq = 0;
static int32_t last_freq2 = 0;
uint32_t tmp;

	if (!event) return;

	char buffer[16];
	switch (event) {
		case DIAL_UP:
		if (!freq2selected)
		{
			frequency.hz += frequency.step;
			//si5351aSetFrequency1(frequency.hz);
			set_freq();
		}
		else
		{
			//frequency2.hz += frequency2.step;
			//si5351aSetFrequency2(frequency2.hz);
			if (eBfo == LSB) eBfo = USB;
			else if (eBfo == USB) eBfo = LSB;
		}

		break;

		case DIAL_DOWN:
		if (!freq2selected)
		{
			frequency.hz -= frequency.step;
			//si5351aSetFrequency1(frequency.hz);
			set_freq();
		}
		else
		{
			//frequency2.hz -= frequency2.step;
			//si5351aSetFrequency2(frequency2.hz);
			if (eBfo == LSB) eBfo = USB;
			else if (eBfo == USB) eBfo = LSB;
		}
		break;

		case PUSH_BTN:
		if (!freq2selected)
		{
			frequency.step *= 10;
			if (frequency.step >= 10000000)
			{
				frequency.step = 100;
				freq2selected = 1;
			}

			sprintf(buffer, "%lu    ", frequency.hz + frequency.step);
			frequency_commas(buffer);
			lcd_send_instr(LCD_INSTR_SET_DDRAM | LCD_FREQ_POSITION);
			lcd_print(buffer);

			_delay_ms(1000);

			sprintf(buffer, "%lu    ", frequency.hz);
			frequency_commas(buffer);
			lcd_send_instr(LCD_INSTR_SET_DDRAM | LCD_FREQ_POSITION);
			lcd_print(buffer);

		}
		else
		{
//			frequency2.step *= 10;
//			if (frequency2.step >= 10000000)
//			{
//				frequency2.step = 100;
//				freq2selected = 0;
//			}
//
//			sprintf(buffer, "%lu    ", frequency2.hz + frequency2.step);
//			lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
//			frequency_commas(buffer);
//			lcd_print(buffer);
//
//			_delay_ms(1000);
//
//			sprintf(buffer, "%lu    ", frequency2.hz);
//			lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
//			frequency_commas(buffer);
//			lcd_print(buffer);
			if (eBfo == LSB)
			{
				eBfo = USB;
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print("USB  ");
			}
			else if (eBfo == USB)
			{
				eBfo = LSB;
				lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
				lcd_print("LSB  ");
			}
			freq2selected = 0;
		}

		break;
	}
	
#if 0
	if (!freq2selected)
	{
		if (last_freq != frequency.hz) {
			sprintf(buffer, "%lu    ", frequency.hz);
			frequency_commas(buffer);
			lcd_send_instr(LCD_INSTR_SET_DDRAM | LCD_FREQ_POSITION);
			last_freq = frequency.hz;
			lcd_print(buffer);
		}
	}
	else
	{
		if (last_freq2 != frequency2.hz)
		{
			sprintf(buffer, "%lu    ", frequency2.hz);
			lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
			last_freq2 = frequency2.hz;
			frequency_commas(buffer);
			lcd_print(buffer);
		}
	}
#endif

	clear_events();
}


static void inline
frequency_init(void)
{
	frequency.hz = 3705000;
	frequency.step = 1000; // TODO: cannot be 50000 for example
	
	frequency2.hz = 9996300;//10000000;
	frequency2.step = 1000;
}

int main(void)
{
	char buffer[16];
	uint8_t a = 0;
	uint16_t zzz = 0;

// power the display on
DDRB = 0b00000001;
PORTB = 0b00000001;

// turn the backlight on (PD7)
//DDRD = 0b10000000;
//PORTD = 0b10000000;

//	spi_init();
//	extern void fnRFPlatformInit(void);
//	fnRFPlatformInit();
	lcd_init(LCD_SET_TWO_LINES);
	frequency_init();
	i2cInit();
	set_freq();
//	si5351aSetFrequency1(frequency.hz);
//	si5351aSetFrequency2(frequency2.hz);

//	dds_init(frequency.hz);
//	dac_init();
//	dac_write(15);
	adc_init();
	encoder_init();


//	sprintf(buffer, "%lu    ", frequency.hz);
//	frequency_commas(buffer);
//	lcd_send_instr(LCD_INSTR_SET_DDRAM | LCD_FREQ_POSITION);
//	lcd_print(buffer);
//
//	sprintf(buffer, "%lu    ", frequency2.hz);
//	lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
//	frequency_commas(buffer);
//	lcd_print(buffer);

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
		
		if (push)
		{
			event = PUSH_BTN;
			push = 0;
		}
		process_event();
	}

	return -1;
}

