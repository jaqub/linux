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
#define DS1624_REG_CONFIG_DONE

#define DS1624_REG_CONF 0xAC
#define DS1624_REG_COM_START 0xEE
#define DS1624_REG_COM_STOP 0x22
#define DS1624_REG_COM_READ_TEMP 0xAA

struct ds1624_data {
  struct i2c_client *client;
  struct mutex update_mutex;
  u16 temp;
};

static int ds1624_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
  struct ds1624_data *ds1624;

  dev_info(&client->dev, "DS1624 Probe called for device at address: %h\n",
    client->addr);

  ds1624 = devm_kzalloc(&client->dev, sizeof(struct ds1624_data), GFP_KERNEL);
  if (!ds1624) {
    dev_err($client->dev, "Failed to allocate memory.\n");
    return -ENOMEM;
  }
  
  mutex_init(&ds1624->update_mutex);

  ds1624->client = client;
  i2c_set_clientdata(&client->dev, data);

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
  .id_table = ds1624_id,
};

MODULE_AUTHOR("Jakub Biwel <jakub.biwel@gmail.com>");
MODULE_DESCRIPTION("DS1624");
MODULE_LICENSE("GPL");
