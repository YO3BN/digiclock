/*
 * itu_table.c
 *
 *  Created on: Sep 7, 2019
 *      Author: Cristian Ionita
 */


#include <string.h>
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
		{2501,	BAND_STS | BAND_SAR},
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
		{4063,	BAND_MAR},
		{4438,	BAND_FIX | BAND_MOB | BAND_LOC},
		{4488,	BAND_FIX | BAND_MOB},
		{4650,	BAND_AER},
		{4750,	BAND_AER | BAND_BCT | BAND_FIX | BAND_MOB},
		{4850,	BAND_BCT | BAND_FIX | BAND_MOB},
		{4995,	BAND_STS},
		{5003,	BAND_STS | BAND_SAR},
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
		{6200,	BAND_MAR},
		{6525,	BAND_AER},
		{6765,	BAND_FIX | BAND_MOB},
		{7000,	BAND_AMT},
		{7200,	BAND_BCT},
		{7450,	BAND_FIX | BAND_MOB},
		{8100,	BAND_FIX | BAND_MAR},
		{8195,	BAND_MAR},
		{8815,	BAND_AER},
		{9040,	BAND_FIX},
		{9305,	BAND_FIX | BAND_LOC},
		{9355,	BAND_FIX},
		{9400,	BAND_BCT},
		{9900,	BAND_FIX},
		{9995,	BAND_STS},
		{10003,	BAND_STS | BAND_SAR},
		{10005,	BAND_AER},
		{10100,	BAND_FIX | BAND_AMT},
		{10150,	BAND_FIX | BAND_MOB},
		{11175,	BAND_AER},
		{11400,	BAND_FIX},
		{11600,	BAND_BCT},
		{12100,	BAND_FIX},
		{12230,	BAND_MAR},
		{13200,	BAND_AER},
		{13360,	BAND_FIX | BAND_AST},
		{13410,	BAND_FIX | BAND_MOB},
		{13450,	BAND_FIX | BAND_MOB | BAND_LOC},
		{13550,	BAND_FIX | BAND_MOB},
		{13570,	BAND_BCT},
		{13870,	BAND_FIX | BAND_MOB},
		{14000,	BAND_AMT},
		
		{0xffffff, 0},
};




static uint16_t get_itu_index(int32_t freq)
{
	uint16_t i = 0;

	/* Find band */
	while (freq >= ItuBand[i].bottom_freq)
	{
		i++;
	}
	return i - 1;
}

static void get_itu_title(uint16_t index, char *buffer)
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

		case BAND_SAR:
			strcat(buffer, BAND_SAR_STR);
			break;

		case BAND_LOC:
			strcat(buffer, BAND_LOC_STR);
			break;

		default:
			break;
		}
	}
}


//TODO: maybe this should be moved into display_layout.c
void show_itu(int32_t freq)
{
	static char buffer[32] = "";
	static uint16_t last_index = 0;
	uint16_t itu_index = 0;
	uint8_t	x;

	itu_index = get_itu_index(freq);

	if (itu_index != last_index)
	{
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
}




