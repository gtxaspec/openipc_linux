/*
   * linux/drivers/watchdog/jz_wdt.c - Ingenic Watchdog Controller driver
   *
   * Copyright (C) 2015 Ingenic Semiconductor Co., Ltd.
   * Written by Mick <dongyue.ye@ingenic.com>.
   *
   * This program is free software; you can redistribute it and/or modify
   * it under the terms of the GNU General Public License version 2 as
   * published by the Free Software Foundation.
   */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <jz_proc.h>
#include <soc/base.h>
#include <soc/tcu.h>

#define JZ_REG_WDT_TIMER_DATA     0x0
#define JZ_REG_WDT_COUNTER_ENABLE 0x4
#define JZ_REG_WDT_TIMER_COUNTER  0x8
#define JZ_REG_WDT_TIMER_CONTROL  0xC

#define JZ_WDT_CLOCK_PCLK 0x1
#define JZ_WDT_CLOCK_RTC  0x2
#define JZ_WDT_CLOCK_EXT  0x4

#define JZ_WDT_CLOCK_DIV_SHIFT   3

#define JZ_WDT_CLOCK_DIV_1    (0 << JZ_WDT_CLOCK_DIV_SHIFT)
#define JZ_WDT_CLOCK_DIV_4    (1 << JZ_WDT_CLOCK_DIV_SHIFT)
#define JZ_WDT_CLOCK_DIV_16   (2 << JZ_WDT_CLOCK_DIV_SHIFT)
#define JZ_WDT_CLOCK_DIV_64   (3 << JZ_WDT_CLOCK_DIV_SHIFT)
#define JZ_WDT_CLOCK_DIV_256  (4 << JZ_WDT_CLOCK_DIV_SHIFT)
#define JZ_WDT_CLOCK_DIV_1024 (5 << JZ_WDT_CLOCK_DIV_SHIFT)

#define DEFAULT_HEARTBEAT 5
#define MAX_HEARTBEAT     2048

static int WATCHDOG_STATE = 0;

static DEFINE_SPINLOCK(watchdog_lock);

static bool nowayout = WATCHDOG_NOWAYOUT;

module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout,
		 "Watchdog cannot be stopped once started (default="
		 __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

static unsigned int heartbeat = DEFAULT_HEARTBEAT;
module_param(heartbeat, uint, 0);
MODULE_PARM_DESC(heartbeat,
		"Watchdog heartbeat period in seconds from 1 to "
		__MODULE_STRING(MAX_HEARTBEAT) ", default "
		__MODULE_STRING(DEFAULT_HEARTBEAT));

struct jz_wdt_drvdata {
	struct watchdog_device wdt;
	void __iomem *base;
	struct clk *rtc_clk;
};

static int jz_wdt_ping(struct watchdog_device *wdt_dev)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);

	writew(0x0, drvdata->base + JZ_REG_WDT_TIMER_COUNTER);
	return 0;
}

static int jz_wdt_set_timeout(struct watchdog_device *wdt_dev,
					unsigned int new_timeout)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);
	unsigned int rtc_clk_rate;
	unsigned int timeout_value;
	unsigned short clock_div = JZ_WDT_CLOCK_DIV_1;

	/*rtc_clk_rate = clk_get_rate(drvdata->rtc_clk);*/
	rtc_clk_rate = 24000000 / 512;// becasue OPCR.ERCS select EXT/512

	timeout_value = rtc_clk_rate * new_timeout;
	while (timeout_value > 0xffff) {
		if (clock_div == JZ_WDT_CLOCK_DIV_1024) {
			/* Requested timeout too high;
			* use highest possible value. */
			timeout_value = 0xffff;
			break;
		}
		timeout_value >>= 2;
		clock_div += (1 << JZ_WDT_CLOCK_DIV_SHIFT);
	}
	/*printk("timeout = %d,timeout_value = %d,clock_div = %d,rtc_clk_rate = %d ----------\n",new_timeout,timeout_value,clock_div,rtc_clk_rate);*/

	writeb(0x0, drvdata->base + JZ_REG_WDT_COUNTER_ENABLE);
	writew(clock_div, drvdata->base + JZ_REG_WDT_TIMER_CONTROL);

	writew((u16)timeout_value, drvdata->base + JZ_REG_WDT_TIMER_DATA);
	writew(0x0, drvdata->base + JZ_REG_WDT_TIMER_COUNTER);
	writew(clock_div | JZ_WDT_CLOCK_RTC,
		drvdata->base + JZ_REG_WDT_TIMER_CONTROL);

	writeb(0x1, drvdata->base + JZ_REG_WDT_COUNTER_ENABLE);

	wdt_dev->timeout = new_timeout;
	return 0;
}

