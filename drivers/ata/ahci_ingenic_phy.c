#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <soc/gpio.h>
#include <linux/proc_fs.h>
#include "dwc_sata.h"

#define SATA_MAGIC 'D'
#define IOCTL_SATA_PHY_EN _IOW(SATA_MAGIC,110,unsigned int)
#ifdef  DEBUG
#define SATA_DEBUG(format, ...) do{printk(format, ## __VA_ARGS__);}while(0)
#else
#define SATA_DEBUG(format, ...) do{ } while(0)
#endif
#define SATA_PHY_ADDR		0x10
#define writel_with_flush(a,b)	do { writel(a,b); readl(b); } while (0)


struct i2c_client *g_sata_phy0_i2c_client = NULL;
struct i2c_client *g_sata_phy1_i2c_client = NULL;

struct sata_driver
{
	struct miscdevice miscdev;
	struct i2c_client *client;
	struct mutex mutex;
};

static unsigned int sata_phy_i2c_read(struct i2c_client *client,unsigned char reg)
{
	unsigned char rxbuf[1] = {reg};
	unsigned char txbuf[1] = {0};
	struct i2c_msg xfer[2] = {
		[0]	= {
			.addr = client->addr,
			.flags = 0,
			.len = sizeof(rxbuf),
			.buf = rxbuf,
		},
		[1]	= {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = sizeof(txbuf),
			.buf = txbuf,
		}
	};

	i2c_transfer(client->adapter, xfer, ARRAY_SIZE(xfer));
	return (unsigned int)txbuf[0];
}

static void sata_phy_i2c_write(struct i2c_client *client,unsigned char reg, unsigned char value) {
	unsigned char txbuf[2] = {reg,value};
	struct i2c_msg xfer[1] = {
		[0]	= {
			.addr = client->addr,
			.flags = 0,
			.len = sizeof(txbuf),
			.buf = txbuf,
		}
	};
	i2c_transfer(client->adapter,xfer,ARRAY_SIZE(xfer));
}

static ssize_t sata_phy_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	return 0;
}

static int sata_phy_open(struct inode *inode, struct file *file)
{
	SATA_DEBUG("%s - %d\n",__func__,__LINE__);
	return 0;
}

static int sata_phy_release(struct inode *inode, struct file *file)
{
	SATA_DEBUG("%s - %d\n",__func__,__LINE__);
	return 0;
}

static long sata_phy_ioctl(struct file *file, unsigned int cmd, unsigned long arg )
{
	int ret = 0;
	struct miscdevice  *dev = file->private_data;
	struct sata_driver *sata = container_of(dev,struct sata_driver,miscdev);
	mutex_lock(&sata->mutex);

	switch(cmd) {
		case IOCTL_SATA_PHY_EN:
			{
				SATA_DEBUG("sata phy 0x0a= %x\n",vga_phy_i2c_read(cga->client,0x0a));
			}
			break;
		default:
			SATA_DEBUG("%s - - %d\n invalid command",__func__,__LINE__);
			ret = - EFAULT;
			break;
	}
	mutex_lock(&sata->mutex);
	return ret;
}

static const struct file_operations sata_phy_fops = {
	.owner		= THIS_MODULE,
	.write		= sata_phy_write,
	.open		= sata_phy_open,
	.unlocked_ioctl	= sata_phy_ioctl,
	.release	= sata_phy_release,
};

