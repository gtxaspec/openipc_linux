/*
 * dwmac-ingenic.c - Ingenic A1 DWMAC specific glue layer
 *
 * Copyright (C) 2021 Ingenic
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/stmmac.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/of_net.h>
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#include "stmmac_platform.h"

struct ingenic_priv_data {
	int interface;
	int clk_enabled;
	unsigned int cpm_mphyc;
	/*hw reset*/
	u32 reset_ms;
	int reset_gpio;
	u32 reset_lvl;
	struct clk *tx_clk;
	struct clk *clk_gate;
	struct clk *macphy_clk;
	unsigned int macphy_rate;
	unsigned int macphy_type;
};

#define INGENIC_GMAC_RATE_1000	125000000
#define INGENIC_GMAC_RATE_100	25000000
#define INGENIC_GMAC_RATE_10	2500000
#define INGENIC_GMAC_RMII_RATE	50000000        //Edwin.Wen@2022/05/16 if rmii phy need 25Mhz clk,modify this macro.

static int ingenic_gmac_init(struct platform_device *pdev, void *priv)
{
	struct plat_stmmacenet_data *plat_dat = priv;
	struct ingenic_priv_data *gmac = plat_dat->bsp_priv;
	struct device_node *np = pdev->dev.of_node;
	unsigned int data;
	int err;

	/* Set GMAC interface port mode
	 *
	 * The GMAC TX clock lines are configured by setting the clock
	 * rate, which then uses the auto-reparenting feature of the
	 * clock driver, and enabling/disabling the clock.
	 */

	pr_debug("%s[%d]: start\n",__func__,__LINE__);
	err = of_property_read_u32(np, "ingenic,mode-reg", &gmac->cpm_mphyc);
	if (err < 0)
		return err;
	pr_debug("%s[%d]: cpm = 0x%08x\n",__func__,__LINE__,gmac->cpm_mphyc);

	data = *(volatile unsigned int *)gmac->cpm_mphyc;
	data &= ~(0x7 << 29);
	data &= ~0x3;

	if (gmac->interface == PHY_INTERFACE_MODE_RGMII) {

		if (gmac->clk_enabled) {
			clk_disable_unprepare(gmac->macphy_clk);
			clk_disable_unprepare(gmac->tx_clk);
			gmac->clk_enabled = 0;
		}
		//clk_unprepare(gmac->tx_clk);
		//printk("%s[%d]: rmii %d\n",__func__,__LINE__,err);

		//set phy work clk 25Mhz
		clk_set_rate(gmac->macphy_clk, gmac->macphy_rate);

		if(plat_dat->max_speed == 1000){
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_1000);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0x60000001;
		}else if(plat_dat->max_speed == 100){
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_100);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0x80000001;
		}else{
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_10);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0xe0000001;
		}

		err=clk_prepare_enable(gmac->macphy_clk);
		if (err)
			printk("%s[%d]: rgmii ret %d\n",__func__,__LINE__,err);
		err=clk_prepare_enable(gmac->tx_clk);
		if (err)
			printk("%s[%d]: rgmii ret %d\n",__func__,__LINE__,err);
		err=clk_prepare_enable(gmac->clk_gate);
		if (err)
			printk("%s[%d]: rgmii ret %d\n",__func__,__LINE__,err);
	} else if(gmac->interface == PHY_INTERFACE_MODE_RMII) {

		if (gmac->clk_enabled) {
			clk_disable_unprepare(gmac->macphy_clk);
			clk_disable_unprepare(gmac->tx_clk);
			gmac->clk_enabled = 0;
		}

		clk_set_rate(gmac->macphy_clk, gmac->macphy_rate);

		if(plat_dat->max_speed == 100){
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_100);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0x80000002;
		}else{
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_10);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0xe0000002;
		}

		err=clk_prepare_enable(gmac->macphy_clk);
		if (err)
			printk("%s[%d]: rmii ret %d\n",__func__,__LINE__,err);
		err=clk_prepare_enable(gmac->tx_clk);
		if (err)
			printk("%s[%d]: rgmii ret %d\n",__func__,__LINE__,err);
		err=clk_prepare_enable(gmac->clk_gate);
		if (err)
			printk("%s[%d]: rmii ret %d\n",__func__,__LINE__,err);
	}else{
		return -EINVAL;
	}
	pr_debug("%s[%d]: set txclk,max speed %d,err %d\n",__func__,__LINE__,plat_dat->max_speed,err);

	gmac->clk_enabled = 1;
	return err;
}

static void ingenic_gmac_exit(struct platform_device *pdev, void *priv)
{
	struct ingenic_priv_data *gmac = priv;

	if(gmac->clk_enabled){
		clk_disable_unprepare(gmac->macphy_clk);
		clk_disable_unprepare(gmac->tx_clk);
		clk_disable_unprepare(gmac->clk_gate);
	}
	gmac->clk_enabled = 0;
}

