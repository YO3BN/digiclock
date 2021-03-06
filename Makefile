MCU=atmega1284
F_CPU=8000000
VERSION=\"v0.1\"

BINDIR=bin
SRCDIR=src
INCDIR=.
LIBS=

CC=avr-gcc
CFLAGS=-Wall -g3 -O2 -pipe -gstabs -std=c99 -fdata-sections -ffunction-sections -Wl,--gc-sections,-u,vfprintf -lprintf_flt -lm

OBJCOPY=avr-objcopy
OBJOPT=-O ihex -j .text -j .data

all:
	$(CC) $(CFLAGS) -mmcu=$(MCU)			\
		-DF_CPU=$(F_CPU) -DVERSION=$(VERSION)	\
		-I$(INCDIR)				\
		-o $(BINDIR)/firmware.elf		\
		$(SRCDIR)/main.c			\
		$(SRCDIR)/encoder.c			\
		$(SRCDIR)/adc.c				\
		$(SRCDIR)/voltmeter.c			\
		$(SRCDIR)/keypad.c			\
		$(SRCDIR)/itu_table.c			\
		lcd/lcd.c					\
		Si5351_ex/si5351a.c			\
		Si5351_ex/i2c.c				\
		$(LIBS)

	$(OBJCOPY) $(OBJOPT) $(BINDIR)/firmware.elf $(BINDIR)/firmware.hex

clean:
	rm -f $(BINDIR)/*.a
	rm -f $(BINDIR)/*.o
	rm -f $(BINDIR)/*.hex
	rm -f $(BINDIR)/*.elf

