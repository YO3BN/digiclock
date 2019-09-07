/*
 * keypad.c
 *
 *  Created on: Sep 6, 2019
 *      Author: Cristian Ionita
 */


#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <compat/deprecated.h>

#include "keypad.h"

char keypad_4x4map[4][4] = 
{
		{ '1', '2', '3', 'A', },
		{ '4', '5', '6', 'B', },
		{ '7', '8', '9', 'C', },
		{ '*', '0', '#', 'D', },
};

volatile uint8_t keypad_event = 0;
static volatile uint8_t keypad_pin = 0;

/* Keypad interrupt handler only for rows */
ISR(KEYPAD_ISR_HANDLER)
{
	/* Disable ISR */
	keypad_isr_disable();
	keypad_event = 1;
	/* Save the pressed row */
	keypad_pin = KEYPAD_ROW_PIN;
}


void keypad_init(void)
{
	keypad_event = 0;
	keypad_pin = 0;

	keypad_isr_disable();
	keypad_pull_rows_up();
	keypad_set_cols_low();
	keypad_isr_enable();
}


void inline keypad_isr_enable(void)
{
	/* Enable interrupts only for row pins */
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_ONE);
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_TWO);
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_THREE);
	sbi(KEYPAD_PCMSK, KEYPAD_ROW_FOUR);
	
	/* Enable port interrupt */
	sbi(PCICR, KEYPAD_PCIE);
}


void inline keypad_isr_disable(void)
{
	/* First, clear the interrupt flag. */
	sbi(PCIFR, KEYPAD_PCIF);

	/* Disable port interrupt */
	cbi(PCICR, KEYPAD_PCIE);

	/* Disable interrupts only for row pins */
	cbi(KEYPAD_PCMSK, KEYPAD_ROW_ONE);
	cbi(KEYPAD_PCMSK, KEYPAD_ROW_TWO);
	cbi(KEYPAD_PCMSK, KEYPAD_ROW_THREE);
	cbi(KEYPAD_PCMSK, KEYPAD_ROW_FOUR);
}


char keypad_get_key(void)
{
	char row = 0;
	char col = 0;
	char key = 0;

	/* Do we really have a keypad event ? */
	if (!keypad_event && !keypad_pin)
	{
		return 0;
	}

	/*
	 * Here we are assuming that we have received the interrupt,
	 * therefore we have to find out the row pressed.
	 */
	switch (keypad_pin & 0xf0)
	{
	case 0b01110000:
		row = 1;
		break;
		
	case 0b10110000:
		row = 2;
		break;
		
	case 0b11010000:
		row = 3;
		break;
		
	case 0b11100000:
		row = 4;
		break;
	}
	
	/*
	 * Here we know the row, therefore we have to pull up cols
	 * and set rows to low.
	 */
	keypad_pull_cols_up();
	keypad_set_rows_low();
	
	// FIXME do busyless sleep.
	// We need this sleep in order to settle the voltage levels
	// within cable wires due capacitance effect.
	_delay_ms(5);
	
	/* Read columns */
	// FIXME Hardware dependent. please fix it in hardware first.
	keypad_pin = KEYPAD_ROW_PIN & 0b00001100;
	keypad_pin |= (PIND & 0x80) >> 6;
	keypad_pin |= (PIND & 0x08) >> 3;
	switch (keypad_pin & 0x0f)
	{
	case 0b00000111:
		col = 1;
		break;
		
	case 0b00001011:
		col = 2;
		break;
		
	case 0b00001101:
		col = 3;
		break;
		
	case 0b00001110:
		col = 4;
		break;
	}

	/* Array lookup */
	//FIXME hardware fix rows vs cols.
	key = keypad_4x4map[col -1][row - 1];
	
	/* Re-initialize the keypad */
	keypad_init();


	return (row && col) ? key : 0;
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

