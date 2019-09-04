#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <stdlib.h>
#include <compat/deprecated.h>
#include "keypad.h"

//  FIXME
#define F_CPU 16000000UL
#include <util/delay.h>

static volatile char tmp_row;
static volatile char tmp_col;
volatile unsigned char pin;


/* TODO TODO TODO TODO TODO
 * cand este si battery_management() in main loop driverul de keypad tinde
 * sa se blocheze daca apas din ce in ce mai repede pe o tasta si nu mai
 * intrerupe, deci nu mai functioneaza, ramane in sleep pana intrerupe
 * watchdogul.... please FIXME asap!!!
 */

static inline void
keypad_pullup_all_rows(void)
{
	/*
	 * Set ROW pins as INPUT in Data Direction Register
	 */
//	KEYPAD_DDR &= ~((1 << KEYPAD_ROW_ONE)	|
//					(1 << KEYPAD_ROW_TWO)	|
//					(1 << KEYPAD_ROW_THREE)	|
//					(1 << KEYPAD_ROW_FOUR));
	// FIXME
	cbi(KEYPAD_DDR, KEYPAD_ROW_ONE);
	cbi(KEYPAD_DDR, KEYPAD_ROW_TWO);
	cbi(KEYPAD_DDR, KEYPAD_ROW_THREE);
	cbi(KEYPAD_DDR, KEYPAD_ROW_FOUR);
	/*
	 * Enable the internal pull-up resistor for ROW pins
	 */
//	KEYPAD_PORT |= ((1 << KEYPAD_ROW_ONE)	|
//					(1 << KEYPAD_ROW_TWO)	|
//					(1 << KEYPAD_ROW_THREE)	|
//					(1 << KEYPAD_ROW_FOUR));

	// FIXME
	sbi(KEYPAD_PORT, KEYPAD_ROW_ONE);
	sbi(KEYPAD_PORT, KEYPAD_ROW_TWO);
	sbi(KEYPAD_PORT, KEYPAD_ROW_THREE);
	sbi(KEYPAD_PORT, KEYPAD_ROW_FOUR);
}


static inline void
keypad_pullup_all_cols(void)
{
	/*
	 * Set COL pins as INPUT in Data Direction Register
	 */
//	KEYPAD_DDR &= ~((1 << KEYPAD_COLUMN_ONE)	|
//					(1 << KEYPAD_COLUMN_TWO)	|
//					(1 << KEYPAD_COLUMN_THREE));

	// FIXME
	cbi(KEYPAD_DDR, KEYPAD_COLUMN_ONE);
	cbi(KEYPAD_DDR, KEYPAD_COLUMN_TWO);
	cbi(KEYPAD_DDR, KEYPAD_COLUMN_THREE);
	/*
	 * Enable the internal pull-up resistor for COL pins
	 */
//	KEYPAD_PORT |= ((1 << KEYPAD_COLUMN_ONE)	|
//					(1 << KEYPAD_COLUMN_TWO)	|
//					(1 << KEYPAD_COLUMN_THREE));

	// FIXME
	sbi(KEYPAD_PORT, KEYPAD_COLUMN_ONE);
	sbi(KEYPAD_PORT, KEYPAD_COLUMN_TWO);
	sbi(KEYPAD_PORT, KEYPAD_COLUMN_THREE);
}


static inline void
keypad_output_low_all_cols(void)
{
	/*
	 * First clear COL bits in PORT Register, which also means
	 * current sinking, thus preventing for accidentally output HIGH
	 */
//	KEYPAD_PORT &= ~((1 << KEYPAD_COLUMN_ONE)	|
//					(1 << KEYPAD_COLUMN_TWO)	|
//					(1 << KEYPAD_COLUMN_THREE));

	// FIXME
	cbi(KEYPAD_PORT, KEYPAD_COLUMN_ONE);
	cbi(KEYPAD_PORT, KEYPAD_COLUMN_TWO);
	cbi(KEYPAD_PORT, KEYPAD_COLUMN_THREE);

	// FIXME
	cbi(KEYPAD_DDR, KEYPAD_COLUMN_ONE);
	cbi(KEYPAD_DDR, KEYPAD_COLUMN_TWO);
	cbi(KEYPAD_DDR, KEYPAD_COLUMN_THREE);

	/*
	 * Set COL pins as output
	 */
//	KEYPAD_DDR |= ((1 << KEYPAD_COLUMN_ONE)		|
//					(1 << KEYPAD_COLUMN_TWO)	|
//					(1 << KEYPAD_COLUMN_THREE));

	// FIXME
	sbi(KEYPAD_DDR, KEYPAD_COLUMN_ONE);
	sbi(KEYPAD_DDR, KEYPAD_COLUMN_TWO);
	sbi(KEYPAD_DDR, KEYPAD_COLUMN_THREE);
}


