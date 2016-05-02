/*
 * The DS1624 device is a digital temperature with 12 bits resolution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/delay.h>

/* Supported devices */
enum devices { ds1624 };

/* The configuration/status register
 *
 * - DS1621
 *    7   6   5   4   3   2   1   0
 * |Done| X | X | X | X | X | X |1Shot|
 *
 * Where:
 * - 'X' is Reserved
 */

#define DS1624_REG_CONFIG_1SHOT 0x01
#define DS1624_REG_CONFIG_DONE 0x08

#define DS1624_REG_CONF 0xAC
#define DS1624_REG_COM_START 0xEE
#define DS1624_REG_COM_STOP 0x22
#define DS1624_REG_COM_READ_TEMP 0xAA

struct ds1624_data {
  struct i2c_client *client;
  struct mutex update_mutex;
  u16 temp;
};

static ssize_t show_temp(struct device *dev, struct device_attribute *attr,
    char *buf)
{
  int ret;
  struct i2c_msg msg[2];
  struct ds1624_data *ds1624 = dev_get_drvdata(dev);
  u8 data;
  if (!ds1624) {
    dev_err(dev, "Driver data not found\n");
    return -EIO;
  }

  mutex_lock(&ds1624->update_mutex);

  data = DS1624_REG_COM_START;
  msg[0].addr = ds1624->client->addr;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &data;

  ret = i2c_transfer(ds1624->client->adapter, msg, 1);
  if (ret != 1) {
    dev_err(dev, "Start convert temperature failed\n");
    mutex_unlock(&ds1624->update_mutex);
    return -EIO;
  }

  mdelay(750);

  data = DS1624_REG_COM_READ_TEMP;
  msg[0].addr = ds1624->client->addr;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &data;

  msg[1].addr = ds1624->client->addr;
  msg[1].flags = I2C_M_RD;
  msg[1].len = 1;
  msg[1].buf = &data;

  ret = i2c_transfer(ds1624->client->adapter, msg, 2);
  mutex_unlock(&ds1624->update_mutex);

  if (ret != 2) {
    dev_err(dev, "Read temperature failed\n");
    return -EIO;
  }

  return sprintf(buf, "%d\n", data);
}

static DEVICE_ATTR(input, S_IRUGO, show_temp, NULL);

static struct attribute *ds1624_attributes[] = {
  &dev_attr_input.attr,
  NULL
};

static const struct attribute_group ds1624_attr_grp = {
  .attrs = ds1624_attributes,
};

static int ds1624_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
  struct ds1624_data *ds1624;
  int ret = 0;

  dev_info(&client->dev, "DS1624 Probe called for device at address: 0x%04X\n",
    client->addr);

  ds1624 = devm_kzalloc(&client->dev, sizeof(struct ds1624_data), GFP_KERNEL);
  if (!ds1624) {
    dev_err(&client->dev, "Failed to allocate memory.\n");
    return -ENOMEM;
  }

  ret = sysfs_create_group(&client->dev.kobj, &ds1624_attr_grp);
  if (ret) {
    dev_err(&client->dev, "Failed to create sysfs entries.\n");
    return ret;
  }

  mutex_init(&ds1624->update_mutex);

  ds1624->client = client;

  i2c_set_clientdata(client, ds1624);

  return 0;
}

static int ds1624_remove(struct i2c_client *client)
{
  sysfs_remove_group(&client->dev.kobj, &ds1624_attr_grp);

  return 0;
}

static const struct i2c_device_id ds1624_id[] = {
  { "ds1624", ds1624 },
  { }
};

MODULE_DEVICE_TABLE(i2c, ds1624_id);

static struct i2c_driver ds1624_driver = {
  .class = I2C_CLASS_HWMON,
  .driver = {
    .name = "ds1621",
  },
  .probe = ds1624_probe,
  .remove = ds1624_remove,
  .id_table = ds1624_id,
};

module_i2c_driver(ds1624_driver);

MODULE_AUTHOR("Jakub Biwel <jakub.biwel@gmail.com>");
MODULE_DESCRIPTION("DS1624");
MODULE_LICENSE("GPL");
