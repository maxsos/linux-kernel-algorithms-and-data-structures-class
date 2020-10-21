#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>

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

static int dummy_open(struct inode *inode, struct file *file)
{
	printk("%s: opened.\n", DUMMY_MODULE_NAME);

	return 0;
}

static int dummy_release (struct inode *inode, struct file *file)
{
	printk("%s: closed.\n", DUMMY_MODULE_NAME);

	return 0;
}

static struct file_operations dummy_fops = {
	.owner = THIS_MODULE,
	.open = dummy_open,
	.release = dummy_release
};

static int dummy_chrdev_init(void)
{
	int error;
	dev_t first_dev;
	unsigned int major, first_minor;
	int i;

	error = alloc_chrdev_region(&first_dev, DUMMY_MINOR, DUMMY_NUM_MINORS,
				    DUMMY_MODULE_NAME);
	if (error) {
		pr_err("%s: alloc_chrdev_region failed: %d\n",
		       __FUNCTION__, error);
		return error;
	}

	major = MAJOR(first_dev);
	first_minor = MINOR(first_dev);
	BUG_ON(first_minor + DUMMY_NUM_MINORS > MINORMASK);

	for (i = 0; i < DUMMY_NUM_MINORS; i++) {
		dummy_device_data[i].dev = MKDEV(major, first_minor + i);
	}

	return 0;
}

static int dummy_cdev_init(struct dummy_device_data *device_data)
{
	int error;
	struct cdev *cdev = &device_data->cdev;

	cdev_init(cdev, &dummy_fops);
	error = cdev_add(cdev, device_data->dev, 1);
	if (error) {
		pr_err("%s: cdev_add failed: %d\n",
		       __FUNCTION__, error);
	}

	printk("%s: created character device %x:%x (ma:mi).\n", DUMMY_MODULE_NAME,
	       MAJOR(device_data->dev), MINOR(device_data->dev));

	return error;
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