static int jz_wdt_start(struct watchdog_device *wdt_dev)
{
	unsigned long flags;
	spin_lock_irqsave(&watchdog_lock, flags);

	outl(1 << 16,TCU_IOBASE + TCU_TSCR);
	jz_wdt_set_timeout(wdt_dev, wdt_dev->timeout);

	spin_unlock_irqrestore(&watchdog_lock, flags);

	// Update the state to indicate the watchdog is now active
	WATCHDOG_STATE = 1;
	return 0;
}

static int jz_wdt_stop(struct watchdog_device *wdt_dev)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);

	unsigned long flags;
	spin_lock_irqsave(&watchdog_lock, flags);

	outl(1 << 16,TCU_IOBASE + TCU_TSCR);
	writew(0,drvdata->base + JZ_REG_WDT_TIMER_COUNTER);		//counter
	writew(65535,drvdata->base + JZ_REG_WDT_TIMER_DATA);	//data
	outl(1 << 16,TCU_IOBASE + TCU_TSSR);

	spin_unlock_irqrestore(&watchdog_lock, flags);

// Update the state to indicate the watchdog is now inactive
	WATCHDOG_STATE = 0;
	return 0;
}

static const struct watchdog_info jz_wdt_info = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
	.identity = "jz Watchdog",
};

static const struct watchdog_ops jz_wdt_ops = {
	.owner = THIS_MODULE,
	.start = jz_wdt_start,
	.stop = jz_wdt_stop,
	.ping = jz_wdt_ping,
	.set_timeout = jz_wdt_set_timeout,
};

// Read the timeout of the watchdog
static ssize_t watchdog_timeout_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	struct jz_wdt_drvdata *drvdata = PDE_DATA(file_inode(file));
	char timeout_str[20];
	int len;

	if (*ppos > 0 || count < sizeof(timeout_str)) {
		return 0;
	}

	len = snprintf(timeout_str, sizeof(timeout_str), "%u\n", drvdata->wdt.timeout);

	if (copy_to_user(buf, timeout_str, len)) {
		return -EFAULT;
	}

	*ppos = len; // Update the position for next read
	return len;
}

// Read the state of the watchdog
static ssize_t watchdog_state_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	char state[2];
	state[0] = WATCHDOG_STATE ? '1' : '0';
	state[1] = '\n';

	if (*ppos > 0 || count < 2) {
		return 0;
	}

	if (copy_to_user(buf, state, 2)) {
		return -EFAULT;
	}

	*ppos = 2;
	return 2;
}

static ssize_t watchdog_state_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos) {
	struct jz_wdt_drvdata *drvdata = PDE_DATA(file_inode(file));
	char buf[3]; // One for '0'/'1', one for potential newline, and one for null terminator

	if (!drvdata)
		return -ENODEV;

	if (count < 1 || count > sizeof(buf) - 1)
		return -EINVAL;

	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0'; // Null-terminate the string

	// Trim newline character if present
	if (buf[count - 1] == '\n')
		buf[count - 1] = '\0';

	if (strcmp(buf, "1") == 0) {
		// Enable the watchdog
		if (!WATCHDOG_STATE) {
			jz_wdt_start(&drvdata->wdt);
		}
	} else if (strcmp(buf, "0") == 0) {
		// Disable the watchdog
		if (WATCHDOG_STATE) {
			jz_wdt_stop(&drvdata->wdt);
		}
	} else {
		return -EINVAL; // Invalid input
	}

	*ppos += count;
	return count;
}

static ssize_t watchdog_timeout_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos) {
	struct jz_wdt_drvdata *drvdata = PDE_DATA(file_inode(file));
	char buf[12];
	unsigned long new_timeout;
	int err;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0'; // Null-terminate the string

	err = kstrtoul(buf, 10, &new_timeout);
	if (err)
		return err;

	if (new_timeout < drvdata->wdt.min_timeout || new_timeout > drvdata->wdt.max_timeout)
		return -EINVAL;

	// Update the stored timeout, but only change the hardware setting if the watchdog is active
	drvdata->wdt.timeout = new_timeout;

	if (WATCHDOG_STATE) {
		jz_wdt_set_timeout(&drvdata->wdt, new_timeout);
	}

	*ppos = count;
	return count;
}

static ssize_t watchdog_cmd_set(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
	unsigned int cmd_value;
	unsigned int reg04 = 0, reglc = 0;

	char *buf = kzalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[count] = '\0'; // Null terminate

	if (kstrtouint(buf, 0, &cmd_value))
		return -EINVAL;

	mdelay(1000);
	if(1 == cmd_value) {
		// This works on =<T31
		int time = 24000000 / 64 * 4 / 1000;
		if(time > 65535)
				time = 65535;

		outl(1 << 16,TCU_IOBASE + 0x0c);

		outl(0,WDT_IOBASE + 0x08);          //counter
		outl(time,WDT_IOBASE + 0x00);        //data
		outl((3<<3 | 1<<1),WDT_IOBASE + 0x0c);
		outl(0,WDT_IOBASE + 0x04);
		outl(1,WDT_IOBASE + 0x04);

		printk("watchdog reboot system!!!\n");
		}

	kfree(buf);
	return count;
}

