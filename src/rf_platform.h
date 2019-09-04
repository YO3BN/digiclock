
#ifndef __RF_PLATFORM__
#define __RF_PLATFORM__

typedef enum IF_OPERATION_ETag
{
	IF_OPERATION_NONE,	/* Direct Conversion */
	IF_OPERATION_ADD,	/* Addition  (RF + IF) */
	IF_OPERATION_SUB	/* Substraction (RF - IF) || (IF - RF) */
} IF_OPERATION_E;


struct RfPlatform_Tag
{
	uint32_t ulIfCenter;		/* IF filter center frequency */
	uint32_t ulIfUpper;		/* IF filter upper edge frequency */
	uint32_t ulIfLower;		/* IF filter lower edge frequency */
	int32_t lLoAberration;		/* Local Oscillator Aberration/Correction */
	int32_t lBfoOffset;		/* BFO Offset */
	IF_OPERATION_E eIfOperation;	/* IF operation {none, add, sub} */
} RfPlatform;


extern void fnRFPlatformInit(void)
{
	//TODO read from EEEPROM
	RfPlatform.ulIfCenter = 9997200;
	RfPlatform.lLoAberration = 0;
	RfPlatform.eIfOperation = IF_OPERATION_ADD;
	RfPlatform.lBfoOffset = -1600;
}

#define FREQ_TO_PLATFORM(x) fnFreqToPlatform(x)

extern uint32_t fnFreqToPlatform(uint32_t ulRfFreq)
{
	uint32_t lIFFreq = 0;
       
	switch (RfPlatform.eIfOperation)
	{
		case IF_OPERATION_ADD:
		lIFFreq = RfPlatform.ulIfCenter + ulRfFreq;
		break;

		case IF_OPERATION_SUB:
		lIFFreq = (RfPlatform.ulIfCenter > ulRfFreq) ? RfPlatform.ulIfCenter - ulRfFreq : ulRfFreq - RfPlatform.ulIfCenter;
		break;

		default:
		return 0;
	}

	lIFFreq += RfPlatform.lLoAberration;
	lIFFreq += RfPlatform.lBfoOffset;
	
	return lIFFreq;
}

extern void inline
fnSetBFOOffset(int32_t b)
{
	RfPlatform.lBfoOffset = b;
}

extern int32_t
fnGetBFOOffset(void)
{
	return RfPlatform.lBfoOffset;
}

#endif

