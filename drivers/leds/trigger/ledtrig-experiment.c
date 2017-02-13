#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/leds.h>

static void ex_trig_activate(struct led_classdev *led_cdev)
{

}

static void ex_trig_deactivate(struct led_classdev *led_cdev)
{

}

static struct led_trigger ex_led_trigger = {
  .name = "experimental",
  .activate = ex_trig_activate,
  .deactivate = ex_trig_deactivate,
};

static int __init ex_trig_init(void)
{
  return led_trigger_register(&ex_led_trigger);
}
module_init(ex_trig_init);

static void __exit ex_trig_exit(void)
{
  return led_trigger_unregister(&ex_led_trigger);
}
module_exit(ex_trig_exit);

MODULE_AUTHOR("Jakub Biwel <jakub.biwel@gmail.com>");
MODULE_DESCRIPTION("Experimental LED trigger");
MODULE_LICENSE("GPL");
