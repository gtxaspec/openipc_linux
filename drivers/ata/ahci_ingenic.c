/*
 * Ingenic Technology Co.Ltd.
 * 2021 for A1 NVR Chip.SATA driver Based on
 * AHCI SATA platform driver
 * Copyright 2004-2005  Red Hat, Inc.
 *   Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2010  MontaVista Software, LLC.
 *   Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include <linux/acpi.h>
#include <linux/pci_ids.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <soc/cpm.h>
#include <soc/ata.h>
#include "ahci.h"
#include "dwc_sata.h"
#include <linux/delay.h>
#define DRV_NAME "ahci-ingenic"

unsigned int sata_phy_reset(unsigned int port_num, unsigned char rst_n)
{
	unsigned int reg = (port_num == 0)?P0PHYCR_ADDR:(P0PHYCR_ADDR+0x80);
	if(0 == rst_n) {
		unsigned int data = 0;

		data = *(volatile unsigned int *)reg;
		data &= ~PnPHYCR_PHY_RESET_N;
		data &= ~PnPHYCR_POWER_RESET_N;
		*(volatile unsigned int *)reg = data;
	} else {
		unsigned int data = 0;

		data = *(volatile unsigned int *)reg;
		udelay(2);
		data |= PnPHYCR_POWER_RESET_N;
		*(volatile unsigned int *)reg = data;
		udelay(2);
		data |= PnPHYCR_PHY_RESET_N;
		*(volatile unsigned int *)reg = data;
	}
	return 0;
}

void dump_phy_status(void)
{
	printk(KERN_DEBUG "dump sata phy status:\n");
	printk(KERN_DEBUG "0x130d0178 = 0x%08x\n", *(volatile unsigned int *)0xb30d0178);
	printk(KERN_DEBUG "0x130d017c = 0x%08x\n", *(volatile unsigned int *)0xb30d017c);
	printk(KERN_DEBUG "0x130d01f8 = 0x%08x\n", *(volatile unsigned int *)0xb30d01f8);
	printk(KERN_DEBUG "0x130d01fc = 0x%08x\n", *(volatile unsigned int *)0xb30d01fc);
	printk(KERN_DEBUG "0x130d00d0 = 0x%08x\n", *(volatile unsigned int *)0xb30d00d0);
	printk(KERN_DEBUG "0x130d00d4 = 0x%08x\n", *(volatile unsigned int *)0xb30d00d4);
}

#define SATA_GEN 2
static int ahci_ingenic_phy_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	unsigned int cpm_clkgate, p0_rxsqul,p0_rxoffseta, p0_rxoffsetb, p0_rxoffsetc;
	int err;
	unsigned int data, srbc;

	err = of_property_read_u32(np, "ingenic,clkgate_reg", &cpm_clkgate);
	if (err < 0)
		return 0;
	/*CPM config
	CLKGR1:bit11  open
	SRBS0: bit18  reset
	*/
	srbc  = *(volatile unsigned int *)cpm_clkgate;
	srbc |= SRBC_SATA_SR;
	*(volatile unsigned int *)cpm_clkgate = srbc;
	srbc &= ~SRBC_SATA_SR;
	*(volatile unsigned int *)cpm_clkgate = srbc;

	/* add these config for inno phy port0&port1 to solve some problem disk,
	 * these config set for rx squelch and rx offset calibration.
	 * Port0 config as below:
	 */
	err = of_property_read_u32(np, "ingenic,phy0_rxsqul_reg", &p0_rxsqul);
	if (err < 0)
		return 0;

	err = of_property_read_u32(np, "ingenic,phy0_rxoffseta_reg", &p0_rxoffseta);
	if (err < 0)
		return 0;

	err = of_property_read_u32(np, "ingenic,phy0_rxoffsetb_reg", &p0_rxoffsetb);
	if (err < 0)
		return 0;

	err = of_property_read_u32(np, "ingenic,phy0_rxoffsetc_reg", &p0_rxoffsetc);
	if (err < 0)
		return 0;

    /* sata inno phy  clock register && Rx squelch&offset config value*/
	*(volatile unsigned int *)p0_rxsqul = 0x1284;
	*(volatile unsigned int *)p0_rxoffseta &= (~(0xff));
	data = *(volatile unsigned int *)p0_rxoffsetb;
	data &= (~(0x1<<16));
	data |= (0x1<<17);
	*(volatile unsigned int *)p0_rxoffsetb = data;
	data = *(volatile unsigned int *)p0_rxoffsetc;
	data &= (~(0x3<<30));
	data |= (0x1<<29);
	*(volatile unsigned int *)p0_rxoffsetc = data;

	/*Eye Diagran correction for port0 */
	*(volatile unsigned int *)P0PHY_TX_MAIN_LEG_BYPS	= PHY_TX_MAIN_LEG_BYPS_VALUE;
	*(volatile unsigned int *)P0PHY_TX_MAIN_LEG_VAL		= PHY_TX_MAIN_LEG_VAL_VALUE;
	*(volatile unsigned int *)P0PHY_RX_CTLE_EQN_BYPS	= PHY_RX_CTLE_EQN_BYPS_VALUE;
	*(volatile unsigned int *)P0PHY_RX_CTLE_EQN	        = PHY_RX_CTLE_EQN_VALUE;
	/* Set port0 Driver strength */
	*(volatile unsigned int *)P0PHY_DRV_STRENGTH		= PHY_DRV_STRENGTH_VALUE;

	/*set for SATA Controller PnPHYCR(for n=0; n < AHSATA_NUM_PORTS)*/
	//P0PHYCR
	data = *(volatile unsigned int *)P0PHYCR_ADDR;
	printk("Before config,P0PHYCR is 0x%04x\n",data);