static void ingenic_fix_speed(void *priv, unsigned int speed)
{
	struct ingenic_priv_data *gmac = priv;
	unsigned int data;
	int ret = 0;

	printk("%s[%d]: cpm = %x,speed = %d,interface = %d,7rmii 8rgmii\n",__func__,__LINE__,gmac->cpm_mphyc,speed,gmac->interface);

	data = *(volatile unsigned int *)gmac->cpm_mphyc;
	data &= ~0x3 | ~(0x7 << 29);
	if (gmac->interface == PHY_INTERFACE_MODE_RGMII) {

		if (gmac->clk_enabled) {
			/*clk_disable_unprepare(gmac->macphy_clk);*/
			clk_disable_unprepare(gmac->tx_clk);
			gmac->clk_enabled = 0;
		}

		//set phy work clk 25Mhz
		/*clk_set_rate(gmac->macphy_clk, gmac->macphy_rate);*/

		if(speed == 1000){
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_1000);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0x60000001;
		}else if(speed == 100){
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_100);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0x80000001;
		}else{
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_10);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0xe0000001;
		}
		/*clk_prepare_enable(gmac->macphy_clk);*/
		/*if (ret)*/
			/*printk("%s[%d],ret=%d\n",__func__,__LINE__,ret);*/
		clk_prepare_enable(gmac->tx_clk);
		if (ret)
			printk("%s[%d],ret=%d\n",__func__,__LINE__,ret);

	} else if(gmac->interface == PHY_INTERFACE_MODE_RMII) {

		if (gmac->clk_enabled) {
			/*clk_disable_unprepare(gmac->macphy_clk);*/
			clk_disable_unprepare(gmac->tx_clk);
			printk("%s[%d],clk_disable\n",__func__,__LINE__);
			gmac->clk_enabled = 0;
		}

		/*ret=clk_set_rate(gmac->macphy_clk, gmac->macphy_rate);*/
		/*if (ret)*/
			/*printk("%s[%d],ret=%d\n",__func__,__LINE__,ret);*/

		if(speed == 100){
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_100);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0x80000002;
		}else{
			clk_set_rate(gmac->tx_clk, INGENIC_GMAC_RATE_10);
			*(volatile unsigned int *)gmac->cpm_mphyc = data | 0xe0000002;
		}

		/*ret=clk_prepare_enable(gmac->macphy_clk);*/
		/*if (ret)*/
			/*printk("%s[%d],ret=%d\n",__func__,__LINE__,ret);*/
		clk_prepare_enable(gmac->tx_clk);
		if (ret)
			printk("%s[%d],ret=%d\n",__func__,__LINE__,ret);

	}else{
		;
	}

	gmac->clk_enabled = 1;

}

#if 0 //Edwin.Wen@2022/05/20 phy reset in uboot one time,do not need reset in kernel probe again.
static int ingenic_xgmac_phy_hwrst(struct platform_device *pdev, bool init,struct ingenic_priv_data  *gmac)
{
	//struct net_device *ndev = platform_get_drvdata(pdev);
	struct ingenic_priv_data *lp = gmac;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	enum of_gpio_flags flags;
	int ret = 0;
	struct pinctrl *p = pinctrl_get(dev);
	struct pinctrl_state *state = NULL;

	pr_debug("!!!! %s[%d]: np %#x,dev %#x ,lp %#x ,p %#x\n",__func__,__LINE__,(u32)np,(u32)dev,(u32)lp,(u32)p);

	if (IS_ERR_OR_NULL(p) || IS_ERR_OR_NULL(lp)) {
		dev_err(&pdev->dev, "can not get pinctrl,or lp is null\n");
		return -1;
	}

	if (!init)
		goto hw_reset;


	lp->reset_gpio = of_get_named_gpio_flags(np, "ingenic,reset-gpios", 0, &flags);
	if (lp->reset_gpio < 0){
		ret=lp->reset_gpio;
		printk("!!!! error %s[%d]: lp->reset_gpio %d\n",__func__,__LINE__,lp->reset_gpio);
		goto out;
	}
	ret = devm_gpio_request(dev, lp->reset_gpio, "mac-hw-rst");
	if (ret) {
		lp->reset_gpio = ret;
		dev_err(dev, "ingenic mac-hw-rst gpio request failed errno(%d)\n", ret);
		goto out;
	}

	lp->reset_lvl = flags & OF_GPIO_ACTIVE_LOW ? 0 : 1;

#define DEFAULT_RESET_MS 10
	if (of_property_read_u32(np, "ingenic,rst-ms", &lp->reset_ms) < 0)
		lp->reset_ms = DEFAULT_RESET_MS;
#undef DEFAULT_RESET_MS

hw_reset:
	state = pinctrl_lookup_state(p, "reset");
	if (!IS_ERR_OR_NULL(state))
		pinctrl_select_state(p, state);

	gpio_direction_output(lp->reset_gpio, lp->reset_lvl);
	if (in_atomic())
		mdelay(lp->reset_ms);
	else
		msleep(lp->reset_ms);
	gpio_direction_output(lp->reset_gpio, !lp->reset_lvl);
        mdelay(80);// need delay any time to read phy reg.
	pr_debug("!!!! gmac %s[%d]: set reset gpio low level\n",__func__,__LINE__);

	state = pinctrl_lookup_state(p, PINCTRL_STATE_DEFAULT);
	if (!IS_ERR_OR_NULL(state))
		pinctrl_select_state(p, state);

out:
	pinctrl_put(p);
	pr_debug("!!!! %s[%d]:ret %d,lp->reset_ms %d,lp->reset_gpio %d,lp->reset_lvl %d, \n",
			__func__,__LINE__,ret,lp->reset_ms,lp->reset_gpio,lp->reset_lvl);
	return ret;
}
#endif


