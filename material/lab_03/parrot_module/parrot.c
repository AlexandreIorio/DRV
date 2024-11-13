#include <linux/fs.h> /* Needed for file_operations */
#include <linux/init.h> /* Needed for the macros */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/module.h> /* Needed by all modules */
#include <linux/slab.h> /* Needed for kmalloc */
#include <linux/uaccess.h> /* copy_(to|from)_user */
#include <linux/cdev.h> /* Needed for cdev */
#include <linux/string.h> /* Needed for string manipulation */

#include "parrot.h"

#define MAJOR_NUM 97
#define DEVICE_NAME "parrot"
#define PERMISSIONS 777

static char *global_buffer;
static char *initial_buffer;
static int buffer_size;

// store the device number
static dev_t dev_nbr;

// store the device character properties
static struct cdev c_device;

static struct class *cls_device;
/**
 * @brief String manipulation to put all char in upper/lower case or invert
 * them.
 *
 * @param str        String on which the manipulation are done.
 * @param swap_lower Swap all lower case letters to upper case.
 * @param swap_upper Swap all upper case letters to lower case.
 */
static void str_manip(char *str, int swap_lower, int swap_upper)
{
	while (*str != '\0') {
		if (*str >= 'a' && *str <= 'z' && swap_lower) {
			*str = *str + ('A' - 'a');
		} else if (*str >= 'A' && *str <= 'Z' && swap_upper) {
			*str = *str + ('a' - 'A');
		}

		str++;
	}
}

/**
 * @brief Device file read callback to get the current value.
 *
 * @param filp  File structure of the char device from which the value is read.
 * @param buf   Userspace buffer to which the value will be copied.
 * @param count Number of available bytes in the userspace buffer.
 * @param ppos  Current cursor position in the file (ignored).
 *
 * @return Number of bytes written in the userspace buffer.
 */
static ssize_t parrot_read(struct file *filp, char __user *buf, size_t count,
			   loff_t *ppos)
{
	if (buf == 0 || count < buffer_size) {
		return 0;
	}
	// This a simple usage of ppos to avoid infinit loop with `cat`
	// it may not be the correct way to do.
	if (*ppos != 0) {
		return 0;
	}
	*ppos = buffer_size;
	// check if the copy_to_user succeed and if the uncopied data is not 0
	if (copy_to_user(buf, global_buffer, buffer_size)) {
		return -EFAULT;
	}

	return buffer_size;
}

/**
 * @brief Device file write callback to set the current value.
 *
 * @param filp  File structure of the char device to which the value is written.
 * @param buf   Userspace buffer from which the value will be copied.
 * @param count Number of available bytes in the userspace buffer.
 * @param ppos  Current cursor position in the file.
 *
 * @return Number of bytes read from the userspace buffer.
 */

static ssize_t parrot_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	if (count == 0) {
		return 0;
	}

	*ppos = 0;

	// check if the buffer is already allocated
	if (global_buffer != NULL) {
		kfree(global_buffer);
		global_buffer = NULL;
	}

	if (initial_buffer != NULL) {
		kfree(initial_buffer);
		initial_buffer = NULL;
	}

	global_buffer = kmalloc(count + 1, GFP_KERNEL);
	initial_buffer = kmalloc(count + 1, GFP_KERNEL);

	// be sure that the allocation succeed
	if (!global_buffer) {
		return -ENOMEM;
	}

	// be sure that the allocation succeed
	if (!initial_buffer) {
		kfree(global_buffer);
		global_buffer = NULL;
		initial_buffer = NULL;
		return -ENOMEM;
	}

	// check if the copy_from_user succeed and if the uncopide data is not 0
	if (copy_from_user(global_buffer, buf, count)) {
		kfree(global_buffer);
		kfree(initial_buffer);
		global_buffer = NULL;
		initial_buffer = NULL;
		return -EFAULT;
	}

	// save the initial value
	strcpy(initial_buffer, global_buffer);

	global_buffer[count] = '\0';

	buffer_size = count + 1;

	return count;
}

