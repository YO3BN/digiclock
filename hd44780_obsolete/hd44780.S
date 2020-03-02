
/*
 * About:	A software driver for Hitachi LCD Controller HD44780.
 *		Provide both 8 bit and 4 bit operation mode.
 *		Optimized for 16MHz cpufreq.
 *
 *		NatriX 2014
 */

#ifndef __SFR_OFFSET
#	define __SFR_OFFSET 0
#endif

#ifndef F_CPU
#	warning "F_CPU NOT DEFINED! This should be same as in your project."
#	warning "Please, see Makefile."
#endif

#include <avr/io.h>

#include "hd44780.h"
#include "mcu.h"

/*      GCC Register Layout     */

/*
 * TEMPORARY REGISTER.
 * Is used as scratch register that need not to be restored after
 * its usage. It must be saved and restored in interrupt service
 * routine's (ISR) prologue and epilogue.
 */
#define __tmp_reg__ r0

/*
 * ZERO REGISTER.
 *
 * Always contains zero. During an insn the content might be destroyed,
 * e.g. by a MUL instruction that uses R0/R1 as implicit output register.
 * If an insn destroys R1, the insn must restore R1 to zero afterwards.
 * This register must be saved in ISR prologues and must then be set to
 * zero because R1 might contain values other than zero.
 * The ISR epilogue restores the value.
 */
#define __zero_reg__ r1

#define	__r_tmp1 r18

.global lcd_init
.global lcd_write_string
.global	lcd_send_instr
.global lcd_send_data
.global lcd_wait_if_busy
#ifndef	LCD_COM_DUMMY_READ
.global lcd_get_address
.global lcd_read_data
#endif	/* LCD_COM_DUMMY_READ	*/

/*
 * void lcd_init(uint8_t flags);
 * Reset the LCD controller by instruction.
 */
lcd_init:

	/*	Make sure they are 0V or high-Z	*/
	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RS
	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RW
	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_E

	/*	Set them to output low		*/
	sbi	LCD_COM_INSTR_DDR, LCD_COM_PIN_RS
	sbi	LCD_COM_INSTR_DDR, LCD_COM_PIN_RW
	sbi	LCD_COM_INSTR_DDR, LCD_COM_PIN_E

#ifdef	LCD_COM_4_BIT_MODE
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB4
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB5
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB6
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB7

	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB4
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB5
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB6
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB7
#else
	out	LCD_COM_DATA_PORT, __zero_reg__
	ldi	__r_tmp1, 0xFF
	out	LCD_COM_DATA_DDR, __r_tmp1
#endif	/* #ifdef  LCD_COM_4_BIT_MODE	*/

	/* save arg flags to stack	*/
	push	r24

	/* these values must not be changed
		very soon. */
	;ldi	rARG1, 0x30 ;FIXME
	clr	r25

/* HD44780 Init by instruction */
/* Step 1 */
	ldi	r24, 60
	rcall	delay_ms

/* Step 2 */
	ldi	r24, 0x30
#ifdef	LCD_COM_4_BIT_MODE
	rcall	lcd_send_first_nibble
#else
	rcall	lcd_send_instr_no_check
#endif  /* LCD_COM_4_BIT_MODE   */

/* Step 3 */
	ldi	r24, 10
	rcall	delay_ms
	ldi	r24, 0x30
#ifdef	LCD_COM_4_BIT_MODE
	rcall	lcd_send_first_nibble
#else
	rcall	lcd_send_instr_no_check
#endif	/* LCD_COM_4_BIT_MODE	*/

/* Step 4 */
	rcall	delay_1ms
	ldi	r24, 0x30; FIXME needed?
#ifdef	LCD_COM_4_BIT_MODE
	rcall	lcd_send_first_nibble
#else
	rcall	lcd_send_instr_no_check
#endif  /* LCD_COM_4_BIT_MODE   */

/* Step 5 */
	ldi	r24, 10; FIXME remove?
	rcall	delay_ms

/* Step 6 - Setting the display lines and fonts. These cannot
	be changed after this point, so we gonna make them
	accesible via arguments rARG1, or default as one line and 5x8 font. */
