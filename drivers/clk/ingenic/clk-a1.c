#include <linux/slab.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/syscore_ops.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <soc/cpm.h>
#include <soc/base.h>
#include <dt-bindings/clock/ingenic-a1.h>

#include <jz_proc.h>
#include "clk.h"

#define CLK_FLG_ENABLE BIT(1)
static struct ingenic_clk_provider *ctx;

/*******************************************************************************
 *      FIXED CLK
 ********************************************************************************/
static struct ingenic_fixed_rate_clock a1_fixed_rate_ext_clks[] __initdata = {
	FRATE(CLK_EXT, "ext", NULL, CLK_IS_ROOT, 24000000),
	FRATE(CLK_RTC_EXT, "rtc_ext", NULL, CLK_IS_ROOT, 32768),
};

/*******************************************************************************
 *      PLL
 ********************************************************************************/

static struct ingenic_pll_rate_table a1_pll_rate_table[] = {
	PLL_RATE(1500000000, 125, 1, 2, 1),
	PLL_RATE(1404000000, 117, 1, 2, 1),
	PLL_RATE(1392000000, 116, 1, 2, 1),
	PLL_RATE(1296000000, 108, 1, 2, 1),
	PLL_RATE(1200000000, 100, 1, 2, 1),
	PLL_RATE(1000000000, 125, 1, 3, 1),
	PLL_RATE(900000000, 75, 1, 2, 1),
	PLL_RATE(891000000, 297, 4, 2, 1),
	PLL_RATE(864000000, 72, 1, 2, 1),
	PLL_RATE(600000000, 75, 1, 3, 1),
};

/*PLL HWDESC*/
static struct ingenic_pll_hwdesc apll_hwdesc = \
	PLL_DESC(CPM_CPAPCR, 20, 12, 14, 6, 11, 3, 8, 3, 3, 0, 2);

static struct ingenic_pll_hwdesc mpll_hwdesc = \
	PLL_DESC(CPM_CPMPCR, 20, 12, 14, 6, 11, 3, 8, 3, 3, 0, 2);

static struct ingenic_pll_hwdesc vpll_hwdesc = \
	PLL_DESC(CPM_CPVPCR, 20, 12, 14, 6, 11, 3, 8, 3, 3, 0, 2);

static struct ingenic_pll_hwdesc epll_hwdesc = \
	PLL_DESC(CPM_CPEPCR, 20, 12, 14, 6, 11, 3, 8, 3, 3, 0, 2);



static struct ingenic_pll_clock a1_pll_clks[] __initdata = {
	PLL(CLK_PLL_APLL,	"apll",	"ext",	&apll_hwdesc,	a1_pll_rate_table),
	PLL(CLK_PLL_MPLL,	"mpll",	"ext",	&mpll_hwdesc,	a1_pll_rate_table),
	PLL(CLK_PLL_VPLL,	"vpll", "ext",	&vpll_hwdesc,	a1_pll_rate_table),
	PLL(CLK_PLL_EPLL,	"epll", "ext",	&epll_hwdesc,	a1_pll_rate_table),
};


/*******************************************************************************
 *      MUX
 ********************************************************************************/
PNAME(mux_table_0) = {"stop",	"ext",		"apll"};
PNAME(mux_table_1) = {"stop",	"sclka",	"mpll"};
PNAME(mux_table_2) = {"sclka",	"mpll",		"vpll",		"epll"};
PNAME(mux_table_3) = {"sclka", 	"mpll",		"epll",		"ext"};
// PNAME(mux_table_4) = {"sclka", 	"vpll",		"ext"};
// PNAME(mux_table_5) = {"mux_ahb2",	"ext"};

static unsigned int ingenic_mux_table[] = {0, 1, 2, 3};

static struct ingenic_mux_clock a1_mux_clks[] __initdata = {
	MUX(CLK_MUX_SCLKA,	"sclka",	ingenic_mux_table,	mux_table_0,	CPM_CPCCR,	30, 2, 0),
	MUX(CLK_MUX_CPU_L2C,	"mux_cpu_l2c",	ingenic_mux_table,	mux_table_1, 	CPM_CPCCR,	28, 2, 0),
	MUX(CLK_MUX_AHB0,	"mux_ahb0",	ingenic_mux_table,	mux_table_1, 	CPM_CPCCR,	26, 2, 0),
	MUX(CLK_MUX_AHB2,	"mux_ahb2",	ingenic_mux_table,	mux_table_1, 	CPM_CPCCR,	24, 2, 0),
	MUX(CLK_MUX_AHB1,	"mux_ahb1",	ingenic_mux_table,	mux_table_2, 	CPM_AHB1CDR,    0,  2, 0),

