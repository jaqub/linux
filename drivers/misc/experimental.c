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
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/signal.h>
#include <linux/of.h>
#include <linux/sysfs.h>

static struct task_struct *t;
static DECLARE_COMPLETION(update_fw);

static struct ex_t {
  struct mutex dev_lock;
};


#ifdef CONFIG_OF
static const struct of_device_id experimental_dt_ids[] = {
	{
		.compatible = "biwel,ex1",
	},
  {}
};
MODULE_DEVICE_TABLE(of, experimental_dt_ids);
#endif

static 	ssize_t set_trigger(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count) {

  struct ex_t *exdev = (struct ex_t *)dev_get_drvdata(dev);

  dev_info(dev, "Trigger: started\n");

  complete(&update_fw);
  msleep(1000);

  dev_info(dev, "Trigger: before try lock\n");

  mutex_lock(&exdev->dev_lock);

  dev_info(dev, "Trigger: Mutex locked\n");

  mutex_unlock(&exdev->dev_lock);

  dev_info(dev, "Trigger: Mutex unlocked\n");

  return count;
}

static DEVICE_ATTR(trigger, S_IWUSR, NULL, set_trigger);

static const struct attribute *ex_attr[] = {
  &dev_attr_trigger.attr,
  NULL
};

static int updatefn(void *data) {
  int ret = 0;
  struct platform_device *pdev = (struct platform_device *)data;
  struct ex_t *exdev = (struct ex_t *)platform_get_drvdata(pdev);

  dev_info(&pdev->dev, "Update: started\n");

  allow_signal(SIGKILL);

  ret = wait_for_completion_killable(&update_fw);
  if (ret == -ERESTARTSYS) {
    dev_info(&pdev->dev, "Update: canceled\n");
    return 0;
  }

  mutex_lock(&exdev->dev_lock);
  
  dev_info(&pdev->dev, "Update: Mutex locked - going to sleep\n");
  msleep(5000);
  
  mutex_unlock(&exdev->dev_lock);
  
  dev_info(&pdev->dev, "Update: Mutex unlocked\n");

  dev_info(&pdev->dev, "Update: complete\n");

  return 0;
}

static int ex_probe(struct platform_device *pdev)
{
  int ret = 0;
  struct ex_t *exdev = NULL;

	dev_info(&pdev->dev, "Probe for experimental driver called\n");

  exdev = devm_kzalloc(&pdev->dev, sizeof(struct ex_t), GFP_KERNEL);
  if (!exdev) {
    dev_err(&pdev->dev, "Failed to allocate memory\n");
    ret = -ENOMEM;
    goto err_memalloc;
  }

  mutex_init(&exdev->dev_lock);

  ret = sysfs_create_files(&pdev->dev.kobj, ex_attr);
  if (ret)
    goto err_sysfs;

  platform_set_drvdata(pdev, exdev);

  t = kthread_run(&updatefn, pdev, "fw update");
  if (IS_ERR(t)) {
    dev_err(&pdev->dev, "Failed to create task\n");
    ret = PTR_ERR(t);
    goto err_thread_create;
  }

	return 0;

err_thread_create:
  sysfs_remove_files(&pdev->dev.kobj, ex_attr);
err_sysfs:
err_memalloc:
  return ret;
}

static int ex_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "Remove for experimental driver called\n");

  sysfs_remove_files(&pdev->dev.kobj, ex_attr);

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