#ifdef	LCD_COM_4_BIT_MODE
/* Step 6' -- extrastep for 4 bit mode */
	ldi	r24, 0x20
	rcall	lcd_send_first_nibble
	ldi	r24, 10
	rcall	delay_ms

	/* get the saved flags	*/
	pop	r24
	ori	r24, (LCD_INSTR_FUNCTION_SET)
	rcall	lcd_send_instr_no_check
#else
        /* get the saved flags	*/
        pop	r24

	ori	r24, (LCD_INSTR_FUNCTION_SET | LCD_SET_8_BITS)
	rcall	lcd_send_instr_no_check
#endif	/* #ifdef  LCD_COM_4_BIT_MODE	*/

/* Step 7 */
	ldi	r24, 10
	rcall	delay_ms
	ldi	r24, LCD_INSTR_DISPLAY_CTRL
	rcall	lcd_send_instr_no_check

/* Step 9 */
	ldi	r24, 10
	rcall	delay_ms
	ldi	r24, LCD_INSTR_CLEAR_DISPLAY
	rcall	lcd_send_instr_no_check

/* Step 10 */
	ldi	r24, 10
	rcall	delay_ms
	ldi	r24, (LCD_INSTR_ENTRY_MODE_SET | LCD_SET_INCREMENT)
	rcall	lcd_send_instr_no_check
/* Done init the LCD by instruction	*/
/* Turn display on */
	ldi	r24, 10
	rcall	delay_ms
	ldi	r24, (LCD_INSTR_DISPLAY_CTRL | LCD_SET_DISPLAY_ON)
	rcall	lcd_send_instr_no_check
	ret



/*
 * Multiple functions used to write data/instructions to
 * LCD controller.
 * TODO: save T flag ?
 */
#ifdef	LCD_COM_4_BIT_MODE
/*
 * void lcd_send_first_nibble(unsigned char byte);
 */
lcd_send_first_nibble:
	set
	rjmp again4
#endif	/* LCD_COM_4_BIT_MODE	*/
/*
 * void lcd_send_data(unsigned char byte);
 */
lcd_send_data:
	/*	lcd_wait_if_busy will modify our r24 register
		which is uchar byte arg. so must be saved	*/
	mov	__tmp_reg__, r24
	rcall	lcd_wait_if_busy
	mov	r24, __tmp_reg__
	rjmp	lcd_send_data_no_check
/*
 * void lcd_send_instr(unsigned char byte);
 */
lcd_send_instr:
	/*	lcd_wait_if_busy will modify our r24 register
		which is uchar byte arg. so must be saved	*/
	mov	__tmp_reg__, r24
	rcall	lcd_wait_if_busy
	mov	r24, __tmp_reg__
/*
 * void lcd_send_instr_no_check(unsigned char byte);
 */
lcd_send_instr_no_check:
	rjmp	1f
/*
 * void lcd_send_data_no_check(unsigned char byte);
 */
lcd_send_data_no_check:
	sbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RS
#ifdef	LCD_COM_4_BIT_MODE
1:	clt	/* T flag used in 4bit mode */
	/* 	write data 4	*/
again4:	sbrc	r24, 7
	sbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB7
	sbrc	r24, 6
	sbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB6
	sbrc	r24, 5
	sbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB5
	sbrc	r24, 4
	sbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB4
#else
	/*	write data 8	*/
1:	out	LCD_COM_DATA_PORT, r24
#endif	/* LCD_COM_4_BIT_MODE	*/

	/*	set E on	*/
	sbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_E
	nop
	nop
#if	F_CPU > 4000000
	nop
	nop
	nop
#endif	/* F_CPU > 4000000	*/

	/*	turn E off	*/
	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_E
#ifdef	LCD_COM_4_BIT_MODE
	/*	erase data 4	*/
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB7
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB6
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB5
	cbi	LCD_COM_DATA_PORT, LCD_COM_PIN_DB4

	brts	end4
	/*	this sould happen once	*/
	swap	r24
	set
	rjmp	again4
#else
	/*	erase data 8	*/
	out	LCD_COM_DATA_PORT, __zero_reg__
#endif	/*	LCD_COM_4_BIT_MOD	*/
	/*	turn RS off	*/
end4:	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RS
	ret