	MUX(CLK_MUX_DDR,	"mux_ddr",	ingenic_mux_table, 	mux_table_1, 	CPM_DDRCDR,	30, 2, 0),
	/* MUX(CLK_MUX_RSA,	"mux_rsa",	ingenic_mux_table, 	mux_table_2, 	CPM_RSACDR,	30, 2, 0), */
	MUX(CLK_MUX_SSI,	"mux_ssi",	ingenic_mux_table, 	mux_table_2, 	CPM_SSICDR,	30, 2, 0),
	MUX(CLK_MUX_SFC0,	"mux_sfc0",	ingenic_mux_table, 	mux_table_2, 	CPM_SFC0CDR,	30, 2, 0),
	MUX(CLK_MUX_SFC1,	"mux_sfc1",	ingenic_mux_table, 	mux_table_2, 	CPM_SFC1CDR,	30, 2, 0),
	MUX(CLK_MUX_MSC0,	"mux_msc0",	ingenic_mux_table, 	mux_table_3, 	CPM_MSC0CDR,	30, 2, 0),
	MUX(CLK_MUX_MSC1,	"mux_msc1",	ingenic_mux_table, 	mux_table_3, 	CPM_MSC1CDR,	30, 2, 0),
	MUX(CLK_MUX_I2ST0,	"mux_i2st0",	ingenic_mux_table, 	mux_table_2, 	CPM_I2S0TCDR,   30, 2, 0),
	MUX(CLK_MUX_I2SR0,	"mux_i2sr0",	ingenic_mux_table, 	mux_table_2, 	CPM_I2S0RCDR,   30, 2, 0),
	MUX(CLK_MUX_I2ST1,	"mux_i2st1",	ingenic_mux_table, 	mux_table_2, 	CPM_I2S1TCDR,   30, 2, 0),
	MUX(CLK_MUX_I2SR1,	"mux_i2sr1",	ingenic_mux_table, 	mux_table_2, 	CPM_I2S1RCDR,   30, 2, 0),
	MUX(CLK_MUX_VDEV,	"mux_vdev",	ingenic_mux_table, 	mux_table_2, 	CPM_VDEVCDR,	30, 2, 0),
	MUX(CLK_MUX_VDEA,	"mux_vdea",	ingenic_mux_table, 	mux_table_2, 	CPM_VDEACDR,	30, 2, 0),
	MUX(CLK_MUX_VDEM,	"mux_vdem",	ingenic_mux_table, 	mux_table_2, 	CPM_VDEMCDR,	30, 2, 0),
	MUX(CLK_MUX_IPU,	"mux_ipu",	ingenic_mux_table, 	mux_table_2, 	CPM_IPUMCDR,	30, 2, 0),
	MUX(CLK_MUX_MAC0PHY,	"mux_mac0phy",	ingenic_mux_table, 	mux_table_2, 	CPM_MAC0CDR,	30, 2, 0),
	MUX(CLK_MUX_MAC0TXPHY,	"mux_mac0txphy",ingenic_mux_table, 	mux_table_2, 	CPM_MAC0TXCDR,	30, 2, 0),
	MUX(CLK_MUX_MAC1PHY,	"mux_mac1phy",	ingenic_mux_table, 	mux_table_2, 	CPM_MAC1CDR,	30, 2, 0),
	MUX(CLK_MUX_MAC1TXPHY,	"mux_mac1txphy",ingenic_mux_table, 	mux_table_2, 	CPM_MAC1TXCDR,	30, 2, 0),
	MUX(CLK_MUX_MACPTP,	"mux_macptp",	ingenic_mux_table, 	mux_table_2, 	CPM_MACPTPCDR,	30, 2, 0),
	MUX(CLK_MUX_BT0,	"mux_bt0",	ingenic_mux_table, 	mux_table_2, 	CPM_BT0CDR,	30, 2, 0),
	MUX(CLK_MUX_PWM,	"mux_pwm",	ingenic_mux_table, 	mux_table_2, 	CPM_PWMCDR,	30, 2, 0),

};