#if (SATA_GEN == 2)
	data |= PnPHYCR_HOST_SEL|PnPHYCR_SPDSEL|PnPHYCR_MSB;//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#else
	data |= (PnPHYCR_HOST_SEL&(~PnPHYCR_SPDSEL))|PnPHYCR_MSB;//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#endif
	*(volatile unsigned int *)P0PHYCR_ADDR = data;

	udelay(2);
	data |= PnPHYCR_POWER_RESET_N;
	*(volatile unsigned int *)P0PHYCR_ADDR = data;

	udelay(2);
	data |= PnPHYCR_PHY_RESET_N;
	*(volatile unsigned int *)P0PHYCR_ADDR = data;

    /*AHCI port register has same offset address,
	* same register config for port0&port1
	*/
	*(volatile unsigned int *)(p0_rxsqul+0x10000) = 0x1284;
	*(volatile unsigned int *)(p0_rxoffseta+0x10000) &= (~(0xff));
	data = *(volatile unsigned int *)(p0_rxoffsetb+0x10000);
	data &= (~(0x1<<16));
	data |= (0x1<<17);
	*(volatile unsigned int *)(p0_rxoffsetb+0x10000) = data;
	data = *(volatile unsigned int *)(p0_rxoffsetc+0x10000);
	data &= (~(0x3<<30));
	data |= (0x1<<29);
	*(volatile unsigned int *)(p0_rxoffsetc+0x10000) = data;

    /*Eye Diagran correction for port1 */
	*(volatile unsigned int *)P1PHY_TX_MAIN_LEG_BYPS	= PHY_TX_MAIN_LEG_BYPS_VALUE;
	*(volatile unsigned int *)P1PHY_TX_MAIN_LEG_VAL		= PHY_TX_MAIN_LEG_VAL_VALUE;
	*(volatile unsigned int *)P1PHY_RX_CTLE_EQN_BYPS	= PHY_RX_CTLE_EQN_BYPS_VALUE;
	*(volatile unsigned int *)P1PHY_RX_CTLE_EQN			= PHY_RX_CTLE_EQN_VALUE;
	/* Set port1 Driver strength */
	*(volatile unsigned int *)P1PHY_DRV_STRENGTH		= PHY_DRV_STRENGTH_VALUE;

	//P1PHYCR = p0PHYCR + 0x80
	data = *(volatile unsigned int *)(P0PHYCR_ADDR+0x80);
#if (SATA_GEN == 2)
	data |= PnPHYCR_HOST_SEL|PnPHYCR_SPDSEL|PnPHYCR_MSB;//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#else
	data |= (PnPHYCR_HOST_SEL&(~PnPHYCR_SPDSEL))|PnPHYCR_MSB;//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#endif
	*(volatile unsigned int *)(P0PHYCR_ADDR+0x80) = data;

	udelay(2);
	data |= PnPHYCR_POWER_RESET_N;
	*(volatile unsigned int *)(P0PHYCR_ADDR+0x80) = data;

	udelay(2);
	data |= PnPHYCR_PHY_RESET_N;
	*(volatile unsigned int *)(P0PHYCR_ADDR+0x80) = data;

	return 0;
}

static const struct ata_port_info ahci_ingenic_port_info = {
	.flags		= AHCI_FLAG_COMMON | ATA_FLAG_NCQ,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &ahci_platform_ops,
};

static struct scsi_host_template ahci_platform_sht = {
	AHCI_SHT(DRV_NAME),
};