#ifdef LCD_COM_DUMMY_READ
/*
 *	void lcd_wait_if_busy(void);
 */
lcd_wait_if_busy:
	ldi	r24, 5
	rcall	delay_ms
	ret
#else
/* FIXME: este ok cand folosesc comunicatie de 4 biti cu pinii 4-7
 din mcu indiferent de ordine, dar cand folosesc pini de la 3 la 6
 am o problema in functia de read, ramane hangedup cu RW pe on si E
 intermitent, probabil ceva gen data reordering nu este facuta cum
 trebuie */


/*
 * unsigned char lcd_read_data(void);
 * Reads a byte from LCD cursor.
 */
lcd_read_data:
	rcall	lcd_wait_if_busy
	sbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RS

/*
 * unsigned char lcd_wait_if_busy( void );
 * Return when busy flag is off and it also reads
 * the cursor address and store it in the rRET register
 * as returned value.
 */
lcd_wait_if_busy:
lcd_get_address:	/*	just an alias	*/
#ifdef	LCD_COM_4_BIT_MODE
	/* set DATA PORT to input 4 bit mode	*/
	cbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB4
	cbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB5
	cbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB6
	cbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB7
#else	/* LCD_COM_4_BIT_MODE	*/
	/* set DATA PORT to input 8 bit mode	*/
	out	LCD_COM_DATA_DDR, __zero_reg__
#endif	/* LCD_COM_4_BIT_MODE	*/
	nop	/* nop for sync	*/
	/*	set RW on	*/
	sbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RW

check_again:
#ifdef	LCD_COM_4_BIT_MODE
	/* clear T flag and return
		retister	*/
	clt
	eor	r24, r24
read_2nd_nibble:
#endif	/* LCD_COM_4_BIT_MODE	*/

	/*	set E on	*/
	sbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_E
	nop
	nop
	nop
	/* Using 16Mhz, data from HD44780 should be
		available. Just NOP for pin sync */
	nop

#ifdef	LCD_COM_4_BIT_MODE
	/* read data from LCD 4 bit mode */
	sbic	LCD_COM_DATA_PIN, LCD_COM_PIN_DB7
	sbr	r24, 0b00001000
	sbic	LCD_COM_DATA_PIN, LCD_COM_PIN_DB6
	sbr	r24, 0b00000100
	sbic	LCD_COM_DATA_PIN, LCD_COM_PIN_DB5
	sbr	r24, 0b00000010
	sbic	LCD_COM_DATA_PIN, LCD_COM_PIN_DB4
	sbr	r24, 0b00000001
#else	/* LCD_COM_4_BIT_MODE	*/
	/* read data from LCD 8 bit mode */
	in	r24, LCD_COM_DATA_PIN
#endif	/* LCD_COM_4_BIT_MODE	*/

	/*	turn E off	*/
	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_E

#ifdef	LCD_COM_4_BIT_MODE
	/* read second nibble	*/
	brts	done_4bit
	nop	/* little delay. E must be off >250ns */
	swap	r24
	set
	rjmp	read_2nd_nibble
done_4bit:
#endif  /* LCD_COM_4_BIT_MODE	*/

	/* lcd_read_data or lcd_wait_if_busy ?	*/
	sbic	LCD_COM_INSTR_PORT, LCD_COM_PIN_RS
	rjmp	1f

	/* check the busy bit	*/
	sbrc	r24, LCD_COM_PIN_DB7
	rjmp	check_again

	/*	turn RW off	*/
1:	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RW
	/*	turn RS off	*/
	cbi	LCD_COM_INSTR_PORT, LCD_COM_PIN_RS

#ifdef	LCD_COM_4_BIT_MODE
	/* set pins again as output 4 bit mode */
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB4
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB5
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB6
	sbi	LCD_COM_DATA_DDR, LCD_COM_PIN_DB7
#else	/* LCD_COM_4_BIT_MODE	*/
	/* set pins again as output 8 bit mode */
	ldi	__r_tmp1, 0xFF
	out	LCD_COM_DATA_DDR, __r_tmp1
#endif	/* LCD_COM_4_BIT_MODE	*/
	ret
#endif	/* LCD_COM_DUMMY_READ	*/

