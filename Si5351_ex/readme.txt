This page contains some example code for the Si5351A synthesiser module chip.
There are a couple of other Si5351A libraries on the internet. However they
are either incomplete (don't go all the way from specifying a frequency, to
generating and setting the configuration registers), or quite over-complex.
Some are designed to cater for all the variants of the Si5351 (A/B/C) and all
the features. There's a Linux Si5351 driver but the code is very confusing and
anyway it does not seem well-suited to a limited-space microcontroller
environment.

So, with these C examples I have tried to keep it very simple, just to show
what needs to be done to get some output out of the Si5351A (only), and only
the 10-pin MSOP version with 3 outputs. From here, you can build on it, piece
by piece, to customise it for your application. So these examples generate a
10MHz signal on the Clk0 output, that's all. You can choose any frequency in
the range 1-150MHz, in the function call.

For frequencies under 1MHz, you need to employ the final divider stage in the
Si5351A, which can divide by powers of 2 from 1 to 128. So for example, if you
want 100kHz, then one way to do it is to configure the chip for a frequency of
1.6MHz, then configure the final division stage as divide-by-16 to get to
100kHz output. Using the division ratios, according to the datasheet, the
Si5351A can then generate low output frequencies from 8kHz to 1MHz.

For frequencies above 150MHz, and to the maximum specified frequency of 160MHz
according to the datasheet, some special configuration is required. I haven't
attempted to tackle that in this example code.
Useful documents

The Si5351A datasheet:
http://www.silabs.com/Support%20Documents/TechnicalDocs/Si5351-B.pdf
Manually generating an Si5351A register map:
http://www.silabs.com/Support%20Documents/TechnicalDocs/AN619.pdf

Application note AN619 "Manually generating an Si5351A register map" is much
more useful than the actual Si5351A datasheet. I refer to it often. There are
quite a lot of mistakes and confusions in both the datasheet and AN619 but all
of the information is actually in there and it can be decyphered.
Some theoretical discussion

The Si5351A is confugured using two signals, an I2C bus protocol. The QRP Labs
Si5351A module kit includes all the necessary pullups and level conversion to
run on a 5V microcontroller system. You only need to connect these two wires
to the AVR processor. It is important to understand the basic synthesis
mechanism of the Si5351A. There are really three stages to it:

1) A digital PLL multiplies the 27MHz crystal oscillator frequency up, to an
internal frequency; somewhere (that you configure) that must be in the range
600-900MHz. The multiplication factor is fractional, not an integer. The code
must configure all the registers that specify the integer part, and the
numerator/denominator of the fractional part. The integer part must be in the
range 15..90; the numerator 0..1048575 (20 bits), and the denominator
1..1048575 (also 20 bits). There are TWO PLLs available, PLL A and PLL B.

2) A divider stage divides this internal UHF frequency back down to the
desired output frequency. The division ratio is also fractional with 20-bit
fractional parts. However for best jitter performance the datasheet recommends
using even integers for the division factor. There are THREE "multisynth"
divider stages, one for each of the three Clk outputs of the Si5351A (other
variants of the Si5351A have 8 outputs, and various other input arrangements;
we aren't considering those here, we're keeping it simple, remember). Each
Multisynth divider can be fed by either of the two PLL outputs, you choose.

3) Finally there is also a final division stage for each output, which divides
by a power-of-2 factor of 1..128, this is needed for low frequency outputs in
the range 8kHz to 1MHz.

Since there are 2 PLLs and 3 outputs, and each output "multisynth" divider can
be fed from either of the two PLLs, if you want to set this up for multiple
outputs you have to do some tough thinking. If you stick to the "even integer"
divider, you don't really have THREE independent flexible output frequencies,
you have only two. Because two of those outputs are going to have to feed off
the same PLL. If there is not a convenient integral relationship between two
of the output frequencies, then for one of them you have to abandon the "even
integer division" rule for low jitter.

Anyway, I'm just saying. All of these things are considerations to take into
account. But not part of my simple simple example here.
Simple example code

The example code is written in C for AVR microcontrollers, I use the GCC
compiler in AVR Studio. It should be easy to use this on any AVR, or on the
Arduino platform. Only minor changes would be needed for other
microcontrollers.

There are TWO examples here. The first example allows you to use ANY I/O pins
of the processor to bit-bang as I2C. That gives a lot of flexibility, if you
happen to be using I/O pins other than the AVR's onboard I2C peripheral (they
call it the Two Wire interface in the Atmel datasheets and application notes),
or if you are using an AVR device that doesn't have an I2C peripheral onboard:
not all support I2C. In this example we use a very nice assembly language
library by Peter Fleury to take care of the I2C bit-banging. It works
perfectly!

The second example uses the AVR's internal I2C peripheral (which they call the
Two Wire interface, TWI, in the datasheets and application notes). That
assumes that in your application, your chosen AVR does have a TWI peripheral,
and that you aren't using those particular I2C pins of the processor for
something else, so you can afford to dedicate them to I2C purposes. This is
the ideal situation.