/*******************************************************************************
 *      DIV BUS
 ********************************************************************************/

static struct ingenic_bus_clock a1_bus_div_clks[] __initdata = {
	BUS_DIV(CLK_DIV_CPU,		"div_cpu",		"mux_cpu_l2c",	CPM_CPCCR,	0,  4, 0, 0, CPM_CPCSR, 0, 22, BUS_DIV_SELF),
	BUS_DIV(CLK_DIV_L2C,		"div_l2c",		"mux_cpu_l2c",	CPM_CPCCR,	4,  4, 0, 0, CPM_CPCSR, 0, 22, BUS_DIV_SELF),
	BUS_DIV(CLK_DIV_AHB0,		"div_ahb0",		"mux_ahb0",	CPM_CPCCR,	8,  4, 0, 0, CPM_CPCSR, 1, 21, BUS_DIV_SELF),
	BUS_DIV(CLK_DIV_AHB1,		"div_ahb1",		"mux_ahb1",	CPM_AHB1CDR,	4,  4, 0, 0, CPM_CPCSR, 2, 8, BUS_DIV_SELF),
	BUS_DIV(CLK_DIV_AHB2,		"div_ahb2",		"mux_ahb2",	CPM_CPCCR,	12, 4, 0, 0, CPM_CPCSR, 3, 20, BUS_DIV_SELF),
	BUS_DIV(CLK_DIV_APB,		"div_apb",		"mux_ahb2",	CPM_CPCCR,	16, 4, 0, 0, CPM_CPCSR, 3, 20, BUS_DIV_SELF),
	BUS_DIV(CLK_DIV_CPU_L2C_X1,	"div_cpu_l2c_x1",	"mux_cpu_l2c",	CPM_CPCCR,	0,  4, 4, 4, CPM_CPCSR, 0, 22, BUS_DIV_ONE),
	BUS_DIV(CLK_DIV_CPU_L2C_X2,	"div_cpu_l2c_x2",	"mux_cpu_l2c",	CPM_CPCCR,	0,  4, 4, 4, CPM_CPCSR, 0, 22, BUS_DIV_TWO),
};


/*******************************************************************************
 *      DIV
 ********************************************************************************/

static struct clk_div_table a1_clk_div_table[256] = {};

static struct ingenic_div_clock a1_div_clks[] __initdata = {
	DIV(CLK_DIV_DDR,		"div_ddr",		"mux_ddr",		CPM_DDRCDR,	4,  0, NULL),
	/* DIV(CLK_DIV_RSA,		"div_rsa",		"mux_rsa",		CPM_RSACDR,	4,  0, NULL), */
	DIV(CLK_DIV_SSI,		"div_ssi",		"mux_ssi",		CPM_SSICDR,	8,  0, NULL),
	DIV(CLK_DIV_SFC0,		"div_sfc0",		"mux_sfc0",		CPM_SFC0CDR,	8,  0, NULL),
	DIV(CLK_DIV_SFC1,		"div_sfc1",		"mux_sfc1",		CPM_SFC1CDR,	8,  0, NULL),
	DIV(CLK_DIV_MSC0,		"div_msc0",		"mux_msc0",		CPM_MSC0CDR,	8,  0, a1_clk_div_table),
	DIV(CLK_DIV_MSC1,		"div_msc1",		"mux_msc1",		CPM_MSC1CDR,	8,  0, a1_clk_div_table),
	DIV(CLK_DIV_VDEV,		"div_vdev",		"mux_vdev",		CPM_VDEVCDR,	8,  0, NULL),
	DIV(CLK_DIV_VDEA,		"div_vdea",		"mux_vdea",		CPM_VDEACDR,	8,  0, NULL),
	DIV(CLK_DIV_VDEM,		"div_vdem",		"mux_vdem",		CPM_VDEMCDR,	8,  0, NULL),
	DIV(CLK_DIV_IPU,		"div_ipu",		"mux_ipu",		CPM_IPUMCDR,	8,  0, NULL),
	DIV(CLK_DIV_MAC0PHY,		"div_mac0phy",		"mux_mac0phy",		CPM_MAC0CDR,	8,  0, NULL),
	DIV(CLK_DIV_MAC0TXPHY,		"div_mac0txphy",	"mux_mac0txphy",	CPM_MAC0TXCDR,	8,  0, NULL),
	DIV(CLK_DIV_MAC1PHY,		"div_mac1phy",		"mux_mac1phy",		CPM_MAC1CDR,	8,  0, NULL),
	DIV(CLK_DIV_MAC1TXPHY,		"div_mac1txphy",	"mux_mac1txphy",	CPM_MAC1TXCDR,	8,  0, NULL),
	DIV(CLK_DIV_MACPTP,		"div_macptp",		"mux_macptp",		CPM_MACPTPCDR,	8,  0, NULL),
	DIV(CLK_DIV_BT0,		"div_bt0",		"mux_bt0",		CPM_BT0CDR,	8,  0, NULL),
	DIV(CLK_DIV_PWM,		"div_pwm",		"mux_pwm",		CPM_PWMCDR,	8,  0, NULL),

};

