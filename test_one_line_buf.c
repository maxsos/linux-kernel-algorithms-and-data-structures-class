#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>

MODULE_DESCRIPTION("My kernel module");
MODULE_AUTHOR("maxsos");
MODULE_LICENSE("GPL");

#define DUMMY_MINOR 0
#define DUMMY_NUM_MINORS 1
#define DUMMY_MODULE_NAME "dummy"

struct dummy_device_data {
	struct cdev cdev;
	dev_t dev;
	size_t size;
	char* buffer;
};

static struct dummy_device_data dummy_device_data[DUMMY_NUM_MINORS];

static int dummy_open(struct inode *inode, struct file *file) {
	printk("%s: opened.\n", DUMMY_MODULE_NAME);
	return 0;
}


static int dummy_release(struct inode *inode, struct file *file) {
	printk("%s: closed.\n", DUMMY_MODULE_NAME);
	return 0;
}

static ssize_t dummy_read(struct file *file, char __user *user_buffer,
			  size_t size, loff_t * offset) {
	size_t len = dummy_device_data[0].size;
    printk("READ %zu//%zu \n", size, len);
    if (len <= 0)
        return 0;
	
    if (copy_to_user(user_buffer, 
                dummy_device_data[0].buffer, 
                len))
        return -EFAULT;

    vfree(dummy_device_data[0].buffer);
    dummy_device_data[0].size = 0;
	return len;
}

static ssize_t dummy_write(struct file *file, const char __user *user_buffer,
			   size_t size, loff_t * offset) {
    printk("WRITE %zu//%zu \n", size, dummy_device_data[0].size);
    if (size <= 0)
        return 0;

    dummy_device_data[0].buffer = vmalloc(size * sizeof(char));

    if (!dummy_device_data[0].buffer) {
		pr_err("%s: vmalloc failed: can't alloc memory %zu size. \n",
				__FUNCTION__, size); 
        return -1;
    }
	
    if (copy_from_user(dummy_device_data[0].buffer, user_buffer, size))
        return -EFAULT;

    dummy_device_data[0].size = size;
	return size;
}

static struct file_operations dummy_fops = {
	.owner = THIS_MODULE,
	.open = dummy_open, 
	.release = dummy_release,
	.read = dummy_read,
	.write = dummy_write,
};

static int dummy_chrdev_init(void)
{
	int error;
	dev_t first_dev;
	int i;

	error = alloc_chrdev_region(&first_dev, DUMMY_MINOR,
			DUMMY_NUM_MINORS, DUMMY_MODULE_NAME);
	
	if (error) {
		pr_err("%s: alloc_chrdev_region failed: %d. \n",
				__FUNCTION__, error);
		return error;
	}
	BUG_ON(MINOR(first_dev) + DUMMY_NUM_MINORS > MINORMASK);
	
	for(i = 0; i < DUMMY_NUM_MINORS; ++i) {
		dummy_device_data[i].dev = MKDEV(MAJOR(first_dev), 
						 MINOR(first_dev) + i);	
        dummy_device_data[i].size = 0;
        dummy_device_data[i].buffer = NULL;
	}

	return 0;
}

static int dummy_cdev_init(struct dummy_device_data *device_data)
{
	int error;

	cdev_init(&device_data->cdev, &dummy_fops);
	error = cdev_add(&device_data->cdev, device_data->dev, 1);

	if (error) {
		pr_err("%s: alloc_cdev_add failed: %d. \n",
				__FUNCTION__, error);
		return error;
	}
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
	int i;
	for(i = 0; i < DUMMY_NUM_MINORS; ++i)
		vfree(dummy_device_data[i].buffer);

	for(i = 0; i < DUMMY_NUM_MINORS; i++) {
        	/* release devs[i] fields */
        	cdev_del(&dummy_device_data[i].cdev);
	}
	unregister_chrdev_region(dummy_device_data[0].dev,
				 DUMMY_NUM_MINORS);

        printk("%s: Bye\n", __FUNCTION__);
}
module_init(dummy_init);
module_exit(dummy_exit);
