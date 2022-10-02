#include <avr/io.h>
#include <avr/interrupt.h>

#include <semaphore.h>


/**
 * Since the ADC maximum value is hold on 10bits,
 * we can use some MSBs as an indicator of non-yet-read value.
 */
#define ADC_INVALID_VALUE	0x7000


static volatile int16_t adc_value = ADC_INVALID_VALUE;
static volatile int8_t busy = 0; // <- we can use a bit from ADCSRA, see the datasheet.
semaphore_t adc_sem;


ISR(ADC_vect)
{
	adc_value = ADCL;
	adc_value |= ((ADCH & 0x03) << 8);
	adc_value &= 0x03ff;

	busy = 0;

	sem_giveISR(&adc_sem);
}


extern void
adc_init(void)
{
	sem_init(&adc_sem); // <-- TODO: move this to an init function, but be careful cannot be initialized before the kernel starts.

	/* Set VREF 2.56V */
	ADMUX = (uint8_t)	\
		(1 << REFS1)	|
		(1 << REFS0);

	ADCSRA = (uint8_t)	\
		 (1 << ADIE)	|	/* ADC Interrupt	*/
		 (1 << ADEN)	|	/* Enable ADC Circuitry */
		 (0 << ADATE)	|	/* Auto-Trigger		*/
		 (0 << ADPS2)	|	/* ADC Clock Prescaler  */
		 (0 << ADPS1)	|	/* ADC Clock Prescaler  */
		 (0 << ADPS0);		/* ADC Clock Prescaler  */
	
	adc_value = ADC_INVALID_VALUE;
	busy = 0;
	//TODO: disable PIN into DIR0
}

extern void
adc_disable(void)
{
	ADCSRA = 0;
	ADMUX = 0;
	memset((void *) &adc_sem, 0, sizeof(semaphore_t));
	adc_value = ADC_INVALID_VALUE;
}


extern int16_t
adc_read(const uint8_t pin)
{
	if (busy) return -1;

	/* Measure signal for the following ADC pin */
	ADMUX &= 0b11100000; // <- clear the previous pin
	ADMUX |= (uint8_t) 0x1f & pin;

	/* Start conversion */
	ADCSRA |= (uint8_t) (1 << ADSC);

	adc_value = ADC_INVALID_VALUE;
	busy = 1;

	sem_take(&adc_sem, SEM_WAIT_FOREVER);

	/* Check if ADC was read yet */
	if ((adc_value & ADC_INVALID_VALUE) == ADC_INVALID_VALUE)
		return -1;

	/* return only LSB 10 bits */ 
	return adc_value & 0x03ff;
}
