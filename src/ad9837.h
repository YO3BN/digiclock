

/* AD9837 Control Register Bits */
#define	D15	15	/* Register select; Control/FREQ0/FREQ1/PHASE0/PHASE1	*/
#define D14	14	/* Register select; Control/FREQ0/FREQ1/PHASE0/PHASE1	*/
#define B28	13	/* Load frequency registers by two write operations	*/
#define HLB	12	/* MSB(1)/LSB(0) frequency register; disabled by B28	*/
#define FSEL	11	/* Select FREQ0/FREQ1 register				*/
#define PSEL	10	/* Select PHASE0/PHASE1 register			*/
#define RESET	8	/* This bit controls the reset function			*/
#define SLEEP1	7	/* Enables/Disables the internal MCLK			*/
#define SLEEP12	6	/* Enable/Disable the internal DAC			*/
#define OPBITEN	5	/* Connect/Disconect DAC to VOUT			*/
#define DIV2	3	/* MSB/2 of the DAC data to be output at the VOUT pin	*/
#define MODE	1	/* Use/bypass SIN_ROM; 0 = sinwave / 1 = triangle	*/


#define DDS_MCLK		75000000
#define FREQ_TO_DDS(x)		(x) = ((((float)(x)) / DDS_MCLK) * 0x10000000)
#define PACKET_HEAD(x,y)	(x) |= (y)
#define GET_14_MSB(x)		(((x) & 0x0fffc000) >> 14)
#define GET_14_LSB(x)		((x) & 0x00003fff)


#define CTRL_REG		0x0000
#define FREQ0			0x4000
#define FREQ1			0x8000
#define PHASE0			0xc000
#define PHASE1			0xe000