static inline void
keypad_output_low_all_rows(void)
{
	/*
	 * First clear ROW bits in PORT Register, which also means
	 * current sinking, thus preventing for accidentally output HIGH
	 */
//	KEYPAD_PORT &= ~((1 << KEYPAD_ROW_ONE)	|
//					(1 << KEYPAD_ROW_TWO)	|
//					(1 << KEYPAD_ROW_THREE)	|
//					(1 << KEYPAD_ROW_FOUR));

	// FIXME
	cbi(KEYPAD_PORT, KEYPAD_ROW_ONE);
	cbi(KEYPAD_PORT, KEYPAD_ROW_TWO);
	cbi(KEYPAD_PORT, KEYPAD_ROW_THREE);
	cbi(KEYPAD_PORT, KEYPAD_ROW_FOUR);

	// FIXME
	cbi(KEYPAD_DDR, KEYPAD_ROW_ONE);
	cbi(KEYPAD_DDR, KEYPAD_ROW_TWO);
	cbi(KEYPAD_DDR, KEYPAD_ROW_THREE);
	cbi(KEYPAD_DDR, KEYPAD_ROW_FOUR);
	/*
	 * Set ROW pins as output
	 */
//	KEYPAD_DDR |= ((1 << KEYPAD_ROW_ONE)	|
//					(1 << KEYPAD_ROW_TWO)	|
//					(1 << KEYPAD_ROW_THREE)	|
//					(1 << KEYPAD_ROW_FOUR));

	// FIXME
	sbi(KEYPAD_DDR, KEYPAD_ROW_ONE);
	sbi(KEYPAD_DDR, KEYPAD_ROW_TWO);
	sbi(KEYPAD_DDR, KEYPAD_ROW_THREE);
	sbi(KEYPAD_DDR, KEYPAD_ROW_FOUR);
}


static inline void
keypad_disable_isr(void)
{

	cbi(PCICR, KEYPAD_PCIE);
}


static inline void
keypad_enable_isr(void)
{
	sbi(PCICR, KEYPAD_PCIE);
}


static void keypad_hardware_init(void)
{

	pin = 0;
	tmp_row = tmp_col = -1;
	KEYPAD_PCMSK = 0;

	keypad_pullup_all_rows();
	keypad_output_low_all_cols();

	/*
	 * Configure external interrupts only for keypad ROW pins
	 */
//	KEYPAD_PCMSK |= ((1 << KEYPAD_ROW_ONE)	|
//		 		  	(1 << KEYPAD_ROW_TWO)	|
//					(1 << KEYPAD_ROW_THREE)	|
//					(1 << KEYPAD_ROW_FOUR));

	// FIXME
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_ONE);
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_TWO);
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_THREE);
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_FOUR);

	/*
	 * Clear our pin in Pin Change Interrupt Flag Register
	 */
	sbi(PCIFR, PCIF1); // TODO: move PCIF1 to config. Test with SBI or by write logical 1

	/*
	 * We are ready to receive keypad events by now,
	 * therefore we can enable keypad interrupts
	 */
	keypad_enable_isr();
}


static int8_t _keypad_event = 0;
extern void keypad_init(void)
{
	/* TODO */
	_keypad_event = 0;

	keypad_hardware_init();
}



extern int8_t keypad_event(void)
{
	return _keypad_event;
}


static unsigned char keypad_process_key(void)
{
	uint8_t retval = 1;

	if (!pin) {

	// FIXME
_delay_ms(1);
		pin = KEYPAD_PIN;
	}


	/*
	 * Step 1 - Find row by looking which one is pulled low
	 */
	if (((pin >> KEYPAD_ROW_ONE) & 0x01) == 0) {
		tmp_row = KEYPAD_ROW_ONE;
	} else if (((pin >> KEYPAD_ROW_TWO) & 0x01) == 0) {
		tmp_row = KEYPAD_ROW_TWO;
	} else if (((pin >> KEYPAD_ROW_THREE) & 0x01) == 0) {
		tmp_row = KEYPAD_ROW_THREE;
	} else if (((pin >> KEYPAD_ROW_FOUR) & 0x01) == 0) {
		tmp_row = KEYPAD_ROW_FOUR;
	} else {
		/* no row detected
		 * just return, maybe the key was released
		 * or it is noise (FIXME key holding)
		 */
		retval = 0;
		goto finish;
	}

	/*
	 *
	 */
	keypad_output_low_all_rows();

	// FIXME
_delay_ms(1);


	keypad_pullup_all_cols();

	// FIXME
_delay_ms(1);

	/*
	 * Step 4 - Read pin
	 */
	pin = KEYPAD_PIN;

	if (((pin >> KEYPAD_COLUMN_ONE) & 0x01) == 0) {
		tmp_col = KEYPAD_COLUMN_ONE;
	} else if (((pin >> KEYPAD_COLUMN_TWO) & 0x01) == 0) {
		tmp_col = KEYPAD_COLUMN_TWO;
	} else if (((pin >> KEYPAD_COLUMN_THREE) & 0x01) == 0) {
		tmp_col = KEYPAD_COLUMN_THREE;
	} else {
		/* no col detected
		 * just return, maybe the key was released
		 * or it is noise (FIXME key holding)
		 */
		retval = 0;
		goto finish;
	}

finish:
	pin = 0;
	keypad_pullup_all_rows();
	keypad_output_low_all_cols();
	return retval;
}