/*******************************************************************************
 *      FRACTIONAL-DIVIDER
 ********************************************************************************/

static struct ingenic_fra_div_clock a1_fdiv_clks[] __initdata = {
	FRA_DIV(CLK_DIV_I2ST0,	"div_i2st0",	"mux_i2st0",	CPM_I2S0TCDR,	20, 9, 0, 20),
	FRA_DIV(CLK_DIV_I2SR0,	"div_i2sr0",	"mux_i2sr0",	CPM_I2S0RCDR,	20, 9, 0, 20),
	FRA_DIV(CLK_DIV_I2ST1,	"div_i2st1",	"mux_i2st1",	CPM_I2S1TCDR,	20, 9, 0, 20),
	FRA_DIV(CLK_DIV_I2SR1,	"div_i2sr1",	"mux_i2sr1",	CPM_I2S1RCDR,	20, 9, 0, 20),
};

/*******************************************************************************
 *      GATE
 ********************************************************************************/
static struct ingenic_gate_clock a1_gate_clks[] __initdata = {
	GATE(CLK_GATE_AIC0,	    	"gate_aic0",		"div_i2st0",	CPM_CLKGR,	30,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_IPU,      	"gate_ipu",		"div_ipu",	CPM_CLKGR,	29,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_VDE,      	"gate_vde",		"div_vdea",     CPM_CLKGR,	28,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_HDMI,     	"gate_hdmi",		"div_ahb0",     CPM_CLKGR,	27,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_VGA,      	"gate_vga",		"div_apb",	CPM_CLKGR,	26,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_SFC1,	    	"gate_sfc1",		"div_sfc1",	CPM_CLKGR,	25,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_SFC0,	    	"gate_sfc0",		"div_sfc0",	CPM_CLKGR,	24,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_AIC1,	    	"gate_aic1",		"div_i2st1",	CPM_CLKGR,	23, 	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_RTC,	    	"gate_rtc",		"div_apb",	CPM_CLKGR,	22, 	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_I2C1,	    	"gate_i2c1",		"div_apb",	CPM_CLKGR,	21,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_I2C0,	    	"gate_i2c0",		"div_apb",	CPM_CLKGR,	20,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_AHB1,	    	"gate_ahb1",		"div_ahb1",	CPM_CLKGR,	19,  	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_AIP,		"gate_aip",		"div_ahb0",	CPM_CLKGR,	18,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_SSI1,	    	"gate_ssi1",		"div_ssi",	CPM_CLKGR,	17, 	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_SSI0,	    	"gate_ssi0",		"div_ssi",	CPM_CLKGR,	16,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_MSC1,	    	"gate_msc1",		"div_msc1",	CPM_CLKGR,	15,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_MSC0,	    	"gate_msc0",		"div_msc0",	CPM_CLKGR,	14,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_OTG2,	    	"gate_otg2",		"div_ahb2",	CPM_CLKGR,	13,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_OTG1,	    	"gate_otg1",		"div_ahb2",	CPM_CLKGR,	12,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_OTG0,	    	"gate_otg0",		"div_ahb2",	CPM_CLKGR,	11,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_UART2,		"gate_uart2",		"div_apb",	CPM_CLKGR,	10, 	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_UART1,		"gate_uart1",		"div_apb",	CPM_CLKGR,	9,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_UART0,		"gate_uart0",		"div_apb",	CPM_CLKGR,	8,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_NEMC,	    	"gate_nemc",		"div_ahb2",	CPM_CLKGR,	7,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_OST,	    	"gate_ost",		"ext",		CPM_CLKGR,	6,	CLK_IGNORE_UNUSED, 	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_TCU,	    	"gate_tcu",		"div_apb",	CPM_CLKGR,	5, 	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_EFUSE,		"gate_efuse",		"div_ahb2",	CPM_CLKGR,	4,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_DDR,	    	"gate_ddr",		"div_ddr",	CPM_CLKGR,	3,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_AHB0,	    	"gate_ahb0",		"div_ahb0",	CPM_CLKGR,	2,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_APB,	    	"gate_apb",		"div_apb",	CPM_CLKGR,	1,	CLK_IGNORE_UNUSED,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_CPU,	    	"gate_cpu",		"div_cpu",	CPM_CLKGR,	0, 	0,	CLK_GATE_SET_TO_DISABLE),

