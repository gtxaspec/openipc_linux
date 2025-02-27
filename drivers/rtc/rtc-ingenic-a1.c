/*
 * Copyright (C) 2015 Ingenic Semiconductor Co., Ltd.
 * Author: cli <chen.li@ingenic.com>
 *
 * Real Time Clock interface for Ingenic's SOC, such as X1000,
 * and so on. (kernel.4.4)
 *
 * Base on:rtc-generic: RTC driver using the generic RTC abstraction
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

/*#define DEBUG*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <jz_proc.h>

struct ingenic_rtc_base{
	unsigned long long  u64basesec;
	unsigned int		u32rtcsec;
};

#define RTC_BASETIME_GET _IOR('p', 0x80, struct ingenic_rtc_base)  /* Get base time */
#define RTC_BASETIME_SET _IOW('p', 0x81, struct ingenic_rtc_base)  /* Set base time */

/*
 * RTC registers offset address definition
 */
#define RTC_VERSION		(0x00)	/* r, 32, 0x00000081 */
#define RTC_RTCSR		(0x04)	/* r, 32, 0x???????? */
#define RTC_CLKCNTEN		(0x10)	/* rw, 32, 0x???????? */
#define RTC_CLKCNTINFO		(0x18)	/* r, 32, 0x0??????? */

struct ingenic_rtc_device {
	struct rtc_device *rtc;
	struct device *dev;
	struct mutex  mutex;
	void __iomem *reg_base;
	struct clk *rtc_gate;
	unsigned long long  u64basesec;
	unsigned int		u32rtcsec;
	struct proc_dir_entry *proc;
	int enable;
};

static struct ingenic_rtc_device *m_rtc = NULL;

static int ingenic_rtc_read(struct ingenic_rtc_device* rtc, int reg)
{
	return readl(rtc->reg_base + reg);
}

static int ingenic_rtc_write(struct ingenic_rtc_device* rtc, int reg, int val)
{
	writel(val, rtc->reg_base + reg);
	return 0;
}

static int ingenic_rtc_check_enable(struct ingenic_rtc_device *rtc)
{
	unsigned int clkcnten = 0;
	unsigned int clkcnt = 0;
	unsigned int clkcnt2 = 0;

	if(rtc->enable == 0){
		clkcnten = ingenic_rtc_read(rtc, RTC_CLKCNTEN);
		if(clkcnten == 0)
			ingenic_rtc_write(rtc, RTC_CLKCNTEN, 1);

		clkcnt = ingenic_rtc_read(rtc, RTC_CLKCNTINFO);
		msleep(1);
		clkcnt2 = ingenic_rtc_read(rtc, RTC_CLKCNTINFO);
		rtc->enable = clkcnt == clkcnt2 ? 0 : 1;
	}
	return rtc->enable;
}


static int ingenic_rtc_get_seconds(struct ingenic_rtc_device *rtc, uint32_t *seconds)
{
	uint32_t secs, secs2;
	int timeout = 10;

	/* If the seconds register is read while it is updated, it can contain a
	 * bogus value. This can be avoided by making sure that two consecutive
	 * reads have the same value.
	 */
	secs = readl(rtc->reg_base + RTC_RTCSR);
	secs = readl(rtc->reg_base + RTC_RTCSR);

	while (secs != secs2 && --timeout) {
		secs = secs2;
		secs2 = readl(rtc->reg_base + RTC_RTCSR);
	}

	if (timeout == 0)
		return -EIO;

	*seconds = secs;
	return 0;
}