static int ingenic_xgmac_builtin_phy_init(struct platform_device *pdev    )
{
	//struct net_device *ndev = platform_get_drvdata(pdev);
        int phy_rxd3_pin,phy_rxdv_pin;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	enum of_gpio_flags phy_rxd3_pin_flags,phy_rxdv_pin_flags;
	int ret = 0;
	struct pinctrl *p = pinctrl_get(dev);
	//struct pinctrl_state *state = NULL;

	if (IS_ERR_OR_NULL(p) ) {
		dev_err(&pdev->dev, "can not get pinctrl,or lp is null\n");
		return -1;
	}

	phy_rxd3_pin = of_get_named_gpio_flags(np, "ingenic,phy-rxd3-pin", 0, &phy_rxd3_pin_flags);
	if (phy_rxd3_pin < 0){
		ret=phy_rxd3_pin;
		printk("!!!! error %s[%d]: phy_rxd3_pin %d\n",__func__,__LINE__,phy_rxd3_pin);
		goto out;
	}
	ret = devm_gpio_request(dev, phy_rxd3_pin, "mac-phy_rxd3_pin");
	if (ret) {
		phy_rxd3_pin = ret;
		dev_err(dev, "ingenic phy_rxd3_pin gpio request failed errno(%d)\n", ret);
		goto out;
	}
	gpio_direction_output(phy_rxd3_pin, (phy_rxd3_pin_flags & OF_GPIO_ACTIVE_LOW ? 0 : 1));

        phy_rxdv_pin = of_get_named_gpio_flags(np, "ingenic,phy-rxdv-pin", 0, &phy_rxdv_pin_flags);
        if (phy_rxdv_pin < 0){
                ret=phy_rxdv_pin;
                printk("!!!! error %s[%d]: phy_rxdv_pin %d\n",__func__,__LINE__,phy_rxdv_pin);
                goto out;
        }
        ret = devm_gpio_request(dev, phy_rxdv_pin, "mac-phy_rxdv_pin");
        if (ret) {
                phy_rxdv_pin = ret;
                dev_err(dev, "ingenic phy_rxdv_pin gpio request failed errno(%d)\n", ret);
                goto out;
        }

        gpio_direction_output(phy_rxdv_pin, (phy_rxdv_pin_flags & OF_GPIO_ACTIVE_LOW ? 0 : 1));



out:
	pinctrl_put(p);
        printk("builtin phy init ret %d\n",ret);
	return ret;
}


