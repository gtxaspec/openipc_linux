/*
 * JZSOC CPM register definition.
 *
 * CPM (Clock reset and Power control Management)
 *
 * Copyright (C) 2019 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __CPM_H__
#define __CPM_H__

#define CPM_CPCCR		(0x00)	/* clock control */
#define CPM_RSR			(0x08)	/* Reset status clock divider */
#define CPM_CPPCR		(0x0c)	/* pll control */
#define CPM_CPAPCR		(0x10)	/* apll control */
#define CPM_CPMPCR		(0x14)	/* mpll control */
#define CPM_CPEPCR		(0x18)	/* epll control */
#define CPM_CPVPCR		(0x1c)	/* vpll control */
#define CPM_CPAPACR		(0x20)	/* apll FRAC */
#define CPM_CPMPACR		(0x24)	/* mpll FRAC */
#define CPM_CPEPACR		(0x28)	/* epll FRAC */
#define CPM_CPVPACR		(0x2c)	/* vpll FRAC */
#define CPM_DDRCDR		(0x3c)	/* ddr memory clock divder */
#define CPM_SOFTAPP		(0x40)	/* soft used in application register */
#define CPM_CPSPR		(0x44)	/* cpm scratch pad register */
#define CPM_CPSPPR		(0x48)	/* cpm scratch pad protected register */
#define CPM_USB0PCR		(0x50)	/* usb0 parameter control register 0 */
#define CPM_USB0RDT		(0x54)	/* usb0 reset detect timer register */
#define CPM_USB0VBFIL		(0x58)	/* usb0 vbus jitter filter */
#define CPM_USB0PCR1		(0x5c)	/* usb0 parameter control register 1 */
#define CPM_USB1PCR		(0x60)	/* usb1 parameter control register 0 */
#define CPM_USB1RDT		(0x64)	/* usb1 reset detect timer register */
#define CPM_USB1VBFIL		(0x68)	/* usb1 vbus jitter filter */
#define CPM_USB1PCR1		(0x6c)	/* usb1 parameter control register 0 */
#define CPM_USB2PCR		(0x70)	/* usb2 parameter control register 0 */
#define CPM_USB2RDT		(0x74)	/* usb2 reset detect timer register */
#define CPM_USB2VBFIL		(0x78)	/* usb2 vbus jitter filter */
#define CPM_USB2PCR1		(0x7c)	/* usb2 parameter control register 0 */

//#define CPM_RSACDR		(0x80)	/* RSA clock divider */
#define CPM_AHB1CDR		(0x84)	/* AHB1 clock divider */
#define CPM_SSICDR		(0x8c)
#define CPM_SFC0CDR		(0x90)
#define CPM_SFC1CDR		(0x94)
#define CPM_MSC0CDR		(0x98)
#define CPM_MSC1CDR		(0x9c)
#define CPM_I2S0TCDR 		(0xa0)	/* I2S0 transmit clock divider */
#define CPM_I2S0TCDR1		(0xa4)
#define CPM_I2S0RCDR 		(0xa8)	/* I2S0 receive clock divider */
#define CPM_I2S0RCDR1		(0xac)
#define CPM_VDEVCDR		(0xb0)	/* VDE video output clock divider, dsc output clock */
#define CPM_VDEACDR		(0xb4)	/* VDE AXI clock divider */
#define CPM_VDEMCDR		(0xb8)	/* VDE main clock divider */
#define CPM_IPUMCDR		(0xbc)	/* IPU clock divider */
#define CPM_MAC0CDR		(0xc0)	/* MAC0 clock divider */
#define CPM_MAC0TXCDR		(0xc4)	/* MAC0 TX PHYclock divider */
#define CPM_MAC0PHYC		(0xc8)	/* MAC0 PHY control clock divider */
#define CPM_MACPTPCDR		(0xcc)	/* MAC PTP clock divider */
#define CPM_MAC1CDR		(0xd0)	/* MAC1 clock divider */
#define CPM_MAC1TXCDR		(0xd4)	/* MAC1 TX PHYclock divider */
#define CPM_MAC1PHYC		(0xd8)	/* MAC1 PHY control clock divider */

#define CPM_INTR		(0xe0)
#define CPM_INTRE		(0xe4)
#define CPM_DRCG		(0xe8)	/* DDR clock gate register */
#define CPM_CPCSR		(0xec)	/* clock status register */

#define CPM_BT0CDR		(0xf8)	/* BT0 clock divider */
#define CPM_PWMCDR		(0xfc)	/* BT0 clock divider */
#define CPM_I2S1TCDR 		(0x100)	/* I2S1 transmit clock divider */
#define CPM_I2S1TCDR1		(0x104)
#define CPM_I2S1RCDR 		(0x108)	/* I2S1 receive clock divider */
#define CPM_I2S1RCDR1		(0x10c)