static int ingenic_rtc_get_time(struct device *dev, struct rtc_time *tm)
{
	struct ingenic_rtc_device *rtc = dev_get_drvdata(dev);
	uint32_t u32sec = 0;
	uint64_t u64sec = 0;
	int ret = 0;

	mutex_lock(&rtc->mutex);
	ret = ingenic_rtc_check_enable(rtc);
	if(!ret)
		goto out;

	ret = ingenic_rtc_get_seconds(rtc, &u32sec);
	if(ret)
		goto out;

	if(u32sec < rtc->u32rtcsec){
		u64sec = 1;
		u64sec = u64sec << 32;
	}

	u64sec = u64sec + u32sec - rtc->u32rtcsec;
	u64sec += rtc->u64basesec;
	mutex_unlock(&rtc->mutex);

	rtc_time64_to_tm((time64_t)u64sec, tm);

	return rtc_valid_tm(tm);

out:
	mutex_unlock(&rtc->mutex);
	return -EIO;
}

static int ingenic_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct ingenic_rtc_device *rtc = dev_get_drvdata(dev);
	uint32_t u32sec = 0;
	int ret = 0;

	/*if(rtc_valid_tm(tm))*/
		/*return -EINVAL;*/

	mutex_lock(&rtc->mutex);
	ret = ingenic_rtc_check_enable(rtc);
	if(!ret)
		goto out;

	ret = ingenic_rtc_get_seconds(rtc, &u32sec);
	if(ret)
		goto out;

	rtc->u32rtcsec = u32sec;
	rtc->u64basesec = rtc_tm_to_time64(tm);

	mutex_unlock(&rtc->mutex);

	return 0;

out:
	mutex_unlock(&rtc->mutex);
	return -EIO;
}

static int ingenic_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
	struct ingenic_rtc_device *rtc = dev_get_drvdata(dev);
	struct ingenic_rtc_base rtc_base;
	int ret = 0;
	switch (cmd) {
	case RTC_BASETIME_SET:
		if (copy_from_user((void *)&rtc_base, (void __user *)arg,
					sizeof(rtc_base)))
			return -EFAULT;
		mutex_lock(&rtc->mutex);
		rtc->u32rtcsec = rtc_base.u32rtcsec;
		rtc->u64basesec = rtc_base.u64basesec;
		mutex_unlock(&rtc->mutex);
		break;
	case RTC_BASETIME_GET:
		mutex_lock(&rtc->mutex);
		rtc_base.u32rtcsec = rtc->u32rtcsec;
		rtc_base.u64basesec = rtc->u64basesec;
		mutex_unlock(&rtc->mutex);
		if (copy_to_user((void __user *)arg, (void *)&rtc_base,
					sizeof(rtc_base)))
			return -EFAULT;
		break;
	default:
		return -ENOIOCTLCMD;
	}
	return ret;
}

static int ingenic_rtc_rtc_proc(struct device *dev, struct seq_file *seq)
{
	/*struct ingenic_rtc_device *rtc = dev_get_drvdata(dev);*/

	seq_printf(seq, "Accident power down\n");
	return 0;
}

static const struct rtc_class_ops ingenic_rtc_ops = {
	.read_time = ingenic_rtc_get_time,
	.set_time = ingenic_rtc_set_time,
	.ioctl = ingenic_rtc_ioctl,
	.proc = ingenic_rtc_rtc_proc,
};


/********************** proc debug *******************************/
static ssize_t debug_rtc_set(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
	int ret = 0;
	struct seq_file *m = file->private_data;
	struct ingenic_rtc_device *rtc = (void *)(m->private);
	unsigned long long u64base;
	unsigned int u32rtc;
	char buf[64];

	count = count > 64 ? 64 : count;

	if(copy_from_user(buf, buffer, count)){
		return -EFAULT;
	}

	ret = sscanf(buf, "base:%llu,%u", &u64base, &u32rtc);
	if(ret != 2)
		return 0;

	mutex_lock(&rtc->mutex);
	rtc->u32rtcsec = u32rtc;
	rtc->u64basesec = u64base;
	mutex_unlock(&rtc->mutex);
	return count;
}

