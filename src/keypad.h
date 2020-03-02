/*
 * keypad.h
 *
 *  Created on: Sep 6, 2019
 *      Author: Cristian Ionita
 */

#ifndef SRC_KEYPAD_H_
#define SRC_KEYPAD_H_

/* Keyboard row pin properties */
#define KEYPAD_ROW_ONE				PC7
#define KEYPAD_ROW_ONE_PORT			PORTC
#define KEYPAD_ROW_ONE_DDR			DDRC


/* Keyboard row pin properties */
#define KEYPAD_ROW_TWO				PC6
#define KEYPAD_ROW_TWO_PORT			PORTC
#define KEYPAD_ROW_TWO_DDR			DDRC

/* Keyboard row pin properties */
#define KEYPAD_ROW_THREE			PC5
#define KEYPAD_ROW_THREE_PORT		PORTC
#define KEYPAD_ROW_THREE_DDR		DDRC

/* Keyboard row pin properties */
#define KEYPAD_ROW_FOUR				PC4
#define KEYPAD_ROW_FOUR_PORT		PORTC
#define KEYPAD_ROW_FOUR_DDR			DDRC


/* Keyboard column pin properties */
#define KEYPAD_COL_ONE				PC3
#define KEYPAD_COL_ONE_PORT			PORTC
#define KEYPAD_COL_ONE_DDR			DDRC

/* Keyboard column pin properties */
#define KEYPAD_COL_TWO				PC2
#define KEYPAD_COL_TWO_PORT			PORTC
#define KEYPAD_COL_TWO_DDR			DDRC

/* Keyboard column pin properties */
#define KEYPAD_COL_THREE			PD6
#define KEYPAD_COL_THREE_PORT		PORTD
#define KEYPAD_COL_THREE_DDR		DDRD

/* Keyboard column pin properties */
#define KEYPAD_COL_FOUR				PD7
#define KEYPAD_COL_FOUR_PORT		PORTD
#define KEYPAD_COL_FOUR_DDR			DDRD

#define KEYPAD_ROW_PIN					PINC

/* Define the ISR handler */
#define KEYPAD_ISR_HANDLER			PCINT2_vect

#define KEYPAD_PCIE					PCIE2
#define KEYPAD_PCMSK				PCMSK2
#define KEYPAD_PCIF					PCIF2

/* Function prototypes */
void keypad_init(void);
void inline keypad_isr_enable(void);
void inline keypad_isr_disable(void);
char keypad_get_key(void);
static void keypad_pull_rows_up(void);
static void keypad_pull_cols_up(void);
static void keypad_set_rows_low(void);
static void keypad_set_cols_low(void);


#endif /* SRC_KEYPAD_H_ */
