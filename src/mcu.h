
//#if defined (__AVR_ATmega32__)

/* SPI Pinout */
#define	DDR_SPI		DDRB
#define	PORT_SPI	PORTB
#define	CS_DDS		PB4
#define CS_DAC		PB3
#define	MOSI		PB5
#define	SCK		PB7

#if 0
/* External Interrupts Pinout */
#define	DDR_INT0	DDRD
#define	DDR_INT1	DDRD
#define	DDR_INT2	DDRB
#define PORT_INT0	PORTD
#define PORT_INT1	PORTD
#define PORT_INT2	PORTB
#define PIN_INT0	PD2
#define PIN_INT1	PD3
#define PIN_INT2	PB2

#endif

//#endif /* __AVR_ATmega32__ */

