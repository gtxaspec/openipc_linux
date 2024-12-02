#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>

#include <mach/jzssi.h>
#include "board_base.h"

#define SIZE_256B       256
#define SIZE_4KB       4096
#define SIZE_8KB       8192
#define SIZE_16KB     16384
#define SIZE_32KB     32768
#define SIZE_64KB     65536
#define SIZE_8MB    8388608
#define SIZE_16MB  16777216
#define SIZE_32MB  33554432
#define SIZE_64MB  67108864

#define TIME_400MS      400
#define TIME_80S      80000

#if defined(CONFIG_SPI_GPIO)
static struct spi_gpio_platform_data jz4780_spi_gpio_data = {
	.sck = GPIO_SPI_SCK,
	.mosi = GPIO_SPI_MOSI,
	.miso = GPIO_SPI_MISO,
	.num_chipselect	= 2,
};

static struct platform_device jz4780_spi_gpio_device = {
	.name = "spi_gpio",
	.dev = {
		.platform_data = &jz4780_spi_gpio_data,
	},
};
#ifdef CONFIG_JZ_SPI0
static struct spi_board_info jz_spi0_board_info[] = {
	[0] = {
		.modalias = "spidev",
		.bus_num = 0,
		.chip_select = 0,
		.max_speed_hz = 120000,
	},
};
#endif
#ifdef CONFIG_JZ_SPI1
static struct spi_board_info jz_spi1_board_info[] = {
	[0] = {
		.modalias = "spidev",
		.bus_num = 1,
		.chip_select = 0,
		.max_speed_hz = 120000,
	},
};
#endif
#endif

struct spi_nor_block_info flash_block_info[] = {
	{
		.blocksize = SIZE_64KB,
		.cmd_blockerase = 0xD8,
		.be_maxbusy = 1200,  /* 1.2s */
	},

	{
		.blocksize = SIZE_32KB,
		.cmd_blockerase = 0x52,
		.be_maxbusy = 1000,  /* 1s */
	},
};

#ifdef CONFIG_SPI_QUAD
struct spi_quad_mode  flash_quad_mode[] = {
	{
		.RDSR_CMD = CMD_RDSR_1,
		.WRSR_CMD = CMD_WRSR_1,
		.RDSR_DATE = 0x2,//the data is write the spi status register for QE bit
		.RD_DATE_SIZE = 1,
		.WRSR_DATE = 0x2,//this bit should be the flash QUAD mode enable
		.WD_DATE_SIZE = 1,
		.cmd_read = CMD_QUAD_READ,//
		.sfc_mode = TRAN_SPI_QUAD,
		.dummy_byte = 8,
	},
	{
		.RDSR_CMD = CMD_RDSR,
		.WRSR_CMD = CMD_WRSR,
		.RDSR_DATE = 0x40,//the data is write the spi status register for QE bit
		.RD_DATE_SIZE = 1,
		.WRSR_DATE = 0x40,//this bit should be the flash QUAD mode enable
		.WD_DATE_SIZE = 1,
		.cmd_read = CMD_QUAD_IO_FAST_READ,
		.sfc_mode = TRAN_SPI_IO_QUAD,
	},
	{
		.RDSR_CMD = CMD_RDSR_1,
		.WRSR_CMD = CMD_WRSR,
		.RDSR_DATE = 0x20,//the data is write the spi status register for QE bit
		.RD_DATE_SIZE = 1,
		.WRSR_DATE = 0x200,//this bit should be the flash QUAD mode enable
		.WD_DATE_SIZE = 2,
		.cmd_read = CMD_QUAD_READ,
		.sfc_mode = TRAN_SPI_QUAD,
	},
	{
		.RDSR_CMD = CMD_RDSR,
		.WRSR_CMD = CMD_WRSR,
		.RDSR_DATE = 0x40,//the data is write the spi status register for QE bit
		.RD_DATE_SIZE = 1,
		.WRSR_DATE = 0x40,//this bit should be the flash QUAD mode enable
		.WD_DATE_SIZE = 1,
		.cmd_read = CMD_QUAD_READ,
		.sfc_mode = TRAN_SPI_QUAD,
	},
	{
		.RDSR_CMD = CMD_RDSR_1,
		.WRSR_CMD = CMD_WRSR_1,
		.RDSR_DATE = 0x4,//the data is write the spi status register for QE bit
		.RD_DATE_SIZE = 1,
		.WRSR_DATE = 0x4,//this bit should be the flash QUAD mode enable
		.WD_DATE_SIZE = 1,
		.cmd_read = CMD_QUAD_READ,
		.sfc_mode = TRAN_SPI_QUAD,
	},
};
#endif

