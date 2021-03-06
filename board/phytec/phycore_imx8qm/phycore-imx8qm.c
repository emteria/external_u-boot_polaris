/*
 * Copyright (C) 2017 PHYTEC Messtechnik GmbH
 * Author: Christian Hemp <c.hemp@phytec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <i2c.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <dm.h>
#include <imx8_hsio.h>
#include <usb.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/video.h>
#include <asm/arch/video_common.h>
#include <power-domain.h>
#include <asm/arch/lpcg.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PD << PADRING_PULL_SHIFT))

static iomux_cfg_t uart0_pads[] = {
	SC_P_UART0_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART0_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart0_pads, ARRAY_SIZE(uart0_pads));
}

int checkboard(void)
{
	puts("Board: phyCORE-i.MX8\n");
	print_bootinfo();

	return 0;
}

int board_early_init_f(void)
{
	int ret;

	/* Power up UART0, this is very early while power domain is not working */
	ret = sc_pm_set_resource_power_mode(-1, SC_R_UART_0, SC_PM_PW_MODE_ON);
	if (ret)
		return ret;

	/* Set UART0 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;
	ret = sc_pm_set_clock_rate(-1, SC_R_UART_0, 2, &rate);
	if (ret)
		return ret;

	/* Enable UART0 clock root */
	ret = sc_pm_clock_enable(-1, SC_R_UART_0, 2, true, false);
	if (ret)
		return ret;

	lpcg_all_clock_on(LPUART_0_LPCG);

	setup_iomux_uart();

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	/* Empty like NXP */
}

int board_mmc_get_env_dev(int devno)
{
	/* Use EMMC */
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

	return devno;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	/* Use EMMC */
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

	return dev_no;
}

static int check_mmc_autodetect(void)
{
	char *autodetect_str = env_get("mmcautodetect");

	if ((autodetect_str) &&
	    (strcmp(autodetect_str, "yes") == 0)) {
		return 1;
	}

	return 0;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_dev();

	if (!check_mmc_autodetect())
		return;

	env_set_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw",
		mmc_map_to_kernel_blk(dev_no));
	env_set("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}

int board_late_init(void)
{
	board_late_mmc_env_init();

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

int board_init(void)
{
	return 0;
}
