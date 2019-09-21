/*
 * itu_table.c
 *
 *  Created on: Sep 7, 2019
 *      Author: Cristian Ionita
 */


#include <string.h>
#include <stdint.h>
#include "itu_table.h"

#include "lcd.h"


/*
 * Closer implementation of band titles from ERC REPORT 25 document.
 * It does not support floating frequency.
 * It does not take into account USB/LSB mode.
 */
static ITU_Band ItuBand[] =
{
		{0,		0},
		{8,		BAND_MET},
		{9,		BAND_MET | BAND_NAV},
		{11,	BAND_NAV},
		{14,	BAND_FIX | BAND_MAR},
		{20,	BAND_STS},
		{21,	BAND_FIX | BAND_MAR},
		{70,	BAND_NAV},
		{72,	BAND_FIX | BAND_MAR | BAND_NAV},
		{84,	BAND_NAV},
		{86,	BAND_FIX | BAND_MAR | BAND_NAV},
		{90,	BAND_FIX | BAND_NAV},
		{110,	BAND_FIX | BAND_MAR | BAND_NAV},
		{112,	BAND_NAV},
		{115,	BAND_FIX | BAND_MAR | BAND_NAV},
		{126,	BAND_NAV},
		{129,	BAND_FIX | BAND_MAR | BAND_NAV},
		{130,	BAND_FIX | BAND_MAR},
		{135,	BAND_FIX | BAND_MAR | BAND_AMT},
		{138,	BAND_FIX | BAND_MAR},
		{148,	BAND_BCT},
		{255,	BAND_AER | BAND_BCT | BAND_NAV},
		{283,	BAND_AER | BAND_MAR | BAND_NAV},
		{325,	BAND_AER | BAND_NAV},
		{405,	BAND_NAV},
		{415,	BAND_AER | BAND_MAR | BAND_NAV},
		{472,	BAND_AER | BAND_MAR | BAND_NAV | BAND_AMT},
		{479,	BAND_AER | BAND_MAR | BAND_NAV},
		{495,	BAND_MAR},
		{505,	BAND_AER | BAND_MAR | BAND_NAV},
		{527,	BAND_BCT},
		{1606,	BAND_FIX | BAND_MOB | BAND_MAR},
		{1625,	BAND_LOC},
		{1635,	BAND_FIX | BAND_MOB | BAND_MAR},
		{1800,	BAND_LOC},
		{1810,	BAND_AMT},
		{1850,	BAND_FIX | BAND_MOB | BAND_AMT},
		{2000,	BAND_FIX | BAND_MOB},
		{2025,	BAND_FIX | BAND_MOB | BAND_MET},
		{2045,	BAND_FIX | BAND_MOB | BAND_MAR},
		{2160,	BAND_LOC},
		{2170,	BAND_MAR},
		{2173,	BAND_MOB | BAND_DSC},
		{2191,	BAND_MAR},
		{2194,	BAND_FIX | BAND_MOB},
		{2300,	BAND_FIX | BAND_MOB | BAND_BCT},
		{2498,	BAND_STS},
		{2501,	BAND_STS | BAND_SRE},
		{2502,	BAND_FIX | BAND_MOB},
		{2625,	BAND_MAR | BAND_NAV},
		{2650,	BAND_FIX | BAND_MOB},
		{2850,	BAND_AER},
		{3155,	BAND_FIX | BAND_MOB},
		{3200,	BAND_FIX | BAND_MOB | BAND_BCT},
		{3400,	BAND_AER},
		{3500,	BAND_FIX | BAND_MOB | BAND_AMT},
		{3800,	BAND_FIX | BAND_AER | BAND_MOB},
		{3900,	BAND_AER},
		{3950,	BAND_FIX | BAND_BCT},
		{4000,	BAND_FIX | BAND_MAR},
		{4063,	BAND_MAR | BAND_DSC},
		{4438,	BAND_FIX | BAND_MOB | BAND_LOC},
		{4488,	BAND_FIX | BAND_MOB},
		{4650,	BAND_AER},
		{4750,	BAND_AER | BAND_BCT | BAND_FIX | BAND_MOB},
		{4850,	BAND_BCT | BAND_FIX | BAND_MOB},
		{4995,	BAND_STS},
		{5003,	BAND_STS | BAND_SRE},
		{5005,	BAND_FIX | BAND_BCT},
		{5060,	BAND_FIX | BAND_MOB},
		{5250,	BAND_FIX | BAND_MOB | BAND_LOC},
		{5275,	BAND_FIX | BAND_MOB},
		{5352,	BAND_FIX | BAND_MOB | BAND_AMT},
		{5366,	BAND_FIX | BAND_MOB},
		{5450,	BAND_FIX | BAND_MOB | BAND_AER},
		{5480,	BAND_AER},
		{5730,	BAND_FIX | BAND_MOB},
		{5900,	BAND_BCT},
		{6200,	BAND_MAR | BAND_DSC},
		{6525,	BAND_AER},
		{6765,	BAND_FIX | BAND_MOB},
		{7000,	BAND_AMT},
		{7200,	BAND_BCT},
		{7450,	BAND_FIX | BAND_MOB},
		{8100,	BAND_FIX | BAND_MAR},
		{8195,	BAND_MAR | BAND_DSC},
		{8815,	BAND_AER},
		{9040,	BAND_FIX},
		{9305,	BAND_FIX | BAND_LOC},
		{9355,	BAND_FIX},
		{9400,	BAND_BCT},
		{9900,	BAND_FIX},
		{9995,	BAND_STS},
		{10003,	BAND_STS | BAND_SRE},
		{10005,	BAND_AER},
		{10100,	BAND_FIX | BAND_AMT},
		{10150,	BAND_FIX | BAND_MOB},
		{11175,	BAND_AER},
		{11400,	BAND_FIX},
		{11600,	BAND_BCT},
		{12100,	BAND_FIX},
		{12230,	BAND_MAR | BAND_DSC},
		{13200,	BAND_AER},
		{13360,	BAND_FIX | BAND_AST},
		{13410,	BAND_FIX | BAND_MOB},
		{13450,	BAND_FIX | BAND_MOB | BAND_LOC},
		{13550,	BAND_FIX | BAND_MOB},
		{13570,	BAND_BCT},
		{13870,	BAND_FIX | BAND_MOB},
		{14000,	BAND_AMT},
		{14350,	BAND_FIX | BAND_MOB},
		{14990,	BAND_STS},
		{15005,	BAND_STS | BAND_SRE},
		{15010,	BAND_AER},
		{15100,	BAND_BCT},
		{15800,	BAND_FIX},
		{16100,	BAND_FIX | BAND_LOC},
		{16200,	BAND_FIX},
		{16360,	BAND_MAR},
		{17410,	BAND_FIX},
		{17480,	BAND_BCT},
		{17900,	BAND_AER},
		{18030,	BAND_FIX},
		{18052,	BAND_FIX | BAND_SRE},
		{18068,	BAND_AMT},
		{18168,	BAND_FIX | BAND_MOB},
		{18780,	BAND_MAR},
		{18900,	BAND_BCT},
		{19020,	BAND_FIX},
		{19680,	BAND_MAR},
		{19800,	BAND_FIX},
		{19990,	BAND_STS | BAND_SRE},
		{19995,	BAND_STS},
		{20010,	BAND_FIX | BAND_MOB},
		{21000,	BAND_AMT},
		{21450,	BAND_BCT},
		{21850,	BAND_FIX},
		{21924,	BAND_AER},
		{22000,	BAND_MAR},
		{22855,	BAND_FIX},
		{23000,	BAND_FIX | BAND_MOB},
		{23200,	BAND_AER | BAND_FIX},
		{23350,	BAND_FIX | BAND_MOB},
		{24450,	BAND_FIX | BAND_MOB | BAND_LOC},
		{24600,	BAND_FIX | BAND_MOB},
		{24890,	BAND_AMT},
		{24990,	BAND_STS},
		{25005,	BAND_STS | BAND_SRE},
		{25010,	BAND_FIX | BAND_MOB},
		{25070,	BAND_MAR},
		{25210,	BAND_FIX | BAND_MOB},
		{25550,	BAND_AST},
		{25670,	BAND_BCT},
		{26100,	BAND_MAR},
		{26175,	BAND_FIX | BAND_MOB},
		{26200,	BAND_FIX | BAND_MOB | BAND_LOC},
		{26350,	BAND_FIX | BAND_MOB},
		{27500,	BAND_FIX | BAND_MOB | BAND_MET},
		{28000,	BAND_AMT},
		{29700,	BAND_FIX | BAND_MOB},
		{30000,	0},
};

