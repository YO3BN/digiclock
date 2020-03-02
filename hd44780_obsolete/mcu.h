
/*
 * About:	A software driver for Hitachi LCD Controller HD44780.
 *		Provide both 8 bit and 4 bit operation mode.
 *		Optimized for 16MHz cpufreq.
 *
 *		NatriX 2014
 */



/*
 */
;#define	LCD_COM_DUMMY_READ
#define	LCD_COM_4_BIT_MODE



/*
 *	Communications pins and ports must be manually set.
 */
#define LCD_COM_INSTR_DDR			DDRB
#define LCD_COM_INSTR_PORT			PORTB
#define LCD_COM_DATA_DDR			DDRB
#define LCD_COM_DATA_PORT			PORTB
#define LCD_COM_DATA_PIN			PINB



/* Instruction pins */
#define LCD_COM_PIN_RS				PB0
#define LCD_COM_PIN_RW				PB1
#define LCD_COM_PIN_E				PB2



/* Data pins. Only upper pins will
    be used in 4 bit communications. */
;#define LCD_COM_PIN_DB0				0
;#define LCD_COM_PIN_DB1				1
;#define LCD_COM_PIN_DB2				2
;#define LCD_COM_PIN_DB3				3
#define LCD_COM_PIN_DB4				PB3
#define LCD_COM_PIN_DB5				PB4
#define LCD_COM_PIN_DB6				PB5
#define LCD_COM_PIN_DB7				PB6
