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
  u8 data[2];
  s16 temp;

  if (!ds1624) {
    dev_err(dev, "Driver data not found\n");
    return -EIO;
  }

  mutex_lock(&ds1624->update_mutex);

  data[0] = DS1624_REG_COM_START;
  msg[0].addr = ds1624->client->addr;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &data[0];

  ret = i2c_transfer(ds1624->client->adapter, msg, 1);
  if (ret != 1) {
    dev_err(dev, "Start convert temperature failed\n");
    mutex_unlock(&ds1624->update_mutex);
    return -EIO;
  }

  //mdelay(200);

  data[0] = DS1624_REG_COM_READ_TEMP;
  msg[0].addr = ds1624->client->addr;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &data[0];

  msg[1].addr = ds1624->client->addr;
  msg[1].flags = I2C_M_RD;
  msg[1].len = 2;
  msg[1].buf = (char *)data;

  ret = i2c_transfer(ds1624->client->adapter, msg, 2);
  mutex_unlock(&ds1624->update_mutex);

  if (ret != 2) {
    dev_err(dev, "Read temperature failed\n");
    return -EIO;
  }

  temp = data[0] * 1000;
  temp += (data[1] >> 4) * 62; 

  return sprintf(buf, "%d\n", temp);
}

static ssize_t show_conf(struct device *dev, struct device_attribute *attr, 
		char *buf)
{
  struct ds1624_data *ds1624 = dev_get_drvdata(dev);
  s32 data;

  mutex_lock(&ds1624->update_mutex);

  data = i2c_smbus_read_byte_data(ds1624->client, DS1624_REG_CONF);
  if (data < 0) {
    dev_err(dev, "Read config failed\n");
    mutex_unlock(&ds1624->update_mutex);
    return -EIO;
  }
  
  mutex_unlock(&ds1624->update_mutex);

  return sprintf(buf, "0x%02X\n", data & 0xFF);
}

static ssize_t store_conf(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t count)
{
  struct ds1624_data *ds1624 = dev_get_drvdata(dev);
  int val;
  int ret;

  ret = kstrtouint(buf, 16, &val);
  if (ret)
    return ret;

  mutex_lock(&ds1624->update_mutex);

  ret = i2c_smbus_write_byte_data(ds1624->client, DS1624_REG_CONF, val & 0xFF);
  if (ret < 0) {
    dev_err(dev, "Failed to write config\n");
    mutex_unlock(&ds1624->update_mutex);
    return ret;
  }

  mutex_unlock(&ds1624->update_mutex);

  return count;
}

static DEVICE_ATTR(input, S_IRUGO, show_temp, NULL);
static DEVICE_ATTR(config, S_IRUGO | S_IWUSR, show_conf, store_conf);

static struct attribute *ds1624_attributes[] = {
  &dev_attr_input.attr,
  &dev_attr_config.attr,
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
