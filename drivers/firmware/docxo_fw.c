#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>

struct docxo {
};

struct docxo_record {
	u32 id;
	u32 len;
	const u8 data[0];
};

struct docxo_header {
	u32 magic;
	struct docxo_record record;
};

static struct docxo_record * docxo_next_record(struct docxo_record * rec) {
	__u32 len = be32_to_cpu(rec->len);
	rec = (struct docxo_record *)((__u8 *)rec + len + sizeof(struct docxo_record));

	return be32_to_cpu(rec->len)? rec : NULL;
}

static int firmware_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct docxo *fw;
	const struct firmware *docxo_fw;
	struct docxo_header *hdocxo;
	struct docxo_record *drecord;
	int i = 0;
	int ret;

	fw = devm_kzalloc(dev, sizeof(*fw), GFP_KERNEL);
	if (!fw)
		return -ENOMEM;

	platform_set_drvdata(pdev, fw);

	ret = request_firmware(&docxo_fw, "docxo", &pdev->dev);
	if (ret) 
		return -ENODEV;

	hdocxo = (struct docxo_header *)docxo_fw->data;
	dev_info(&pdev->dev, "File magic: %08X\n", be32_to_cpu(hdocxo->magic));

	for (drecord = &hdocxo->record; drecord != NULL; 
				drecord = docxo_next_record(drecord)) {
		__u32 record_id = be32_to_cpu(drecord->id);
		__u32 data_len = be32_to_cpu(drecord->len);

		dev_info(&pdev->dev, "Record id: %08X data len: %i\n",
						record_id, data_len);

		for (i = 0; i < data_len; i++)
			dev_info(&pdev->dev, "%02X ", drecord->data[i]);

		dev_info(&pdev->dev, "----------------------------\n");
	}

	release_firmware(docxo_fw);	
	dev_info(&pdev->dev, "Firmware experiments loaded\n");

	return 0;
}

static int firmware_remove(struct platform_device *pdev)
{
	struct docxo *fw = platform_get_drvdata(pdev);

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
