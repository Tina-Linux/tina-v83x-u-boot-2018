/*
 * Copyright (C) 2019 Allwinner.
 * weidonghui <weidonghui@allwinnertech.com>
 *
 * SUNXI AXP  Driver
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <sunxi_power/bmu_axp152.h>
#include <sunxi_power/axp.h>
#include <asm/arch/pmic_bus.h>

static int bmu_axp152_probe(void)
{
	u8 pmu_chip_id;
	if (pmic_bus_init(AXP152_DEVICE_ADDR, AXP152_RUNTIME_ADDR)) {
		tick_printf("%s pmic_bus_init fail\n", __func__);
		return -1;
	}
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_VERSION, &pmu_chip_id)) {
		tick_printf("%s pmic_bus_read fail\n", __func__);
		return -1;
	}
	pmu_chip_id &= 0X0F;
	if (pmu_chip_id == AXP152_CHIP_ID) {
		/*pmu type AXP152*/
		tick_printf("PMU: AXP152\n");
		return 0;
	}
	return -1;

}


int bmu_axp152_get_poweron_source(void)
{
	uchar reg_value;
	uchar reg_value2;
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_MODE_CHGSTATUS, &reg_value2)) {
		return -1;
	}
	if (reg_value2 & (1 << 3)) {
		return AXP_BOOT_SOURCE_IRQ_LOW;
	}

	reg_value = 3;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_GPIO1_CTL, reg_value)) {
		return -1;
	}
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_GPIO0123_SIGNAL, &reg_value)) {
		return -1;
	}
	if (!(reg_value & (1 << 1))) {
		return AXP_BOOT_SOURCE_VBUS_USB;
	}

	if (reg_value2 & (1 << 2)) {
		return AXP_BOOT_SOURCE_BUTTON;
	}
	return -1;
}

int sunxi_gpadc_init(void);
int sunxi_gpadc_read(int channel);


int bmu_axp152_get_battery_vol(void)
{
#ifdef CONFIG_SUNXI_GPADC
	static int gpadc_flag;
	u32 adc_val;
	if (!gpadc_flag) {
		sunxi_gpadc_init();
		gpadc_flag = 1;
	}
	adc_val = sunxi_gpadc_read(1);
	if (adc_val <= 0x9eb)
		return 3500;
	else
		return 3760;
#else
	return -1;
#endif
}

unsigned char bmu_axp152_get_reg_value(unsigned char reg_addr)
{
	u8 reg_value;
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, reg_addr, &reg_value)) {
		return -1;
	}
	return reg_value;
}

unsigned char bmu_axp152_set_reg_value(unsigned char reg_addr, unsigned char reg_value)
{
	unsigned char reg;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, reg_addr, reg_value)) {
		return -1;
	}
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, reg_addr, &reg)) {
		return -1;
	}
	return reg;
}


U_BOOT_AXP_BMU_INIT(bmu_axp152) = {
	.bmu_name		  = "bmu_axp152",
	.probe			  = bmu_axp152_probe,
	.get_poweron_source       = bmu_axp152_get_poweron_source,
	.get_battery_vol  = bmu_axp152_get_battery_vol,
	.get_reg_value	   = bmu_axp152_get_reg_value,
	.set_reg_value	   = bmu_axp152_set_reg_value,
};