static int debug_rtc_show(struct seq_file *m, void *v)
{
	int len = 0;
	struct ingenic_rtc_device *rtc = (void *)(m->private);
	/*printk("base:%llu,%u\n", rtc->u64basesec, rtc->u32rtcsec);*/
	mutex_lock(&rtc->mutex);
	len += seq_printf(m ,"base:%llu,%u\n", rtc->u64basesec, rtc->u32rtcsec);
	mutex_unlock(&rtc->mutex);
	return 0;
}

static int debug_rtc_open(struct inode *inode, struct file *file)
{
	return single_open_size(file, debug_rtc_show, PDE_DATA(inode), 512);
}

static struct file_operations debug_ops ={
	.read = seq_read,
	.open = debug_rtc_open,
	.llseek = seq_lseek,
	.release = single_release,
	.write = debug_rtc_set,
};

static int __init ingenic_rtc_probe(struct platform_device *pdev)
{
	struct ingenic_rtc_device *ingenic_rtc;
	struct resource *res;
	struct clk *rtc_gate;

	ingenic_rtc = devm_kzalloc(&pdev->dev, sizeof(struct ingenic_rtc_device), GFP_KERNEL);
	if (!ingenic_rtc)
		return -ENOMEM;

	ingenic_rtc->dev = &pdev->dev;
	mutex_init(&ingenic_rtc->mutex);

	rtc_gate = clk_get(&pdev->dev, "gate_rtc");
	if (IS_ERR(rtc_gate))
		return PTR_ERR(rtc_gate);
	clk_prepare_enable(rtc_gate);
	clk_put(rtc_gate);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOMEM;

	ingenic_rtc->reg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ingenic_rtc->reg_base))
		return PTR_ERR(ingenic_rtc->reg_base);

	device_init_wakeup(&pdev->dev, true);
	platform_set_drvdata(pdev, ingenic_rtc);

	ingenic_rtc->rtc = devm_rtc_device_register(&pdev->dev, "rtc-ingenic",
					&ingenic_rtc_ops, THIS_MODULE);
	if (IS_ERR(ingenic_rtc->rtc))
		return PTR_ERR(ingenic_rtc->rtc);

	ingenic_rtc->proc = jz_proc_mkdir("rtc");
	if (!ingenic_rtc->proc) {
		goto failed_to_proc;
	}else{
		proc_create_data("base", S_IRUGO, ingenic_rtc->proc, &debug_ops, (void *)ingenic_rtc);
	}

	m_rtc = ingenic_rtc;
	return 0;
failed_to_proc:

	return 0;
}

static void __exit ingenic_rtc_remove(struct platform_device *pdev)
{
	struct ingenic_rtc_device *rtc = platform_get_drvdata(pdev);
	if(!rtc)
		return;
	proc_remove(rtc->proc);
	if(m_rtc)
		devm_kfree(&pdev->dev, m_rtc);
	m_rtc = NULL;
}

static const struct of_device_id ingenic_rtc_of_match[] = {
	{ .compatible = "ingenic,rtc", .data = NULL, },
	{},
};
MODULE_DEVICE_TABLE(of, ingenic_rtc_of_match);

#ifdef CONFIG_PM
static int ingenic_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int ingenic_rtc_resume(struct platform_device *pdev)
{
	return 0;
}

#else
#define ingenic_rtc_suspend NULL
#define ingenic_rtc_resume NULL
#endif

static struct platform_driver ingenic_rtc_driver = {
	.driver	= {
		.name = "rtc-ingenic",
		.of_match_table = ingenic_rtc_of_match,
	},
	.remove		= __exit_p(ingenic_rtc_remove),
	.suspend	= ingenic_rtc_suspend,
	.resume		= ingenic_rtc_resume,
};
module_platform_driver_probe(ingenic_rtc_driver, ingenic_rtc_probe);

MODULE_AUTHOR("Nick <nick.xhshen@ingenic.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ingenic RTC driver");
MODULE_ALIAS("platform:rtc-ingenic");