/*
 * Find the Index in the ITU Table.
 *
 * Based on binary search.
 * Algorithm complexity: O(log2(N)+1)
 */
static uint8_t get_itu_index(uint16_t freq)
{
	/*
	 * TODO: when table reach more than 255 entries,
	 * the following types should the extended to 16 bits.
	 */
	uint8_t hi = 0;
	uint8_t lo = 0;

	uint8_t last_mid = 0;
	uint8_t mid = 0;

	hi = sizeof(ItuBand) / sizeof(ItuBand[0]);

	for (;;)
	{
		mid = (hi + lo) / 2;

		/*
		 * When the same element is found over and over again,
		 * that means it found the right index.
		 */
		if (mid == last_mid)
			break;

		if (freq >= ItuBand[mid].bottom_freq)
			lo = mid;
		else hi = mid;

		last_mid = mid;
	}

	return mid;
}


static void get_itu_title(uint8_t index, char *buffer)
{
	uint8_t x;
	uint16_t mask = 1;
	
	/* Prepare string attributes */
	for (x = 0; x < 16; x++, mask <<= 1)
	{
		switch (ItuBand[index].title & mask)
		{
		case BAND_MAR:
			strcat(buffer, BAND_MAR_STR);
			break;
			
		case BAND_MIL:
			strcat(buffer, BAND_MIL_STR);
			break;
			
		case BAND_AER:
			strcat(buffer, BAND_AER_STR);
			break;
			
		case BAND_AMT:
			strcat(buffer, BAND_AMT_STR);
			break;
			
		case BAND_BCT:
			strcat(buffer, BAND_BCT_STR);
			break;
			
		case BAND_STS:
			strcat(buffer, BAND_STS_STR);
			break;
			
		case BAND_AST:
			strcat(buffer, BAND_AST_STR);
			break;
			
		case BAND_MET:
			strcat(buffer, BAND_MET_STR);
			break;
			
		case BAND_NAV:
			strcat(buffer, BAND_NAV_STR);
			break;
			
		case BAND_FIX:
			strcat(buffer, BAND_FIX_STR);
			break;
			
		case BAND_MOB:
			strcat(buffer, BAND_MOB_STR);
			break;

		case BAND_DSC:
			strcat(buffer, BAND_DSC_STR);
			break;

		case BAND_SRE:
			strcat(buffer, BAND_SRE_STR);
			break;

		case BAND_LOC:
			strcat(buffer, BAND_LOC_STR);
			break;

		default:
			/* Do nothing here !! */
			break;
		}
	}
}


//TODO: maybe this should be moved into display_layout.c
void show_itu(int32_t freq)
{
	static char buffer[32] = "";
	static uint8_t last_index = 0;
	uint8_t itu_index = 0;
	uint8_t	x;

	itu_index = get_itu_index(freq);

	if (itu_index == last_index) return;

	/* Clear previous buffer */
	memset(buffer, 0, sizeof(buffer));
	get_itu_title(itu_index, buffer);

	//TODO move into display_layout.c
	lcd_send_instr(LCD_INSTR_SET_DDRAM | 0x40);
	lcd_print(buffer);

	/* Fill remaining chars on display with spaces */
	//FIXME - 16 hardcoded display size
	for (x = 0; x < 16 - strlen(buffer); x++)
	{
		lcd_send_data(' ');
	}

	last_index = itu_index;
}




