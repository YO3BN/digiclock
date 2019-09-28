
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
#define LCD_COM_INSTR_DDR			DDRD
#define LCD_COM_INSTR_PORT			PORTD
#define LCD_COM_DATA_DDR			DDRD
#define LCD_COM_DATA_PORT			PORTD
#define LCD_COM_DATA_PIN			PIND



/* Instruction pins */
#define LCD_COM_PIN_RS				PD1
#define LCD_COM_PIN_RW				PD2
#define LCD_COM_PIN_E				PD3



/* Data pins. Only upper pins will
    be used in 4 bit communications. */
;#define LCD_COM_PIN_DB0				0
;#define LCD_COM_PIN_DB1				1
;#define LCD_COM_PIN_DB2				2
;#define LCD_COM_PIN_DB3				3
#define LCD_COM_PIN_DB4				PD4
#define LCD_COM_PIN_DB5				PD5
#define LCD_COM_PIN_DB6				PD6
#define LCD_COM_PIN_DB7				PD7

