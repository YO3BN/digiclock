/*
 * itu_table.h
 *
 *  Created on: Sep 7, 2019
 *      Author: Cristian Ionita
 */

#include <stdint.h>

#ifndef ITU_TABLE_H_
#define ITU_TABLE_H_


#define BAND_DSC	0x0001
#define BAND_FIX	0x0002
#define BAND_MOB	0x0004
#define BAND_MAR	0x0008
#define BAND_AER	0x0010
#define BAND_BCT	0x0020
#define BAND_AMT	0x0040
#define BAND_NAV	0x0080
#define BAND_LOC	0x0100
#define BAND_STS	0x0200
#define BAND_AST	0x0400
#define BAND_MET	0x0800
#define BAND_SRE	0x1000
#define BAND_MIL	0x2000

#define BAND_UNK_STR	"UNK "
#define BAND_MAR_STR	"MAR "
#define BAND_MIL_STR	"MIL "
#define BAND_AER_STR	"AER "
#define BAND_AMT_STR	"AMT "
#define BAND_BCT_STR	"BCT "
#define BAND_STS_STR	"STS "
#define BAND_AST_STR	"AST "
#define BAND_MET_STR	"MET "
#define BAND_NAV_STR	"NAV "
#define BAND_FIX_STR	"FIX "
#define BAND_MOB_STR	"MOB "
#define BAND_DSC_STR	"DSC "
#define BAND_SRE_STR	"SRE "
#define BAND_LOC_STR	"LOC "

typedef struct __attribute__ ((packed))
{
	uint32_t bottom_freq;
	uint16_t title;
} ITU_Band;


void show_itu(int32_t freq);


#endif /* ITU_TABLE_H_ */
