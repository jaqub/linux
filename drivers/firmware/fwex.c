#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

struct firmware {
};

static DEFINE_MUTEX(transaction_lock);

static int firmware_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct firmware *fw;

	fw = devm_kzalloc(dev, sizeof(*fw), GFP_KERNEL);
	if (!fw)
		return -ENOMEM;

	platform_set_drvdata(pdev, fw);

	return 0;
}

static int firmware_remove(struct platform_device *pdev)
{
	struct rpi_firmware *fw = platform_get_drvdata(pdev);

	return 0;
}

static const struct of_device_id rpi_firmware_of_match[] = {
	{ .compatible = "firmware-experiments", },
	{},
};
MODULE_DEVICE_TABLE(of, rpi_firmware_of_match);

static struct platform_driver rpi_firmware_driver = {
	.driver = {
		.name = "firmware-experiments",
		.of_match_table = rpi_firmware_of_match,
	},
	.probe		= firmware_probe,
	.remove		= firmware_remove,
};
module_platform_driver(rpi_firmware_driver);

MODULE_AUTHOR("Jakub Biwel <jakub.biwel@gmail.com>");
MODULE_DESCRIPTION("Firmware driver");
MODULE_LICENSE("GPL v2");
