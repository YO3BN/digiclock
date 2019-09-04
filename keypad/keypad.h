#define KEYPAD_DDR				DDRB
#define KEYPAD_PORT				PORTB
#define KEYPAD_PIN				PINB

#define	KEYPAD_COLUMN_ONE		PB0
#define KEYPAD_COLUMN_TWO		PB1
#define KEYPAD_COLUMN_THREE		PB2

#define KEYPAD_ROW_ONE			PB3
#define KEYPAD_ROW_TWO			PB4
#define KEYPAD_ROW_THREE		PB5
#define KEYPAD_ROW_FOUR			PB6

#define KEYPAD_PCMSK			PCMSK1
#define KEYPAD_PCIE				PCIE1

#define KEYPAD_ISR				PCINT1_vect

#define KEYPAD_EVENT_ID			0x31

#define KEYPAD_KEY_PRESSED
#define KEYPAD_KEY_RELEASED
#define KEYPAD_KEY_NONE

#define KEYPAD_BUFFER_SIZE		15
#define KEYPAD_SAMPLES_PER_KEY	10


#if defined KEYPAD_LAYOUT_4X4
// TODO: Keypad layout 4x4 not yet supported
static uint8_t
keypad_table[4][4] = {
	
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

#else
static uint8_t
keypad_table[4][3] = {

	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};
#endif

/*
struct keypad_event {
	uint8_t	event_id;
	void	*data;
}keypad_event;
*/

extern int8_t keypad_event(void);


extern void
keypad_init(void);
//keypad_init(void (*uKernel_event_cb)(uint8_t), uint8_t, uint8_t flags);

extern uint8_t keypad_get(void);