static unsigned char keypad_read(void)
{
	if ((tmp_col == -1) || tmp_row == -1) {
		if (!keypad_process_key()) {
			return 0;
		}
	}

	static uint8_t c = 0;
	unsigned volatile char keypad_lookup_row = 0;
	unsigned volatile char keypad_lookup_col = 0;

	switch (tmp_row) {
		case KEYPAD_ROW_ONE:
			keypad_lookup_row = 1;
			break;

		case KEYPAD_ROW_TWO:
			keypad_lookup_row = 2;
			break;

		case KEYPAD_ROW_THREE:
			keypad_lookup_row = 3;
			break;

		case KEYPAD_ROW_FOUR:
			keypad_lookup_row = 4;
			break;

		default:
			keypad_lookup_row = 0;
	}

	switch (tmp_col) {
		case KEYPAD_COLUMN_ONE:
			keypad_lookup_col = 1;
			break;

		case KEYPAD_COLUMN_TWO:
			keypad_lookup_col = 2;
			break;

		case KEYPAD_COLUMN_THREE:
			keypad_lookup_col = 3;
			break;

		default:
			keypad_lookup_col = 0;
	}

	tmp_row = tmp_col = -1;

	if ((keypad_lookup_row <= 0) || (keypad_lookup_col <= 0)) {
		keypad_lookup_row = 0;
		keypad_lookup_col = 0;
		return 0;
	}

	c = keypad_table [keypad_lookup_row - 1] [keypad_lookup_col - 1];
	return c;
}


static int compare_fn(const void *a, const void* b)
{
	/*
	 * Return less than, qeual to, greather than ZERO, if 'a' is less than,
	 * qeual to, greather than 'b'
	 */
	return ((*(const uint8_t*) a) - (*(const uint8_t*) b));
}


/* TODO: carefully check this function for possible bugs */
static uint8_t find_significant_key(const uint8_t *buffer)
{
	uint8_t i;
	uint8_t findex;
	uint8_t filter_buffer[KEYPAD_BUFFER_SIZE][2] = {{0}};

	for (findex = 0, i = 0; i < KEYPAD_BUFFER_SIZE; i++) {

		if (buffer[i] == 0) {
			break;
		}

		/* hack for first element */
		if ((findex == 0) && (i == 0)) {
			filter_buffer[findex][0] = buffer[0];
		}

		if (filter_buffer[findex][0] != buffer[i]) {
			findex++;
			filter_buffer[findex][0] = buffer[i];
			filter_buffer[findex][1] = 1;
		} else {
			filter_buffer[findex][1]++;
		}
	}

	uint8_t saved_index = 0;
	for (i = 0; i < KEYPAD_BUFFER_SIZE; i++) {

		if (filter_buffer[i] == 0) {
			break;
		}

		/* compare by occurrence and save position */
		if (filter_buffer[i][1] > filter_buffer[saved_index][1]) {
			saved_index = i;
		}
	}

	return filter_buffer[saved_index][0];
}

extern uint8_t keypad_get(void)
{
	uint8_t keys[KEYPAD_BUFFER_SIZE] = {0};
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t c = 0;

	do {
		j++;
		c = keypad_read();
		if (c == 0) { /* TODO double check this. maybe it is safe to return */
			continue;
		} else {
			keys[i] = c;
			i++;
		}
	} while (j < KEYPAD_BUFFER_SIZE);

	if (i < KEYPAD_BUFFER_SIZE) {
		return 0;
	}

	qsort(keys, KEYPAD_BUFFER_SIZE, sizeof(uint8_t), compare_fn);

	return find_significant_key(keys);
}

/* TODO Naked ISR ?? */
ISR(KEYPAD_ISR)
{
	/*
	 * Read pin values and disable keypad interrupts
	 */
	pin = KEYPAD_PIN;
	keypad_disable_isr();

	_keypad_event = 1;
}