	GATE(CLK_GATE_PWM,	    	"gate_pwm",		"div_pwm",	CPM_CLKGR1,	15,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_BT0,	    	"gate_bt0",		"div_bt0",	CPM_CLKGR1,	14,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_JPEG,	    	"gate_jpeg",		"div_ahb1",	CPM_CLKGR1,	13,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_BUS_MONITOR,	"gate_monitor",	"div_ahb0",	CPM_CLKGR1,	12,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_SATA,	    	"gate_sata",		"div_ahb0",	CPM_CLKGR1,	11, 	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_GMAC1,	   	"gate_gmac1",		"div_ahb0",	CPM_CLKGR1,	10,  	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_AES,	    	"gate_aes",		"div_ahb2",	CPM_CLKGR1,	9,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_GMAC0,	   	"gate_gmac0",		"div_ahb0",	CPM_CLKGR1,	8,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_DTRNG,		"gate_dtrng",		"div_apb",	CPM_CLKGR1,	7,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_VC8000D,	   	"gate_vc8000d",		"div_ahb1",	CPM_CLKGR1,	6,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_DES,	    	"gate_des",		"div_apb",	CPM_CLKGR1,	5,	0,	CLK_GATE_SET_TO_DISABLE),
	/* GATE(CLK_GATE_RSA,	    	"gate_rsa",		"div_rsa",	CPM_CLKGR1,	4,	0,	CLK_GATE_SET_TO_DISABLE), */
	GATE(CLK_GATE_PDMA,	    	"gate_pdma",		"div_ahb2",	CPM_CLKGR1,	3,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_GATE_HASH,	    	"gate_hash",		"div_ahb2",	CPM_CLKGR1,	2,	0,	CLK_GATE_SET_TO_DISABLE),
	GATE(CLK_CE_I2S0T,	    	"ce_i2st0",		"div_i2st0",	CPM_I2S0TCDR,	29,	0,	0),
	GATE(CLK_CE_I2S0R,	    	"ce_i2sr0",		"div_i2sr0",	CPM_I2S0RCDR,	29, 	0,	0),
	GATE(CLK_CE_I2S1T,	    	"ce_i2st1",		"div_i2st1",	CPM_I2S1TCDR,	29, 	0,	0),
	GATE(CLK_GATE_USBPHY,   	"gate_usbphy",      	"div_apb",      CPM_OPCR,	23, 	0,	CLK_GATE_SET_TO_DISABLE)
};

static void clk_div_table_generate(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(a1_clk_div_table); i++) {
		a1_clk_div_table[i].val = i;
		a1_clk_div_table[i].div = (i + 1) * 4;
	}

}

static const struct of_device_id ext_clk_match[] __initconst = {
	        { .compatible = "ingenic,fixed-clock", .data = (void *)0, },
			{},
};

