/*
 * Atmel SSC driver
 *
 * Copyright (C) 2007 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>

#include <linux/of.h>


#ifdef CONFIG_OF
static const struct of_device_id experimental_dt_ids[] = {
	{
		.compatible = "biwel,ex1",
	},
  {}
};
MODULE_DEVICE_TABLE(of, experimental_dt_ids);
#endif

static int ex_probe(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "Probe for experimental driver called\n");

	return 0;
}

static int ex_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "Remove for experimental driver called\n");

	return 0;
}

static struct platform_driver experimental_driver = {
	.driver		= {
		.name		= "experimental",
		.of_match_table	= of_match_ptr(experimental_dt_ids),
	},
	.probe		= ex_probe,
	.remove		= ex_remove,
};
module_platform_driver(experimental_driver);

MODULE_AUTHOR("Jakub Biwel <jakub.biwel@gmail.com>");
MODULE_DESCRIPTION("Experimental driver for development purpose");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:experimental");