static void sata_phy_dump(struct i2c_client *i2c)
{
	printk("---------%s--------\n",i2c->name);
	i2c->addr = 0x10;
	printk("0x1000 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x00));
	printk("0x100e = 0x%04x\n", sata_phy_i2c_read(i2c, 0x0e));
	printk("0x101a = 0x%04x\n", sata_phy_i2c_read(i2c, 0x1a));
	printk("0x1023 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x23));
	printk("0x1024 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x24));
	printk("0x1031 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x31));
	printk("0x1034 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x34));
	printk("0x1037 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x37));
	printk("0x1038 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x38));
	printk("0x1060 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x60));
	printk("0x1061 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x61));
	printk("0x1063 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x63));
	printk("0x1064 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x64));
	printk("0x1067 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x67));
	printk("0x107f = 0x%04x\n", sata_phy_i2c_read(i2c, 0x7f));
	printk("0x1084 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x84));
	printk("0x1087 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x87));
	printk("0x1088 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x88));
	printk("0x1089 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x89));
	printk("0x1096 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x96));
	printk("0x1097 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x97));
	printk("0x1098 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x98));
	printk("0x10b8 = 0x%04x\n", sata_phy_i2c_read(i2c, 0xb8));
	printk("0x10b9 = 0x%04x\n", sata_phy_i2c_read(i2c, 0xb9));

	i2c->addr = 0x11;
	printk("0x1112 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x12));
	printk("0x1113 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x13));
	printk("0x1145 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x45));
	printk("0x1147 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x47));
	printk("0x1148 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x48));
	printk("0x114a = 0x%04x\n", sata_phy_i2c_read(i2c, 0x4a));
	printk("0x1173 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x73));
	printk("0x1176 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x76));
	printk("0x1177 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x77));
	printk("0x1178 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x78));
	printk("0x1179 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x79));
	printk("0x117c = 0x%04x\n", sata_phy_i2c_read(i2c, 0x7c));

	i2c->addr = 0x30;
	printk("0x3000 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x00));
	printk("0x3017 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x17));
	printk("0x301a = 0x%04x\n", sata_phy_i2c_read(i2c, 0x1a));
	printk("0x3054 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x54));
	printk("0x3057 = 0x%04x\n", sata_phy_i2c_read(i2c, 0x57));

}
#if 0
static void sata_controller_dump(void)
{
	printk("---------sata controller global--------\n");
	#define PRINT_REG(NAME) printk("0x%08x, %-32s: 0x%08x\n", NAME, #NAME, readl(NAME));
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
		printk("---------sata controller port 1--------\n");
		PRINT_REG(SATA_P_CLB(1))
		PRINT_REG(SATA_P_CLBU(1))
		PRINT_REG(SATA_P_FB(1))
		PRINT_REG(SATA_P_FBU(1))
		PRINT_REG(SATA_P_IS(1))
		PRINT_REG(SATA_P_IE(1))
		PRINT_REG(SATA_P_CMD(1))
		PRINT_REG(SATA_P_TFD(1))
		PRINT_REG(SATA_P_SIG(1))
		PRINT_REG(SATA_P_SSTS(1))
		PRINT_REG(SATA_P_SCTL(1))
		PRINT_REG(SATA_P_SERR(1))
		PRINT_REG(SATA_P_SACT(1))
		PRINT_REG(SATA_P_CI(1))
		PRINT_REG(SATA_P_SNTF(1))
		PRINT_REG(SATA_P_FBS(1))
		PRINT_REG(SATA_P_DEVSLP(1))
		PRINT_REG(SATA_P_DMACR(1))
		PRINT_REG(SATA_P_PHYCR(1))
		PRINT_REG(SATA_P_PHYSR(1))
#undef PRINT_REG
}
#endif
#define SBUFF_SIZE     128
static ssize_t sata_proc_write(struct file *filp, const char __user * buff, size_t len, loff_t * offset)
{
	int ret;
	unsigned int control_addr;
	unsigned int control_value;
	unsigned char *p = NULL;
	unsigned char *sbuffsata = NULL;
	unsigned char cmd_i2c[3], cmd_func[5];

	struct i2c_client *i2c = filp->private_data;

	sbuffsata = (unsigned char *)kzalloc(SBUFF_SIZE * sizeof(char), GFP_KERNEL);
	if (sbuffsata == NULL) {
		printk("malloc memory fail\n");
		kfree(sbuffsata);
		return -EFAULT;
	}

	memset(sbuffsata, 0, SBUFF_SIZE);
	len = len < SBUFF_SIZE ? len : SBUFF_SIZE;
	ret = copy_from_user(sbuffsata, buff, len);

	if (ret) {
        	printk(KERN_INFO "[+sata_proc]: copy_from_user() error!\n");
        	return -EFAULT;
    	}
    	p = sbuffsata;

	sscanf(p,"%s %s", cmd_i2c, cmd_func);
	if (!strcmp(cmd_i2c, "i2c")) {
		if (!strcmp(cmd_func, "write")) {
			sscanf(p,"%s %s %x %x", cmd_i2c, cmd_func, &control_addr, &control_value);
			i2c->addr = ((char)(control_addr >> 8));
			sata_phy_i2c_write(i2c, (unsigned char)control_addr, (unsigned char)control_value);
		}
		else if (!strcmp(cmd_func, "read")) {
			sscanf(p,"%s %s %x", cmd_i2c, cmd_func, &control_addr);
			i2c->addr = (char)(control_addr >> 8);
			printk("0x%x = 0x%x\n", control_addr, sata_phy_i2c_read(i2c, (unsigned char)control_addr));
		}
		else if (!strcmp(cmd_func, "dump")) {
			sata_phy_dump(i2c);
		}
		else {
			printk("there have no such command for I2C debug in proc!\n");
		}
	}
	else
	{
		printk("error command,please input right command!\n");
		return -EFAULT;
	}

	kfree(sbuffsata);
	return len;
}

static int sata_proc_open(struct inode *inode, struct file *file)
{
	file->private_data = PDE_DATA(inode);
	return 0;
}

static struct file_operations sata_devices_fileops = {
	.owner		= THIS_MODULE,
	.open		= sata_proc_open,
	.write		= sata_proc_write,
};

static int sata_phy0_i2c_op_init(struct i2c_client *i2c)
{
	static struct proc_dir_entry *proc_sata_dir;
	static struct proc_dir_entry *entry;
	proc_sata_dir = proc_mkdir("sata",NULL);
	if (!proc_sata_dir)
 		return -ENOMEM;

	entry = proc_create_data("sata_phy0_test", 0666, proc_sata_dir, &sata_devices_fileops, i2c);
	if (!entry) {
		printk("%s:create proc_create error!\n",__func__);
		return -1;
	}
	return 0;
}

static int sata_phy1_i2c_op_init(struct i2c_client *i2c)
{
	static struct proc_dir_entry *proc_sata_dir;
	static struct proc_dir_entry *entry;
	proc_sata_dir = proc_mkdir("sata1",NULL);
	if (!proc_sata_dir)
 		return -ENOMEM;

	entry = proc_create_data("sata_phy1_test", 0666, proc_sata_dir, &sata_devices_fileops, i2c);
	if (!entry) {
		printk("%s:create proc_create error!\n",__func__);
		return -1;
	}
	return 0;
}

int sata_phy_set_gen_mode(int port_num, int mode)
{
	struct i2c_client *i2c;

	printk("%s,%d: port_num = %d, mode = %d\n",
			__func__, __LINE__, port_num ,mode);
	if (0 == port_num) {
		if (!g_sata_phy0_i2c_client) {
			printk("%s,%d: error!", __func__, __LINE__);
			return -1;
		}
		i2c = g_sata_phy0_i2c_client;
	} else {
		if (!g_sata_phy1_i2c_client) {
			printk("%s,%d: error!", __func__, __LINE__);
			return -1;
		}
		i2c = g_sata_phy1_i2c_client;
	}

	if (2 == mode) {
		i2c->addr = 0x10;
		sata_phy_i2c_write(i2c, 0x23, 0x00);
	} else {
		i2c->addr = 0x10;
		sata_phy_i2c_write(i2c, 0x34, 0xcc);
		sata_phy_i2c_write(i2c, 0x23, 0x10);
		sata_phy_i2c_write(i2c, 0x24, 0x1e);
		i2c->addr = 0x11;
		sata_phy_i2c_write(i2c, 0x77, 0x41);
		sata_phy_i2c_write(i2c, 0x13, 0xe3);
		sata_phy_i2c_write(i2c, 0x12, 0x11);
		i2c->addr = 0x30;
		sata_phy_i2c_write(i2c, 0x17, 0xf7);
	}
	return 0;
}

#define SATA_GEN 2
//#define SATA_QUES_DISK

static int sata_phy0_i2c_probe(struct i2c_client *i2c,const struct i2c_device_id *id)
{
	int ret = 0, i = 0;
	struct sata_driver *sata = NULL;
	unsigned int data = 0;
	unsigned int srbc = 0;
	sata = kzalloc(sizeof(struct sata_driver), GFP_KERNEL);
	if (!sata) {
		printk("alloc sata driver error\n");
		return -ENOMEM;
	}
	ret = sata_phy0_i2c_op_init(i2c);
	if (ret)
		printk("proc %s create dir and data fail!\n", i2c->name);
	printk("%s:  i2c = %p\n", __func__, i2c);

	sata->client = i2c;
#if 1
	/*CPM config
	CLKGR1:bit11  open
	SRBS0: bit18  reset
	*/
	srbc  = *(volatile unsigned int *)0xb00000f0;
	srbc |= (1 << 18);
	*(volatile unsigned int *)0xb00000f0 = srbc;
	msleep(100);
	srbc &= ~(1 << 18);
	*(volatile unsigned int *)0xb00000f0 = srbc;
#endif

	//PE12:set bist mode(0:off)
	gpio_request(GPIO_PE(12), "sata phy0 bist en");
	gpio_direction_output(GPIO_PE(12), 0);
	//chipsel
	gpio_request(GPIO_PE(13), "chipsel0");
	gpio_request(GPIO_PE(14), "chipsel1");
	gpio_direction_output(GPIO_PE(13), 0);
	gpio_direction_output(GPIO_PE(14), 1);
	//hard reset
	ret = gpio_request(GPIO_PA(15), "sata phyi0 reset");
	if (ret)
		printk("sata phy1 gpio request error %d\n", ret);
	gpio_direction_output(GPIO_PA(15), 1);
	msleep(200);
	gpio_direction_output(GPIO_PA(15), 0);
	msleep(200);
	gpio_direction_output(GPIO_PA(15), 1);

	i2c->addr = 0x10;
	/*
	  bit28 power_reset_n
	  bit20 offline
	  bit12 host_sel
	  bit9  spdsel select
	  bit8  spdsel

	  bit24 force_ready
	  bit16 listen
	  bit0  idle_en
	  bit5  p0_phy_reset_n select
	  bit4  p0_phy_reset_n
	 */
	data = *(volatile unsigned int *)0xb30d0178;
	printk("Before config,PHYCR is 0x%04x\n",data);
#if (SATA_GEN == 2)
	data |= (1<<12)|(1<<8)|(0<<31);//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#else
	data |= (1<<12)|(0<<8)|(0<<31);//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#endif
	*(volatile unsigned int *)0xb30d0178 = data;
	printk("reg 0x130d0178:0x%08x\n",data);

	msleep(300);

	i2c->addr = 0x10;
#if (SATA_GEN == 2)
	sata_phy_i2c_write(i2c, 0x24, 0x1e);
#else
	sata_phy_i2c_write(i2c, 0x34, 0xcc);
	sata_phy_i2c_write(i2c, 0x23, 0x10);
	sata_phy_i2c_write(i2c, 0x24, 0x1e);
	i2c->addr = 0x11;
	sata_phy_i2c_write(i2c, 0x77, 0x41);
	sata_phy_i2c_write(i2c, 0x13, 0xe3);
	sata_phy_i2c_write(i2c, 0x12, 0x11);
	i2c->addr = 0x30;
	sata_phy_i2c_write(i2c, 0x17, 0xf7);
#endif

	i2c->addr = 0x30;
	sata_phy_i2c_write(i2c, 0x00, 0x4e);
	sata_phy_i2c_write(i2c, 0x1a, 0x7e);
	sata_phy_i2c_write(i2c, 0x54, 0x63);
	sata_phy_i2c_write(i2c, 0x57, 0xe0);

	i2c->addr = 0x10;
	sata_phy_i2c_write(i2c, 0x60, 0x01);
	sata_phy_i2c_write(i2c, 0x64, 0x01);
	sata_phy_i2c_write(i2c, 0x7f, 0x01);
	sata_phy_i2c_write(i2c, 0x87, 0x0f);
	sata_phy_i2c_write(i2c, 0x88, 0xff);
	sata_phy_i2c_write(i2c, 0x89, 0xff);
	sata_phy_i2c_write(i2c, 0x84, 0x28);

	sata_phy_i2c_write(i2c, 0x38, 0x07);
	sata_phy_i2c_write(i2c, 0x37, 0x45);
	sata_phy_i2c_write(i2c, 0x31, 0x0);
	sata_phy_i2c_write(i2c, 0x61, 0x03);
	sata_phy_i2c_write(i2c, 0x67, 0x01);
#ifdef SATA_QUES_DISK
	sata_phy_i2c_write(i2c, 0xb8, 0x60);
	sata_phy_i2c_write(i2c, 0xb9, 0x06);
#endif

	i2c->addr = 0x11;
	sata_phy_i2c_write(i2c, 0x47, 0x02);
	sata_phy_i2c_write(i2c, 0x73, 0x4e);
	sata_phy_i2c_write(i2c, 0x79, 0x21);
	sata_phy_i2c_write(i2c, 0x7c, 0x01);

	i2c->addr = 0x10;
	sata_phy_i2c_write(i2c, 0x0e, 0x02);

	data = *(volatile unsigned int *)0xb30d0178;
	data |= (1<<4) |(0<<24) | (0<<16)  | 0;
	data |= (1 << 28) | (1<<20);
	printk("reg 0x130d0178:0x%08x\n",data);
	*(volatile unsigned int *)0xb30d0178 = data;
	for(i = 0; i < 5;i++)
		printk("after IIC write value is 0x%4x\n", *(volatile unsigned int *) 0xb30d00d4);
	sata_phy_dump(i2c);
	mutex_init(&sata->mutex);
	i2c_set_clientdata(i2c, sata);

	return 0;
}

static int sata_phy1_i2c_probe(struct i2c_client *i2c,const struct i2c_device_id *id)
{
	int ret = 0;
	struct sata_driver *sata = NULL;
	unsigned int data = 0;
	sata = kzalloc(sizeof(struct sata_driver), GFP_KERNEL);
	if (!sata) {
		printk("alloc sata driver error\n");
		return -ENOMEM;
	}

	ret = sata_phy1_i2c_op_init(i2c);
	if (ret)
		printk("proc %s create dir and data fail!\n",i2c->name);

	printk("%s:  i2c = %p\n", __func__, i2c);
	sata->client = i2c;

	//PB29:set bist mode(0:off)
	gpio_request(GPIO_PB(29), "sata phy1 bist en");
	gpio_direction_output(GPIO_PB(29), 0);
	//chipsel
	gpio_request(GPIO_PB(27), "chipsel0");
	gpio_request(GPIO_PA(30), "chipsel1");
	gpio_direction_output(GPIO_PB(27), 0);
	gpio_direction_output(GPIO_PA(30), 1);
	//hard reset
	ret = gpio_request(GPIO_PA(0), "sata phy1 reset");
	if (ret)
		printk("hard reset gpio_request PA0 return value = %08d\n",  ret);
	gpio_direction_output(GPIO_PA(0), 1);
	msleep(200);
	gpio_direction_output(GPIO_PA(0), 0);
	msleep(200);
	gpio_direction_output(GPIO_PA(0), 1);

	i2c->addr = 0x10;
	/*
	  bit28 power_reset_n
	  bit20 offline
	  bit12 host_sel
	  bit9  spdsel select
	  bit8  spdsel

	  bit24 force_ready
	  bit16 listen
	  bit0  idle_en
	  bit5  p0_phy_reset_n select
	  bit4  p0_phy_reset_n
	 */

	data = *(volatile unsigned int *)0xb30d01f8;
	printk("sata phy1 read reg 0x130d01f8:0x%08x\n",data);
#if (SATA_GEN == 2)
	data |= (1<<12)|(1<<8)|(0<<31);//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#else
	data |= (1<<12)|(0<<8)|(0<<31);//bit8:sata gen 1:0;sata gen 2:1,bit5:debug
#endif
	printk("sata phy1 write reg 0x130d01f8:0x%08x\n",data);
	*(volatile unsigned int *)0xb30d01f8 = data;

	msleep(300);

	i2c->addr = 0x10;
#if (SATA_GEN == 2)
	sata_phy_i2c_write(i2c, 0x24, 0x1e);
#else
	sata_phy_i2c_write(i2c, 0x34, 0xcc);
	sata_phy_i2c_write(i2c, 0x24, 0x1e);
	sata_phy_i2c_write(i2c, 0x23, 0x10);
	i2c->addr = 0x11;
	sata_phy_i2c_write(i2c, 0x77, 0x41);
	sata_phy_i2c_write(i2c, 0x13, 0xe3);
	sata_phy_i2c_write(i2c, 0x12, 0x11);
	i2c->addr = 0x30;
	sata_phy_i2c_write(i2c, 0x17, 0xf7);
#endif

	i2c->addr = 0x30;
	sata_phy_i2c_write(i2c, 0x00, 0x4e);
	sata_phy_i2c_write(i2c, 0x1a, 0x7e);
	sata_phy_i2c_write(i2c, 0x54, 0x63);
	sata_phy_i2c_write(i2c, 0x57, 0xe0);

	i2c->addr = 0x10;
	sata_phy_i2c_write(i2c, 0x60, 0x01);
	sata_phy_i2c_write(i2c, 0x64, 0x01);
	sata_phy_i2c_write(i2c, 0x7f, 0x01);
	sata_phy_i2c_write(i2c, 0x87, 0x0f);
	sata_phy_i2c_write(i2c, 0x88, 0xff);
	sata_phy_i2c_write(i2c, 0x89, 0xff);
	sata_phy_i2c_write(i2c, 0x84, 0x28);
	sata_phy_i2c_write(i2c, 0x38, 0x07);
	sata_phy_i2c_write(i2c, 0x37, 0x45);
	sata_phy_i2c_write(i2c, 0x31, 0x0);
	sata_phy_i2c_write(i2c, 0x61, 0x03);
	sata_phy_i2c_write(i2c, 0x67, 0x01);

#ifdef SATA_QUES_DISK
	sata_phy_i2c_write(i2c, 0xb8, 0x60);
	sata_phy_i2c_write(i2c, 0xb9, 0x06);
#endif
	
	i2c->addr = 0x11;
	sata_phy_i2c_write(i2c, 0x47, 0x02);
	sata_phy_i2c_write(i2c, 0x73, 0x4e);
	sata_phy_i2c_write(i2c, 0x79, 0x21);
	sata_phy_i2c_write(i2c, 0x7c, 0x01);

	i2c->addr = 0x10;
	sata_phy_i2c_write(i2c, 0x0e, 0x02);
	data = *(volatile unsigned int *)0xb30d01f8;
	data |= (1<<4) |(0<<24) | (0<<16)  | 0;
	data |= (1 << 28) | (1<<20);
	printk("sata phy1 write reg 0x130d01f8:0x%08x\n",data);
	*(volatile unsigned int *)0xb30d01f8 = data;

	mutex_init(&sata->mutex);
	i2c_set_clientdata(i2c, sata);

	return 0;
}

static int sata_phy0_i2c_remove(struct i2c_client *client)
{
	struct sata_driver *sata = i2c_get_clientdata(client);
	misc_deregister(&sata->miscdev);
	kfree(sata);
	return 0;
}

static int sata_phy1_i2c_remove(struct i2c_client *client)
{
	struct sata_driver *sata = i2c_get_clientdata(client);
	misc_deregister(&sata->miscdev);
	kfree(sata);
	return 0;
}

static const struct i2c_device_id sata_phy0_i2c_id[] = {
	{ "sata_phy0", 0 },
	{}
};

static const struct i2c_device_id sata_phy1_i2c_id[] = {
	{ "sata_phy1", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, sata_phy_i2c_id);
static struct i2c_driver sata_phy0_i2c_driver = {
	.driver = {
		.name = "sata_phy0",
		.owner = THIS_MODULE,
	},
	.probe = sata_phy0_i2c_probe,
	.remove = sata_phy0_i2c_remove,
	.id_table = sata_phy0_i2c_id,
};

static struct i2c_driver sata_phy1_i2c_driver = {
	.driver = {
		.name = "sata_phy1",
		.owner = THIS_MODULE,
	},
	.probe = sata_phy1_i2c_probe,
	.remove = sata_phy1_i2c_remove,
	.id_table = sata_phy1_i2c_id,
};

static int __init init_sata_phy(void)
{
	int ret = 0;
	struct i2c_board_info sata_phy0_i2c_info = {
		I2C_BOARD_INFO("sata_phy0", SATA_PHY_ADDR),
	};
	struct i2c_board_info sata_phy1_i2c_info = {
		I2C_BOARD_INFO("sata_phy1", SATA_PHY_ADDR),
	};
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = i2c_get_adapter(1);
	if (!adapter) {
		printk("error:(%s,%d), i2c get adapter failed.\n",__func__,__LINE__);
		return -1;
	}
	client = i2c_new_device(adapter, &sata_phy0_i2c_info);
	if (!client) {
		printk("error:(%s,%d),new i2c device failed.\n",__func__,__LINE__);
		return -1;
	}
	g_sata_phy0_i2c_client = client;
	i2c_put_adapter(adapter);

	ret = i2c_add_driver(&sata_phy0_i2c_driver);
	if ( ret != 0 ) {
		printk(KERN_ERR "Failed to register sata phy I2C driver: %d\n", ret);
		return -1;
	}

	adapter = i2c_get_adapter(0);
	if (!adapter) {
		printk("error:(%s,%d), i2c get adapter failed.\n",__func__,__LINE__);
		return -1;
	}
	client = i2c_new_device(adapter, &sata_phy1_i2c_info);
	if (!client) {
		printk("error:(%s,%d),new i2c device failed.\n",__func__,__LINE__);
		return -1;
	}
	g_sata_phy1_i2c_client = client;
	i2c_put_adapter(adapter);

	ret = i2c_add_driver(&sata_phy1_i2c_driver);
	if ( ret != 0 ) {
		printk(KERN_ERR "Failed to register sata phy I2C driver: %d\n", ret);
		return -1;
	}

	return ret;
}

static void __exit exit_sata_phy(void)
{
	i2c_unregister_device(g_sata_phy0_i2c_client);
	g_sata_phy0_i2c_client = NULL;
	i2c_del_driver(&sata_phy0_i2c_driver);

	i2c_unregister_device(g_sata_phy1_i2c_client);
	g_sata_phy1_i2c_client = NULL;
	i2c_del_driver(&sata_phy1_i2c_driver);

}

module_init(init_sata_phy);
module_exit(exit_sata_phy);
MODULE_AUTHOR("ingenic");
MODULE_DESCRIPTION("SATA PHY driver");
MODULE_LICENSE("GPL v2");
