
#ifndef __JZSOC_SOC_DEV_H__
#define __JZSOC_SOC_DEV_H__

/*
 * Define the module base addresses
 */

/* AHB0 BUS Devices Base */
#define HARB0_IOBASE	        0x13000000
#define DDRC_BASE	            0xb34f0000
#define DDRC1_IOBASE	        0x13012000 /*DDR_APB_BASE*/
#define DDRC_IOBASE	            0x134f0000 /*TODO:*/
#define DDRPHY_IOBASE	        0x13011000 /*TODO:*/
#define AXI_ARB1_IOBASE	        0x13013000 /*TODO:*/
#define AXI_ARB2_IOBASE	        0x13014000 /*TODO:*/
#define MSC0_IOBASE	            0x13060000
#define MSC1_IOBASE	            0x13070000
#define IPU_IOBASE	            0x13080000
#define AIP_IOBASE	        	0x13090000
#define MONITOR_IOBASE			0x130a0000
#define GMAC0_IOBASE			0x130b0000
#define GMAC1_IOBASE			0x130c0000
#define SATA_IOBASE				0x130d0000
#define VO_IOBASE				0x130e0000
#define VDE_IOBASE				0x13300000
#define VDE_DSC_IOBASE			0x13360000
#define HDMI_IOBASE				0x13380000

/* AHB1 BUS Devices Base */
#define VC8000D_IOBASE			0x13100000
#define JPEG_IOBASE				0x13200000

/* AHB2 BUS Devices Base */
#define HARB2_IOBASE	        0x13400000
#define NEMC_IOBASE	            0x13410000
#define PDMA_IOBASE	            0x13420000
#define AES_IOBASE  	        0x13430000
#define SFC0_IOBASE	            0x13440000
#define SFC1_IOBASE	            0x13450000
#define PWM_IOBASE	            0x13460000
#define HASH_IOBASE	            0x13480000
#define RSA_IOBASE	            0x134c0000
#define OTG0_IOBASE	            0x13600000
#define OTG1_IOBASE	            0x13640000
#define OTG2_IOBASE	            0x13680000
#define EFUSE_IOBASE	        0x13540000
#define INTC_IOBASE	            0x10001000

/* CPU and OST */
#define G_OST_IOBASE	        0x12000000 /* G_OST_BASE */
#define N_OST_IOBASE            0x12100000 /* N_OST_BASE */
#define CCU_IOBASE	            0x12200000
#define INTCN_IOBASE	        0x12300000
#define SRAM_IOBASE	            0x12400000
#define NNDMA_IOBASE	        0x12500000
// #define LEPOST_IOBASE	        0x12600000  /* RISC-V OST */
#define LEPCCU_IOBASE	        0x12a00000  /* RISC-V CCU */

/* APB BUS Devices Base */
#define CPM_IOBASE	            0x10000000
#define TCU_IOBASE	            0x10002000
#define RTC_IOBASE	            0x10003000
#define GPIO_IOBASE	            0x10010000
#define AIC0_IOBASE	            0x10020000
#define AIC1_IOBASE	        	0x10021000
#define CODEC_IOBASE	        0x10022000
#define UART0_IOBASE	        0x10030000
#define UART1_IOBASE	        0x10031000
#define UART2_IOBASE	        0x10032000
#define SSI0_IOBASE	            0x10043000
#define SSI1_IOBASE 	        0x10044000
#define I2C0_IOBASE	            0x10050000
#define I2C1_IOBASE	            0x10051000
#define USB_PHY_IOBASE          0x10060000
#define DES_IOBASE  	        0x10061000
#define DTRNG_IOBASE            0x10072000
#define HDMI_PHY_IOBASE         0x10075000
#define VDAC_IOBASE				0x10076000
#define SATA_PHY0_IOBASE		0x10080000
#define SATA_PHY1_IOBASE		0x10090000
#define WDT_IOBASE	            0x10002000

/* NAND CHIP Base Address*/
#define NEMC_CS1_IOBASE         0X1b000000
#define NEMC_CS2_IOBASE         0X1a000000
#define NEMC_CS3_IOBASE         0X19000000
#define NEMC_CS4_IOBASE         0X18000000
#define NEMC_CS5_IOBASE         0X17000000
#define NEMC_CS6_IOBASE         0X16000000

#endif