/**
 * @brief Device file ioctl callback. This permits to modify the stored string.
 *        - If the command is PARROT_CMD_TOGGLE, then the letter case in
 * inverted.
 *        - If the command is PARROT_CMD_ALLCASE, then all letter will be set to
 *          upper case (arg = TO_UPPERCASE) or lower case (arg = TO_LOWERCASE)
 *
 * @param filp File structure of the char device to which ioctl is performed.
 * @param cmd  Command value of the ioctl
 * @param arg  Optionnal argument of the ioctl
 *
 * @return 0 if ioctl succeed, -1 otherwise.
 */

static long parrot_ioctl(struct file *filep, unsigned int cmd,
			 unsigned long arg)
{
	if (buffer_size == 0) {
		return -1;
	}

	switch (cmd) {
	case PARROT_CMD_TOGGLE:
		str_manip(global_buffer, 1, 1);
		break;

	case PARROT_CMD_ALLCASE:
		switch (arg) {
		case TO_UPPERCASE:
			str_manip(global_buffer, 1, 0);
			break;

		case TO_LOWERCASE:
			str_manip(global_buffer, 0, 1);
			break;

		default:
			return -1;
		}
		break;

	case PARROT_CMD_RESET:
		// restore the initial value
		strcpy(global_buffer, initial_buffer);
		break;

	default:
		break;
	}
	return 0;
}

const static struct file_operations parrot_fops = {
	.owner = THIS_MODULE,
	.read = parrot_read,
	.write = parrot_write,
	.unlocked_ioctl = parrot_ioctl,
};

static int parrot_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
	// Add the device mode to the uevent
	pr_info("Parrot Adding DEVMODE to uevent\n");
	add_uevent_var(env, "DEVMODE=%#o", PERMISSIONS);
	return 0;
}

static int __init parrot_init(void)
{
	int ret;

	// allocate regio for the device
	ret = alloc_chrdev_region(&dev_nbr, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_err("Failed to allocate char device region\n");
		return ret;
	}

	// intialize device structure
	cdev_init(&c_device, &parrot_fops);
	c_device.owner = THIS_MODULE;

	// add the device to the system
	ret = cdev_add(&c_device, dev_nbr, 1);
	if (ret < 0) {
		pr_err("Failed to add cdev\n");
		unregister_chrdev_region(dev_nbr, 1);
		return ret;
	}

	// Initialize the class for the device
	cls_device = class_create(DEVICE_NAME);
	if (IS_ERR(cls_device)) {
		pr_err("Failed to initialize class\n");
		cdev_del(&c_device);
		unregister_chrdev_region(dev_nbr, 1);
		return -1;
	}

	// set the device uevent callback
	cls_device->dev_uevent = parrot_uevent;

	// Do mknod
	if (device_create(cls_device, NULL, dev_nbr, NULL, DEVICE_NAME) ==
	    NULL) {
		pr_err("Failed to create node\n");
		class_destroy(cls_device);
		cdev_del(&c_device);
		unregister_chrdev_region(dev_nbr, 1);
		return -1;
	}

	buffer_size = 0;

	pr_info("Parrot ready!\n");
	pr_info("ioctl PARROT_CMD_TOGGLE: %u\n", PARROT_CMD_TOGGLE);
	pr_info("ioctl PARROT_CMD_ALLCASE: %lu\n", PARROT_CMD_ALLCASE);
	pr_info("ioctl PARROT_CMD_RESET: %u\n", PARROT_CMD_RESET);
	return 0;
}

static void __exit parrot_exit(void)
{
	if (global_buffer != NULL) {
		kfree(global_buffer);
		global_buffer = NULL;
	}
	// remove the device
	device_destroy(cls_device, dev_nbr);

	// remove the class
	class_destroy(cls_device);

	// remove the device node
	cdev_del(&c_device);
	unregister_chrdev_region(dev_nbr, 1);

	pr_info("Parrot done!\n");
}

MODULE_AUTHOR("REDS");
MODULE_LICENSE("GPL");

module_init(parrot_init);
module_exit(parrot_exit);