#if 0
static void sata_controller_dump(void)
{
	printk("---------sata controller global--------\n");
#define PRINT_REG(NAME) printk("0x%08x, %-32s: 0x%08x\n", NAME, #NAME, readl((const volatile void *)NAME));
		PRINT_REG(SATA_CAP)
		PRINT_REG(SATA_GHC)
		PRINT_REG(SATA_IS)
		PRINT_REG(SATA_PI)
		PRINT_REG(SATA_VS)
		PRINT_REG(SATA_CCC_CTL)
		PRINT_REG(SATA_CCC_PORTS)
		PRINT_REG(SATA_CAP2)
		PRINT_REG(SATA_BISTAFR)
		PRINT_REG(SATA_BISTCR)
		PRINT_REG(SATA_BISTFCTR)
		PRINT_REG(SATA_BISTSR)
		PRINT_REG(SATA_BISTDECR)
		PRINT_REG(SATA_MEMEDFBSADDRERRDATA)
		PRINT_REG(SATA_MEMEDADDRERRINJ)
		PRINT_REG(SATA_OOBR)
		PRINT_REG(SATA_MEMEDADDRERRDATAU)
		PRINT_REG(SATA_DIAGNR3)
		PRINT_REG(SATA_GPCR)
		PRINT_REG(SATA_GPSR)
		PRINT_REG(SATA_CACHECR)
		PRINT_REG(SATA_GPARAM3)
		PRINT_REG(SATA_TIMER1MS)
		PRINT_REG(SATA_MEMDPCR)
		PRINT_REG(SATA_GPARAM1R)
		PRINT_REG(SATA_GPARAM2R)
		PRINT_REG(SATA_PPARAMR)
		PRINT_REG(SATA_TESTR)
		PRINT_REG(SATA_VERSIONR)
		PRINT_REG(SATA_IDR)
		printk("---------sata controller port 0--------\n");
		PRINT_REG(SATA_P_CLB(0))
		PRINT_REG(SATA_P_CLBU(0))
		PRINT_REG(SATA_P_FB(0))
		PRINT_REG(SATA_P_FBU(0))
		PRINT_REG(SATA_P_IS(0))
		PRINT_REG(SATA_P_IE(0))
		PRINT_REG(SATA_P_CMD(0))
		PRINT_REG(SATA_P_TFD(0))
		PRINT_REG(SATA_P_SIG(0))
		PRINT_REG(SATA_P_SSTS(0))
		PRINT_REG(SATA_P_SCTL(0))
		PRINT_REG(SATA_P_SERR(0))
		PRINT_REG(SATA_P_SACT(0))
		PRINT_REG(SATA_P_CI(0))
		PRINT_REG(SATA_P_SNTF(0))
		PRINT_REG(SATA_P_FBS(0))
		PRINT_REG(SATA_P_DEVSLP(0))
		PRINT_REG(SATA_P_DMACR(0))
		PRINT_REG(SATA_P_PHYCR(0))
		PRINT_REG(SATA_P_PHYSR(0))
#undef PRINT_REG
}
#endif

static int ahci_ingenic_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	int rc, ret;
	struct clk  *clk_sata_gate;

	clk_sata_gate = clk_get(&pdev->dev, "gate_sata");
	if (!clk_sata_gate) {
		dev_err(&pdev->dev, "Failed to Get sata gate clk!\n");
		return PTR_ERR(clk_sata_gate);
	}
	ret = clk_prepare_enable(clk_sata_gate);
	if(ret) {
		dev_err(&pdev->dev, "enable sata gate clock failed!\n");
	}

	hpriv = ahci_platform_get_resources(pdev);
	if (IS_ERR(hpriv)) {
		return PTR_ERR(hpriv);
	}
	rc = ahci_platform_enable_resources(hpriv);

	if (rc) {
		return rc;
	}
	//port 0/1 implement
	of_property_read_u32(dev->of_node,
			     "ports-implemented", &hpriv->force_port_map);
	if (of_device_is_compatible(dev->of_node, "ingenic,a1-ahci"))
		hpriv->flags |= AHCI_HFLAG_YES_FBS | AHCI_HFLAG_YES_NCQ;

	printk("%s, %d,init phy\n", __func__, __LINE__);
	ret = ahci_ingenic_phy_init(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to Get sata CPM data from DTS!\n");
	}

	rc = ahci_platform_init_host(pdev, hpriv,&ahci_ingenic_port_info,
				     &ahci_platform_sht);
	if (rc)
		goto disable_resources;

	return 0;

disable_resources:
	ahci_platform_disable_resources(hpriv);
	return rc;
}

static SIMPLE_DEV_PM_OPS(ahci_pm_ops, ahci_platform_suspend,
		ahci_platform_resume);

static const struct of_device_id ahci_ingenic_of_match[] = {
	{ .compatible = "generic-ahci"},
	{ .compatible = "ingenic,a1-ahci"},
	{},
};
MODULE_DEVICE_TABLE(of,ahci_ingenic_of_match);

static const struct acpi_device_id ahci_acpi_match[] = {
	{ ACPI_DEVICE_CLASS(PCI_CLASS_STORAGE_SATA_AHCI, 0xffffff) },
	{},
};
MODULE_DEVICE_TABLE(acpi, ahci_acpi_match);

static struct platform_driver ahci_ingenic_driver = {
	.probe = ahci_ingenic_probe,
	.remove = ata_platform_remove_one,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = ahci_ingenic_of_match,
		.pm = &ahci_pm_ops,
	},
};
module_platform_driver(ahci_ingenic_driver);

MODULE_DESCRIPTION("Ingenic AHCI SATA platform driver");
MODULE_AUTHOR("Anton Vorontsov <avorontsov@ru.mvista.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ahci");
