#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include "../spinand.h"
#include "../ingenic_sfc_common.h"
#include "nand_common.h"

#define FS_DEVICES_NUM         5
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		180
#define TPP		400
#define TBE		3

static struct ingenic_sfcnand_device *fs_nand;

static struct ingenic_sfcnand_base_param fs_param[FS_DEVICES_NUM] = {

	[0] = {
		/* FS35ND01G */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[1] = {
		/* FS35ND02G */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[2] = {
		/* F35SQA512M */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 512,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x1,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[3] = {
		/* F35SQA001G */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x1,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[4] = {
		/* F35SQA002G */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x1,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},
};

static struct device_id_struct device_id[FS_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xEA, "FS35ND01G",  &fs_param[0]),
	DEVICE_ID_STRUCT(0xEB, "FS35ND02G",  &fs_param[1]),
	DEVICE_ID_STRUCT(0x70, "F35SQA512M", &fs_param[2]),
	DEVICE_ID_STRUCT(0x71, "F35SQA001G", &fs_param[3]),
	DEVICE_ID_STRUCT(0x72, "F35SQA002G", &fs_param[4]),
};


static cdt_params_t *fs_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(fs_nand->cdt_params);

	switch(device_id) {
		case 0xEA ... 0xEB:
		case 0x70 ... 0x72:
			break;
		default:
			dev_err(flash->dev, "device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &fs_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;

	switch(device_id) {
		case 0xEA ... 0xEB:
		case 0x70 ... 0x72:
			switch((ecc_status >> 4) & 0x3) {
				case 0x2:
					dev_err(flash->dev, "SFC ECC:Bit errors greater than %d bits detected and not corrected.\n", fs_param[0].ecc_max);
					ret = -EBADMSG;
					break;
				default:
					ret = 0;
			}
			break;
		default:
			dev_warn(flash->dev, "device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!
	}
	return ret;
}


static int fs_nand_init(void) {

	fs_nand = kzalloc(sizeof(*fs_nand), GFP_KERNEL);
	if(!fs_nand) {
		pr_err("alloc fs_nand struct fail\n");
		return -ENOMEM;
	}

	fs_nand->id_manufactory = 0xCD;
	fs_nand->id_device_list = device_id;
	fs_nand->id_device_count = FS_DEVICES_NUM;

	fs_nand->ops.get_cdt_params = fs_get_cdt_params;
	fs_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	fs_nand->ops.get_feature = NULL;

	return ingenic_sfcnand_register(fs_nand);
}

fs_initcall(fs_nand_init);
