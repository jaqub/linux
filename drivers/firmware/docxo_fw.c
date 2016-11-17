#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>

struct fwex {
};

static DEFINE_MUTEX(transaction_lock);

static int firmware_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fwex *fw;
	const struct firmware *docxo_fw;
	int ret;

	fw = devm_kzalloc(dev, sizeof(*fw), GFP_KERNEL);
	if (!fw)
		return -ENOMEM;

	platform_set_drvdata(pdev, fw);

	ret = request_firmware(&docxo_fw, "docxo", &pdev->dev);
	if (ret) {
  	dev_err(&pdev->dev, "Failed to find firmware\n");  
		return -ENODEV;
	}

	dev_info(&pdev->dev, "Firmware experiments loaded\n");

	return 0;
}

static int firmware_remove(struct platform_device *pdev)
{
	struct fwex *fw = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Firmware experiments module removed\n");

	return 0;
}

static const struct of_device_id rpi_firmware_of_match[] = {
	{ .compatible = "docxo-a16-0000", },
	{},
};
MODULE_DEVICE_TABLE(of, rpi_firmware_of_match);

static struct platform_driver rpi_firmware_driver = {
	.driver = {
		.name = "docxo",
		.of_match_table = rpi_firmware_of_match,
	},
	.probe		= firmware_probe,
	.remove		= firmware_remove,
};
module_platform_driver(rpi_firmware_driver);

MODULE_AUTHOR("Jakub Biwel <jakub.biwel@gmail.com>");
MODULE_DESCRIPTION("docxo");
MODULE_LICENSE("GPL v2");
