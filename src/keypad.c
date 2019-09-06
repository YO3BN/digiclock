/*
 * keypad.c
 *
 *  Created on: Sep 6, 2019
 *      Author: Cristian Ionita
 */


#include <avr/io.h>
#include <avr/interrupts.h>
#include "keypad.h"


volatile uint8_t keypad_event = 0;


/* Keypad interrupt handler only for rows */
ISR(KEYPAD_ISR_HANDLER)
{
	/* Disable ISR */
	keypad_isr_disable();
	keypad_event = 1;
	/* TODO Save the pressed row */
}


void keypad_init(void)
{
	keypad_event = 0;

	/* Pull rows up */
	keypad_pull_rows_up();

	/* Set columns output to low */
	keypad_set_cols_low();

	/* Enable interrupts */
	keypad_isr_enable();
}


void keypad_isr_enable(void)
{

}


void keypad_isr_disable(void)
{

}


void keypad_get_key(void)
{

}


static void keypad_pull_rows_up(void)
{
	/* Pull first row up */
	cbi(KEYPAD_ROW_ONE_DDR, KEYPAD_ROW_ONE);
	sbi(KEYPAD_ROW_ONE_PORT, KEYPAD_ROW_ONE);

	/* Pull second row up */
	cbi(KEYPAD_ROW_TWO_DDR, KEYPAD_ROW_TWO);
	sbi(KEYPAD_ROW_TWO_PORT, KEYPAD_ROW_TWO);

	/* Pull third row up */
	cbi(KEYPAD_ROW_THREE_DDR, KEYPAD_ROW_THREE);
	sbi(KEYPAD_ROW_THREE_PORT, KEYPAD_ROW_THREE);

	/* Pull fourth row up */
	cbi(KEYPAD_ROW_FOUR_DDR, KEYPAD_ROW_FOUR);
	sbi(KEYPAD_ROW_FOUR_PORT, KEYPAD_ROW_FOUR);
}


static void keypad_pull_cols_up(void)
{
	/* Pull first column up */
	cbi(KEYPAD_COL_ONE_DDR, KEYPAD_COL_ONE);
	sbi(KEYPAD_COL_ONE_PORT, KEYPAD_COL_ONE);

	/* Pull second column up */
	cbi(KEYPAD_COL_TWO_DDR, KEYPAD_COL_TWO);
	sbi(KEYPAD_COL_TWO_PORT, KEYPAD_COL_TWO);

	/* Pull third column up */
	cbi(KEYPAD_COL_THREE_DDR, KEYPAD_COL_THREE);
	sbi(KEYPAD_COL_THREE_PORT, KEYPAD_COL_THREE);

	/* Pull fourth column up */
	cbi(KEYPAD_COL_FOUR_DDR, KEYPAD_COL_FOUR);
	sbi(KEYPAD_COL_FOUR_PORT, KEYPAD_COL_FOUR);
}


static void keypad_set_rows_low(void)
{
	/* Set first row low */
	cbi(KEYPAD_ROW_ONE_PORT, KEYPAD_ROW_ONE);
	sbi(KEYPAD_ROW_ONE_DDR, KEYPAD_ROW_ONE);

	/* Set second row low */
	cbi(KEYPAD_ROW_TWO_PORT, KEYPAD_ROW_TWO);
	sbi(KEYPAD_ROW_TWO_DDR, KEYPAD_ROW_TWO);

	/* Set third row low */
	cbi(KEYPAD_ROW_THREE_PORT, KEYPAD_ROW_THREE);
	sbi(KEYPAD_ROW_THREE_DDR, KEYPAD_ROW_THREE);

	/* Set fourth row low */
	cbi(KEYPAD_ROW_FOUR_PORT, KEYPAD_ROW_FOUR);
	sbi(KEYPAD_ROW_FOUR_DDR, KEYPAD_ROW_FOUR);
}


static void keypad_set_cols_low(void)
{
	/* Set first column low */
	cbi(KEYPAD_COL_ONE_PORT, KEYPAD_COL_ONE);
	sbi(KEYPAD_COL_ONE_DDR, KEYPAD_COL_ONE);

	/* Set second column up */
	cbi(KEYPAD_COL_TWO_PORT, KEYPAD_COL_TWO);
	sbi(KEYPAD_COL_TWO_DDR, KEYPAD_COL_TWO);

	/* Set third column up */
	cbi(KEYPAD_COL_THREE_PORT, KEYPAD_COL_THREE);
	sbi(KEYPAD_COL_THREE_DDR, KEYPAD_COL_THREE);

	/* Set fourth column up */
	cbi(KEYPAD_COL_FOUR_PORT, KEYPAD_COL_FOUR);
	sbi(KEYPAD_COL_FOUR_DDR, KEYPAD_COL_FOUR);
}

