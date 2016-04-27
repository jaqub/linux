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

static int ds1624_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
  dev_info(&client->dev, "DS1624 Probe called for device at address: %h\n", client->addr);

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