struct spi_nor_platform_data spi_nor_pdata[] = {
	{
		.name = "XT25F64B",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x0b4017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[2],
#endif
	},
	{
		.name = "XT25F128B",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x0b4018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[2],
#endif
	},
	{
		.name = "GM25Q64A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x1c4017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "GM25Q128A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x1c4018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "EN25QH64",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x1c7017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "EN25QH128A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x1c7018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "EN25QH256A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_32MB,
		.erasesize = SIZE_32KB,
		.id = 0x1c7019,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 4,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "EN25QH128A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x1c7118,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "XM25QH64C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x204017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "XM25QH128C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x204018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "XM25QH256C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_32MB,
		.erasesize = SIZE_32KB,
		.id = 0x204019,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 4,
		.pp_maxbusy = 4,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "XM25QH64B",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x206017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "XM25QH128B",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x206018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "XM25QH64A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x207017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "XM25QH128A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x207018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "SK25B128",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_64KB,
		.id = 0x256018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "NM25Q64EVB",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x522217,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[4],
#endif
	},
	{
		.name = "NM25Q128EVB",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_64KB,
		.id = 0x522118,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[4],
#endif
	},
	{
		.name = "DQ25Q128AL",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x546018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "ZB25VQ64",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x5e4017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "ZB25VQ128",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x5e4018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "BY25Q64AS",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x684017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "BY25Q128AS",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x684018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "PY25Q128HA",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x852018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "P25Q64H",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x856017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "P25Q128H",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0x856018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "IS25LP064D",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0x9d6017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[3],
#endif
	},
	{
		.name = "IS25LP128F",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_64KB,
		.id = 0x9d6018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[3],
#endif
	},
	{
		.name = "FM25W128",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xa12818,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "FS25Q064F2TFI1",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xa14017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "FM25Q128A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xa14018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "ZD25Q64B",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xba3217,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "MX25L6406F",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xc22017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "MX25L12835F",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xc22018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "MX25L25645G/35F",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_32MB,
		.erasesize = SIZE_32KB,
		.id = 0xc22019,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 4,
		.pp_maxbusy = 4,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "MX25L51245G",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_64MB,
		.erasesize = SIZE_32KB,
		.id = 0xc2201a,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 4,
		.pp_maxbusy = 4,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[1],
#endif
	},
	{
		.name = "GD25Q64C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xc84017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "GD25Q127C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xc84018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "GD25Q256D/E",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_32MB,
		.erasesize = SIZE_32KB,
		.id = 0xc84019,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 4,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "GD25LQ128C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xc86018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[2],
#endif
	},
	{
		.name = "XD25Q128C",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xd84018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 4ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "W25Q64",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xef4017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "WIN25Q128",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xef4018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "W25Q256JV",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_32MB,
		.erasesize = SIZE_32KB,
		.id = 0xef4019,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 4,
		.pp_maxbusy = 4,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "W25Q64-QPI",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xef6017,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "W25Q128-QPI",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_16MB,
		.erasesize = SIZE_32KB,
		.id = 0xef6018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "W25Q128-QPI",
		.pagesize = 256,
		.sectorsize = 4 * 1024,
		.chipsize = 16384 * 1024,
		.erasesize = 32 * 1024,
		.id = 0xef6018,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = 400,          /* 400ms */
		.ce_maxbusy = 8 * 10000,    /* 80s */

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
	{
		.name = "FM25Q64A",
		.pagesize = SIZE_256B,
		.sectorsize = SIZE_4KB,
		.chipsize = SIZE_8MB,
		.erasesize = SIZE_32KB,
		.id = 0xf83217,

		.block_info = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize = 3,
		.pp_maxbusy = 3,            /* 3ms */
		.se_maxbusy = TIME_400MS,
		.ce_maxbusy = TIME_80S,

		.st_regnum = 3,
#ifdef CONFIG_SPI_QUAD
		.quad_mode = &flash_quad_mode[0],
#endif
	},
};

struct jz_sfc_info sfc_info_cfg = {
	.chnl = 0,
	.bus_num = 0,
	.num_chipselect = 1,
	.board_info = spi_nor_pdata,
	.board_info_size = ARRAY_SIZE(spi_nor_pdata),
};
