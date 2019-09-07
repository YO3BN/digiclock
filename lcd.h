/*
 * About:	A software driver for Hitachi LCD Controller HD44780.
 *		Provide both 8 bit and 4 bit operation mode.
 *		Optimized for 16MHz cpufreq.
 *
 *		NatriX 2014
 */


#define LCD_INSTR_RESET			0b00110000
#define LCD_INSTR_CLEAR_DISPLAY		0b00000001	/*	0 0 0 0 0 0 0 1			*/
#define LCD_INSTR_RETURN_HOME		0b00000010	/*	0 0 0 0 0 0 1 -			*/
#define LCD_INSTR_ENTRY_MODE_SET	0b00000100	/*	0 0 0 0 0 1 I/D S		*/
#define LCD_INSTR_DISPLAY_CTRL		0b00001000	/*	0 0 0 0 1 D C B			*/
#define LCD_INSTR_CURSOR_SHIFT		0b00010000	/*	0 0 0 1 S/C R/L - -		*/
#define LCD_INSTR_FUNCTION_SET		0b00100000	/*	0 0 1 DL N F - -		*/
#define LCD_INSTR_SET_CGRAM		0b01000000	/*	0 1 ACG ACG ACG ACG ACG ACG	*/
#define LCD_INSTR_SET_DDRAM		0b10000000	/*	1 ADD ADD ADD ADD ADD ADD ADD	*/
/*
		DEFINE DISPLAY SETTINGS.

	These settings optionally can be sent together with instructions as bit logical OR.

       I/D =	0: Decrement		1: Increment
         S =				1: Accompanies display shift

	 D =	0: Display OFF		1: Display ON
	 C =	0: Cursor OFF		1: Cursor ON
	 B =	0: Cursor Blinks OFF	1: Cursor Blinks ON

       S/C =	0: Cursor move		1: Display shift
       R/L =	0: Shift to the left	1: Shift to the right

        DL =	0: 4 bits		1: 8 bits
         N =	0: 1 line		1: 2 lines
         F =	0: 5 x 8 dots		1: 5 x 10 dots			*/



#define LCD_SET_INCREMENT		0b00000010	/*	used in LCD_INSTR_ENTRY_MODE_SET	*/
#define LCD_SET_ADISPLAY_SHIFT		0b00000001	/*	used in LCD_INSTR_ENTRY_MODE_SET	*/
#define LCD_SET_DISPLAY_ON		0b00000100	/*	used in LCD_INSTR_DISPLAY_CTRL		*/
#define LCD_SET_CURSOR_ON		0b00000010	/*	used in LCD_INSTR_DISPLAY_CTRL		*/
#define LCD_SET_BLINK_ON		0b00000001	/*	used in LCD_INSTR_DISPLAY_CTRL		*/
#define LCD_SET_DISPLAY_SHIFT		0b00001000	/*	used in LCD_INSTR_CURSOR_SHIFT		*/
#define LCD_SET_RIGHT_SHIFT		0b00000100	/*	used in LCD_INSTR_CURSOR_SHIFT		*/
#define LCD_SET_8_BITS			0b00010000	/*	used in LCD_INSTR_FUNCTION_SET		*/
#define LCD_SET_TWO_LINES		0b00001000	/*	used in LCD_INSTR_FUNCTION_SET		*/
#define LCD_SET_5X10_FONT		0b00000100	/*	used in LCD_INSTR_FUNCTION_SET		*/


void lcd_init(unsigned char flags);
void lcd_send_data(unsigned char byte);
void lcd_send_instr(unsigned char byte);
void lcd_send_data_no_check(unsigned char byte);
void lcd_send_instr_no_check(unsigned char byte);
void lcd_send_first_nibble(unsigned char byte);
unsigned char lcd_read_data(void);
#ifdef LCD_COM_DUMMY_READ
void lcd_wait_if_busy(void);
#endif
void lcd_print(const char *p);


