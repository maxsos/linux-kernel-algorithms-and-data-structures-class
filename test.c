#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
MODULE_DESCRIPTION("My kernel module");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");

#define DUMMY_MINOR 0
#define DUMMY_NUM_MINORS 1
#define DUMMY_MODULE_NAME "dummy"

struct dummy_device_data {
	struct cdev cdev;
	dev_t dev;
};

static struct dummy_device_data dummy_device_data[DUMMY_NUM_MINORS];

static struct file_operations dummy_fops = {
	.owner = THIS_MODULE,
};

static int dummy_chrdev_init(void)
{
	/* alloc_chrdev_region() */
	return 0;
}

static int dummy_cdev_init(struct dummy_device_data *device_data)
{
	/* cdev_init() */
	/* cdev_add() */
	return 0;
}

static int dummy_init(void)
{
	int error = 0;
	int i;

	printk("%s: Hi\n", __FUNCTION__);

	error = dummy_chrdev_init();
	if (error) {
		pr_err("%s: dummy_dev_init failed: %d\n",
		       __FUNCTION__, error);
		return error;
	}

	for (i = 0; i < DUMMY_NUM_MINORS && !error; i++) {
		error = dummy_cdev_init(&dummy_device_data[i]);
	}

	if (error) {
		pr_err("%s: dummy_cdev_init failed: %d\n",
			 __FUNCTION__, error);
	}

        return 0;
}

static void dummy_exit(void)
{
        printk("%s: Bye\n", __FUNCTION__);
}

module_init(dummy_init);
module_exit(dummy_exit);
