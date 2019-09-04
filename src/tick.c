#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>


static volatile uint8_t tick_overflow;


ISR(TIMER1_OVF_vect)
{
	tick_overflow = (uint8_t) 1;
}


void tick_init(void)
{
	TCCR1A = (uint8_t) \
		 (0 << COM1A1) |	/* Compare Output Mode A */
		 (0 << COM1A0) |	/* Compare Output Mode A */
		 (0 << COM1B1) |	/* Compare Output Mode B */
		 (0 << COM1B0) |	/* Compare Output Mode B */
		 (0 << FOC1A)  |	/* Force Output Compare A */
		 (0 << FOC1B)  |	/* Force Output Compare B */
		 (0 << WGM11)  |	/* Waveform Generation Mode */
		 (0 << WGM10);		/* Waveform Generation Mode */

	TCCR1B = (uint8_t) \
		 (0 << ICNC1) |		/* Input Capture Noise Canceler */
		 (0 << ICES1) |		/* Input Capture Edge Select */
		 (0 << WGM13) |		/* Waveform Generation Mode */
		 (0 << WGM12) |		/* Waveform Generation Mode */
		 (0 << CS12)  |		/* Clock Select */
		 (0 << CS11)  |		/* Clock Select */
		 (0 << CS10);		/* Clock Select */

	TIMSK = (uint8_t) \
		(0 << TICIE1) |		/* Timer/Cnt1 Input Capture Interrupt */
		(0 << OCIE1A) |		/* Timer/Cnt1 Output Compare Match */
		(0 << OCIE1B) |		/* Timer/Cnt1 Output Compare Match */
		(1 << TOIE1);		/* Timer/Cnt1 Overflow Interrupt Enable */

	/* Reset the values */
	tick_overflow = (uint8_t) 0;
	TCNT1 = (uint16_t) 0;
}


static void timer_start(void)
{
	/* Start Timer/Counter1 by choosing the prescaler */
	TCCR1B |= (uint8_t) \
		  (0 << CS12) |
		  (1 << CS11) |
		  (0 << CS10);

	/* Reset the values */
	tick_overflow = (uint8_t) 0;
	TCNT1 = (uint16_t) 0;
}


static void timer_stop(void)
{
	/* Clearing Clock will disable timer/counter1 */
	TCCR1B &= (uint8_t) ~((1 << CS12) | (1 << CS11) | (1 << CS10));
}


uint8_t tick(void)
{
	cli();

	/* Save MCUCR, it will be restored back later */
	uint8_t mcucr = MCUCR;
tick_overflow = (uint8_t) 0;

	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	sei();
	timer_start();
	sleep_cpu();
	timer_stop();

	/*
	 * We don't need to use sleep_disable(),
	 * just restoring the MCUCR register.
	 */
	MCUCR = mcucr;

	/* Return TRUE when waking the CPU up by TIMER1_OVF */
	//return tick_overflow;
	if (tick_overflow) return 1;
	else return 0;
}


