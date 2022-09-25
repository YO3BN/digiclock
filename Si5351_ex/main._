#include "si5351a.h"
#include "i2c.h"

#include "util/delay.h"

int main(void)
{
	i2cInit();


	while (1)
	{
		si5351aSetFrequency1(2000000);
		si5351aSetFrequency2(3000000);
		_delay_ms(5000);

		si5351aSetFrequency1(4000000);
		si5351aSetFrequency2(5000000);
		_delay_ms(5000);

		si5351aSetFrequency1(6000000);
		si5351aSetFrequency2(7000000);
		_delay_ms(5000);

		si5351aSetFrequency1(8000000);
		si5351aSetFrequency2(9000000);
		_delay_ms(5000);

		si5351aSetFrequency1(10000000);
		si5351aSetFrequency2(11000000);
		_delay_ms(5000);

		si5351aSetFrequency1(14000000);
		si5351aSetFrequency2(16000000);
		_delay_ms(5000);

		si5351aSetFrequency1(18000000);
		_delay_ms(5000);

		si5351aSetFrequency1(24000000);
		_delay_ms(5000);

		si5351aSetFrequency1(27000000);
		_delay_ms(5000);

		si5351aSetFrequency1(30000000);
		_delay_ms(5000);
	}

	return 0;
}

