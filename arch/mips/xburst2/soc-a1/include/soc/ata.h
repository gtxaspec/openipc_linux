#ifndef __ATA_H__
#define __ATA_H__

#define AHSATA_NUM_PORTS			2    //A1 has 2 port //
#define P0PHYCR_ADDR				0xb30d0178

/*A1 Port0&port1 Driver strength settings
 * Enable to modify amplitude*/
#define P0PHY_DRV_STRENGTH				0xb0080028
#define P1PHY_DRV_STRENGTH				0xb0090028
#define PHY_DRV_STRENGTH_VALUE			0xb0002c92

/*A1 Eye Diagram Correction
 *Register Settings for port0&port1*/
#define P0PHY_TX_MAIN_LEG_BYPS			0xb0081c38
#define P0PHY_TX_MAIN_LEG_VAL			0xb0081c3c
#define P0PHY_RX_CTLE_EQN_BYPS			0xb0080674
#define P0PHY_RX_CTLE_EQN				0xb0080714

#define P1PHY_TX_MAIN_LEG_BYPS			0xb0091c38
#define P1PHY_TX_MAIN_LEG_VAL			0xb0091c3c
#define P1PHY_RX_CTLE_EQN_BYPS			0xb0090674
#define P1PHY_RX_CTLE_EQN				0xb0090714

/*A1 Eye Diagram Correction
 *Register Settings Values*/
#define PHY_TX_MAIN_LEG_BYPS_VALUE		0x3
#define PHY_TX_MAIN_LEG_VAL_VALUE		0xfc
#define PHY_RX_CTLE_EQN_BYPS_VALUE		0x800
#define PHY_RX_CTLE_EQN_VALUE			0x1cf3f73c


/* sata inno phy  clock register && Rx squelch&offset config value*/
#define PHY_RX_SQUELCH_VALUE			(0x00001284)
#define PHY_RX_OFFSET_A_VALUE			(0xff)
#define PHY_RX_OFFSET_B_VALUE			(0x00010000)
#define PHY_RX_OFFSET_C_VALUE			(0xd0000000)

/*sata clock and softreset register*/
#define CLKGR1_SATA				(1<<11)
#define SRBC_SATA_SR			(1<<18)

/* PnPHYCR register( for n=0;n <= AHSATA_NUM_PORTS-1) */

#define PnPHYCR_IDLE_EN				(1<<0)
#define PnPHYCR_PHY_RESET_N			(1<<4)
#define PnPHYCR_PHY_RESET_N_SEL		(1<<5)
#define PnPHYCR_SPDSEL				(1<<8)
#define PnPHYCR_SPDSEL_SEL			(1<<9)
#define PnPHYCR_HOST_SEL			(1<<12)
#define PnPHYCR_LISTEN				(1<<16)
#define PnPHYCR_OFFLINE				(1<<20)
#define PnPHYCR_FORCE_READY			(1<<24)
#define PnPHYCR_POWER_RESET_N		(1<<28)
#define PnPHYCR_MSB					(0<<31)

//AHCI reg define
#define AHCI_BISTAFR			0x00a0
#define AHCI_BISTCR				0x00a4
#define AHCI_BISTFCTR			0x00a8
#define AHCI_BISTSR				0x00ac
#define AHCI_BISTDECR			0x00b0
#define AHCI_DIAGNR0			0x00b4
#define AHCI_DIAGNR1			0x00b8
#define AHCI_OOBR				0x00bc
#define AHCI_PHYCS0R			0x00c0
#define AHCI_PHYCS1R			0x00c4
#define AHCI_PHYCS2R			0x00c8
#define AHCI_TIMER1MS			0x00e0
#define AHCI_GPARAM1R			0x00e8
#define AHCI_GPARAM2R			0x00ec
#define AHCI_PPARAMR			0x00f0
#define AHCI_TESTR				0x00f4
#define AHCI_VERSIONR			0x00f8
#define AHCI_IDR				0x00fc

#define AHCI_P0DMACR			0x0170
#define AHCI_P0PHYCR			0x0178
#define AHCI_P0PHYSR			0x017c

//Random BIST Registers
#define INNO_RANDOM_BIST1			0x0d04
#define INNO_RAMDOM_BIST2			0x0d08
#define INNO_RAMDOM_BIST3			0x0d0c
#define INNO_RAMDOM_BIST4			0x0d10
#define INNO_RAMDOM_BIST5			0x0d14
#define INNO_RAMDOM_BIST6			0x0d18
#define INNO_RAMDOM_BIST7			0x0d1c
#define INNO_RAMDOM_BIST8			0x0d20
#define INNO_RAMDOM_BIST9			0x0d24
#define INNO_RAMDOM_BIST10			0x0d28


/*Add phy init for A1 NVR Chip
 *with INNO phy
 */
//input Data Width Control Register
#define INNO_INPUT_DATA_WIDTH			0x0c38

//Output Data Width Control Register
#define INNO_OUTPUT_DATA_WIDTH_CTL1		0x0d54
#define INNO_OUTPUT_DATA_WIDTH_CTL2		0x0c38

//External OOB Input
#define INNO_EXT_OOB_INPUT1			0x0c38
#define INNO_EXT_OOB_INPUT2			0x0c08

//Calibration Registers
#define INNO_CALI_REG1				0x0c08
#define INNO_CALI_REG2				0x0e94

//Fixed Pattern BIST Registers
#define INNO_FIXED_PAT_BIST			0x0c04

#endif
/* __ATA_H__ */