static ssize_t watchdog_reset_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	const char *msg = "Write 1 to force a reset via the watchdog\n";
	int len = strlen(msg);

	// Simple read - only allows reading once
	if (*ppos > 0 || count < len)
		return 0;

	if (copy_to_user(buf, msg, len))
		return -EFAULT;

	*ppos += len; // Update the read position
	return len;
}

static const struct file_operations watchdog_state_fops = {
	.owner = THIS_MODULE,
	.read = watchdog_state_read,
	.write = watchdog_state_write,

};

static const struct file_operations watchdog_timeout_fops = {
	.owner = THIS_MODULE,
	.read = watchdog_timeout_read,
	.write = watchdog_timeout_write,
};

static const struct file_operations watchdog_cmd_fops = {
	.owner = THIS_MODULE,
	.read = watchdog_reset_read,
	.write = watchdog_cmd_set,
};

static int jz_wdt_probe(struct platform_device *pdev)
{
	struct jz_wdt_drvdata *drvdata;
	struct watchdog_device *jz_wdt;
	struct resource	*res;
	struct proc_dir_entry *proc;
	int ret;

	drvdata = devm_kzalloc(&pdev->dev, sizeof(struct jz_wdt_drvdata),
				   GFP_KERNEL);
	if (!drvdata) {
		dev_err(&pdev->dev, "Unable to alloacate watchdog device\n");
		return -ENOMEM;
	}

	if (heartbeat < 1 || heartbeat > MAX_HEARTBEAT)
		heartbeat = DEFAULT_HEARTBEAT;

	jz_wdt = &drvdata->wdt;
	jz_wdt->info = &jz_wdt_info;
	jz_wdt->ops = &jz_wdt_ops;
	jz_wdt->timeout = heartbeat;
	jz_wdt->min_timeout = 1;
	jz_wdt->max_timeout = MAX_HEARTBEAT;
	watchdog_set_nowayout(jz_wdt, nowayout);
	watchdog_set_drvdata(jz_wdt, drvdata);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	drvdata->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(drvdata->base)) {
		ret = PTR_ERR(drvdata->base);
		goto err_out;
	}

#ifdef CONFIG_SOC_T23
	drvdata->rtc_clk = clk_get(NULL, "pclk");
#else
	drvdata->rtc_clk = clk_get(NULL, "rtc");
#endif
	if (IS_ERR(drvdata->rtc_clk)) {
		dev_err(&pdev->dev, "cannot find RTC clock\n");
		ret = PTR_ERR(drvdata->rtc_clk);
		goto err_out;
	}

	ret = watchdog_register_device(&drvdata->wdt);
	if (ret < 0)
		goto err_disable_clk;

	platform_set_drvdata(pdev, drvdata);

	/* proc info */
	proc = jz_proc_mkdir("watchdog");
	if (!proc) {
		printk("create jz-wdt proc entry failed!\n");
	}

	proc_create_data("state", S_IRUGO | S_IWUGO, proc, &watchdog_state_fops, drvdata);
	proc_create_data("timeout", S_IRUGO, proc, &watchdog_timeout_fops, drvdata);
	proc_create_data("reset", S_IRUGO | S_IWUGO, proc, &watchdog_cmd_fops, NULL);

	printk(KERN_INFO "jz-wdt: watchdog initialized\n");

	return 0;

err_disable_clk:
	clk_put(drvdata->rtc_clk);
err_out:
	return ret;
}

static int jz_wdt_remove(struct platform_device *pdev)
{
	struct jz_wdt_drvdata *drvdata = platform_get_drvdata(pdev);

	jz_wdt_stop(&drvdata->wdt);
	watchdog_unregister_device(&drvdata->wdt);
	clk_put(drvdata->rtc_clk);

	return 0;
}

static struct platform_driver jz_wdt_driver = {
	.probe = jz_wdt_probe,
	.remove = jz_wdt_remove,
	.driver = {
		.name = "jz-wdt",
		.owner	= THIS_MODULE,
	},
};

static int __init jzwdt_init(void)
{
	return platform_driver_register(&jz_wdt_driver);
}

static void __exit jzwdt_exit(void)
{
	// Remove proc entries
	remove_proc_entry("watchdog/reset", NULL);
	remove_proc_entry("watchdog/timeout", NULL);
	remove_proc_entry("watchdog/state", NULL);
	remove_proc_entry("watchdog", NULL); // Parent directory
	return platform_driver_unregister(&jz_wdt_driver);
}

module_init(jzwdt_init);
module_exit(jzwdt_exit);

MODULE_DESCRIPTION("jz Watchdog Driver");
MODULE_LICENSE("GPL");