static int ingenic_gmac_probe(struct platform_device *pdev)
{
	struct plat_stmmacenet_data *plat_dat;
	struct stmmac_resources stmmac_res;
	struct ingenic_priv_data *gmac;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	int ret = 0;
	char id[15] = {0};
	char clk_phy[15] = {0};
	char clk_gate[15] = {0};
        //char out_string[20] = {0};

	pr_debug("############### %s[%d]: start\n",__func__,__LINE__);

	ret = stmmac_get_platform_resources(pdev, &stmmac_res);
	if (ret)
		return ret;

	if (pdev->dev.of_node) {
		plat_dat = stmmac_probe_config_dt(pdev, &stmmac_res.mac);
		if (IS_ERR(plat_dat)) {
			dev_err(&pdev->dev, "dt configuration failed\n");
			return PTR_ERR(plat_dat);
		}
	} else {
		plat_dat = dev_get_platdata(&pdev->dev);
		if (!plat_dat) {
			dev_err(&pdev->dev, "no platform data provided\n");
			return  -EINVAL;
		}

		/* Set default value for multicast hash bins */
		plat_dat->multicast_filter_bins = HASH_TABLE_SIZE;

		/* Set default value for unicast filter entries */
		plat_dat->unicast_filter_entries = 1;
	}

	gmac = devm_kzalloc(dev, sizeof(*gmac), GFP_KERNEL);
	if (!gmac) {
		ret = -ENOMEM;
		goto err_remove_config_dt;
	}

#if 0 //Edwin.Wen@2022/05/20 phy reset in uboot one time,do not need reset in kernel probe again.
	//reset phy
	if (ingenic_xgmac_phy_hwrst(pdev, true, gmac)) {
		return -ENODEV;
	}
#endif
	gmac->interface = plat_dat->interface;

	//get tx clk div
	if(strstr(pdev->dev.of_node->full_name, "gmac0")){
		memcpy(id,"div_mac0txphy", strlen("div_mac0txphy"));
		memcpy(clk_gate,"gate_gmac0", strlen("gate_gmac0"));
		memcpy(clk_phy,"div_mac0phy", strlen("div_mac0phy"));
	}else if(strstr(pdev->dev.of_node->full_name, "gmac1")){
		memcpy(id,"div_mac1txphy", strlen("div_mac1txphy"));
		memcpy(clk_gate,"gate_gmac1", strlen("gate_gmac1"));
		memcpy(clk_phy,"div_mac1phy", strlen("div_mac1phy"));
	}else{
		return  -EINVAL;
	}
	gmac->tx_clk = devm_clk_get(dev, id);
	if (IS_ERR(gmac->tx_clk)) {
		dev_err(dev, "Could not get TX clock\n");
		return PTR_ERR(gmac->tx_clk);
	}

	gmac->macphy_clk = devm_clk_get(dev, clk_phy);
	if (IS_ERR(gmac->macphy_clk)) {
		dev_err(dev, "Could not get macphy_clk clock\n");
		return PTR_ERR(gmac->macphy_clk);
	}

	gmac->clk_gate = devm_clk_get(dev, clk_gate);
	if (IS_ERR(gmac->clk_gate)) {
		dev_err(dev, "Could not get clock gate\n");
		return PTR_ERR(gmac->clk_gate);
	}

	ret = of_property_read_u32(np, "macphy-rate", &gmac->macphy_rate);
	if (ret < 0) {
		dev_err(dev, "Could not get macphy_rate, error!!!\n");
		goto err_remove_config_dt;
	}

	gmac->clk_enabled = 0;
#if 0
        ret = of_property_read_string(np, "macphy-type", &out_string);
        if (ret < 0) {
                 printk("Could not bulitin phy!\n");
        }else{
               printk("bulitin phy!\n");
               ingenic_xgmac_builtin_phy_init(pdev);
        }
#endif
	/* platform data specifying hardware features and callbacks.
	 * hardware features were copied from a1 drivers. */
	plat_dat->tx_coe = true;
	plat_dat->rx_coe = true;
	plat_dat->has_xgmac = true;
	plat_dat->tso_en = true;
	plat_dat->pmt = false;
	plat_dat->bsp_priv = gmac;
	plat_dat->init = ingenic_gmac_init;
	plat_dat->exit = ingenic_gmac_exit;
	plat_dat->fix_mac_speed = ingenic_fix_speed;

	/* Custom initialisation (if needed) */
	if (plat_dat->init) {
		ret = plat_dat->init(pdev, plat_dat);
		if (ret)
			goto err_remove_config_dt;
	}

	pr_debug("############### %s[%d]: \n",__func__,__LINE__);
	ret = stmmac_dvr_probe(&pdev->dev, plat_dat, &stmmac_res);
	if (ret)
		goto err_gmac_exit;
	pr_debug("############### %s[%d]: end,ret %d\n",__func__,__LINE__,ret);

	return 0;

err_gmac_exit:
	if (plat_dat->exit)
		plat_dat->exit(pdev, plat_dat->bsp_priv);
err_remove_config_dt:
	if (pdev->dev.of_node)
		stmmac_remove_config_dt(pdev, plat_dat);

	return ret;
}

static const struct of_device_id ingenic_dwmac_match[] = {
	{ .compatible = "ingenic,a1-gmac" },
	{ }
};
MODULE_DEVICE_TABLE(of, ingenic_dwmac_match);

static struct platform_driver ingenic_dwmac_driver = {
	.probe  = ingenic_gmac_probe,
	.remove = stmmac_pltfr_remove,
	.driver = {
		.name           = "ingenic-dwmac",
		.pm		= &stmmac_pltfr_pm_ops,
		.of_match_table = ingenic_dwmac_match,
	},
};
module_platform_driver(ingenic_dwmac_driver);

MODULE_AUTHOR("xhshen <nick.xhshen@ingenic.com>");
MODULE_DESCRIPTION("Ingenic DWMAC specific glue layer");
MODULE_LICENSE("GPL");