static int clocks_show(struct seq_file *m, void *v)
{
	int i = 0;
	struct clk_onecell_data * clk_data = NULL;
	struct clk *clk = NULL;

	if(m->private != NULL) {
		seq_printf(m, "CLKGR\t: %08x\n", cpm_inl(CPM_CLKGR));
		seq_printf(m, "CLKGR1\t: %08x\n", cpm_inl(CPM_CLKGR1));
		seq_printf(m, "LCR1\t: %08x\n", cpm_inl(CPM_LCR));
	} else {
		seq_printf(m, " ID  NAME              FRE          sta     count   parent\n");
		clk_data = &ctx->clk_data;
		for(i = 0; i < clk_data->clk_num; i++) {
			clk = clk_data->clks[i];
			if (clk != ERR_PTR(-ENOENT)) {
				if (__clk_get_name(clk) == NULL) {
					seq_printf(m, "--------------------------------------------------------\n");
				} else {
					unsigned int mhz = _get_rate(__clk_get_name(clk)) / 1000;
					seq_printf(m, "%3d %-15s %4d.%03dMHz %7sable   %d %10s\n", i, __clk_get_name(clk)
								, mhz/1000, mhz%1000
								, __clk_get_flags(clk) & CLK_FLG_ENABLE? "en": "dis"
								, __clk_get_enable_count(clk)
								, clk_get_parent(clk)? __clk_get_name(clk_get_parent(clk)): "root");
				}
			}
		}
	}

	return 0;
}

static int clocks_open(struct inode *inode, struct file *file)
{
	return single_open_size(file, clocks_show, PDE_DATA(inode),8192);
}

static const struct file_operations clocks_proc_fops ={
	.read = seq_read,
	.open = clocks_open,
	.llseek = seq_lseek,
	.release = single_release,
};

/* Register a1 clocks. */
static void __init a1_clk_init(struct device_node *np)
{
	void __iomem *reg_base;

	printk("A1 Clock Power Management Unit init!\n");

	if (np) {
		reg_base = of_iomap(np, 0);
		if (!reg_base)
			panic("%s: failed to map registers\n", __func__);
	}


	ctx = ingenic_clk_init(np, reg_base, NR_CLKS);
	if (!ctx)
		panic("%s: unable to allocate context.\n", __func__);

	/* Register Ext Clocks From DT */
	ingenic_clk_of_register_fixed_ext(ctx, a1_fixed_rate_ext_clks,
			                        ARRAY_SIZE(a1_fixed_rate_ext_clks), ext_clk_match);

	/* Register PLLs. */
	ingenic_clk_register_pll(ctx, a1_pll_clks,
				ARRAY_SIZE(a1_pll_clks), reg_base);


	/* Register Muxs */
	ingenic_clk_register_mux(ctx, a1_mux_clks, ARRAY_SIZE(a1_mux_clks));

	/* Register Bus Divs */
	ingenic_clk_register_bus_div(ctx, a1_bus_div_clks, ARRAY_SIZE(a1_bus_div_clks));

	/* Register Divs */
	clk_div_table_generate();
	ingenic_clk_register_cgu_div(ctx, a1_div_clks, ARRAY_SIZE(a1_div_clks));

	/* Register Fractional Divs */
	ingenic_clk_register_fra_div(ctx, a1_fdiv_clks, ARRAY_SIZE(a1_fdiv_clks));

	/* Register Gates */
	ingenic_clk_register_gate(ctx, a1_gate_clks, ARRAY_SIZE(a1_gate_clks));

	/* Register Powers */
//	ingenic_power_register_gate(ctx, a1_gate_power, ARRAY_SIZE(a1_gate_power));

	ingenic_clk_of_add_provider(np, ctx);


	//ingenic_clk_of_dump(ctx);


	pr_info("=========== a1 clocks: =============\n"
		"\tapll     = %lu , mpll     = %lu\n"
		"\tcpu_clk  = %lu , l2c_clk  = %lu\n"
		"\tahb0_clk = %lu , ahb2_clk = %lu\n"
		"\tapb_clk  = %lu , ext_clk  = %lu\n\n",
		_get_rate("apll"),	_get_rate("mpll"),
		_get_rate("div_cpu"), _get_rate("div_l2c"),
		_get_rate("div_ahb0"), _get_rate("div_ahb2"),
		_get_rate("div_apb"), _get_rate("ext"));

}

CLK_OF_DECLARE(a1_clk, "ingenic,a1-clocks", a1_clk_init);

static int __init init_clk_proc(void)
{
	struct proc_dir_entry *p;

	p = jz_proc_mkdir("clock");
	if (!p) {
		pr_warning("create_proc_entry for common clock failed!\n");
	} else {
		proc_create_data("clocks", 0600, p, &clocks_proc_fops, 0);
		proc_create_data("misc", 0600, p, &clocks_proc_fops, (void *)1);
	}
	return 0;
}

module_init(init_clk_proc);