#define SRBC_USB0_SR BIT(17)
#define SRBC_USB1_SR BIT(12)
#define SRBC_USB2_SR BIT(16)

/* power and clock gate */
#define CPM_LCR			(0x04)	/* low power control */
#define CPM_CLKGR		(0x30)	/* clock gate register 0 */
#define CPM_OPCR		(0x34)	/* special control register, oscillator and power control register*/
#define CPM_CLKGR1		(0x38)	/* clock gate register 1*/
#define CPM_OSCCTRL		(0x4c)	/* oscillator control */
#define CPM_SRBC0		(0xf0)	/* soft reset and bus control register */
#define CPM_MESTSEL		(0xf4)	/* CPM metastable state sel register */

//#define CPM_MEMCTRL_MA0		(0xf0)
//#define CPM_MEMCTRL_MA1		(0xf4)
// #define CPM_MEMCTRL_MA2		(0xf8)


#ifndef BIT
#define BIT(nr)  (1UL << nr)
#endif

/*USB Parameter Control Register*/
#define USBPCR_USB_MODE                 BIT(31)
#define USBPCR_AVLD_REG                 BIT(30)
#define USBPCR_IDPULLUP_MASK_BIT        28
#define USBPCR_IDPULLUP_MASK            (0x3 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_IDPULLUP_OTG             (0x0 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_IDPULLUP_ALWAYS_SUSPEND  (0x1 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_IDPULLUP_ALWAYS          (0x2 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_INCR_MASK                BIT(27)
#define USBPCR_POR_BIT                  22
#define USBPCR_POR                      BIT(USBPCR_POR_BIT)

/*USB Reset Detect Timer Register*/
#define USBRDT_RESUME_INTEEN            BIT(31) /*RW*/
#define USBRDT_RESUME_INTERCLR          BIT(30) /*W0*/
#define USBRDT_RESUME_SPEED_BIT         28  /*RW*/
#define USBRDT_RESUME_SPEED_MSK         (0x3 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_SPEED_HIGH        (0x0 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_SPEED_FULL        (0x1 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_SPEED_LOW         (0x2 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_STATUS            BIT(27) /*RO*/
#define USBRDT_HB_MASK                  BIT(26)
#define USBRDT_VBFIL_LD_EN              BIT(25)
#define USBRDT_IDDIG_EN                 24
#define USBRDT_IDDIG_REG                23
#define USBRDT_USBRDT_MSK               (0x7fffff)
#define USBRDT_USBRDT(x)                ((x) & USBRDT_USBRDT_MSK)
#define USBRDT_UTMI_RST                 BIT(27)

/*USB VBUS Jitter Filter Register*/
#define USBVBFIL_USBVBFIL(x)        ((x) & 0xffff)
#define USBVBFIL_IDDIGFIL(x)        ((x) & (0xffff << 16))

/*USB Parameter Control Register1*/
#define USBPCR1_BVLD_REG    	BIT(31)
#define USBPCR1_DPPULLDOWN  	BIT(29)
#define USBPCR1_DMPULLDOWN  	BIT(28)
#define USBPCR1_PORT_RST    	BIT(21)

/*Oscillator and Power Control Register*/
#define OPCR_USB_SPENDN		BIT(7)
#define OPCR_USB_SPENDN1     	BIT(6)
#define OPCR_USB_SPENDN2     	BIT(5)

#define OPCR_USB_PHY_GATE	BIT(23)
#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3) //T31 delete
#define OPCR_IDLE		(0x1<<31)

#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

/* ?????? */
#define CPM_LCR_PD_X2D		(0x1<<31)
#define CPM_LCR_PD_VPU		(0x1<<30)
#define CPM_LCR_PD_MASK		(0x3<<30)
#define CPM_LCR_X2DS 		(0x1<<27)
#define CPM_LCR_VPUS		(0x1<<26)
#define CPM_LCR_STATUS_MASK	(0x3<<26)

#define CPM_A1_CLKGR0_OTG0_GATE (11)
#define CPM_A1_CLKGR0_OTG1_GATE (12)
#define CPM_A1_CLKGR0_OTG2_GATE (13)


#define cpm_inl(off)		inl(CPM_IOBASE + (off))
#define cpm_outl(val,off)	outl(val,CPM_IOBASE + (off))
#define cpm_clear_bit(val,off)	do{cpm_outl((cpm_inl(off) & ~(1 << (val))),off);}while(0)
#define cpm_set_bit(val,off)	do{cpm_outl((cpm_inl(off) | (1 << (val))),off);}while(0)
#define cpm_test_bit(val,off)	(cpm_inl(off) & (0x1 << (val)))

#endif
/* __CPM_H__ */
