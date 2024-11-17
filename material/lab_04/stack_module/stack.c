// SPDX-License-Identifier: GPL-2.0
/*
 * Stack file
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/string.h>

#define MAJOR_NUM 98
#define MAJMIN MKDEV(MAJOR_NUM, 0)
#define DEVICE_NAME "stack"

static struct cdev cdev;
static struct class *cl;

static LIST_HEAD(stack);

struct element {
	int data;
	struct list_head list;
};

static int get_stack_size(void)
{
	int size = 0;
	struct element *element;

	list_for_each_entry(element, &stack, list)
		size++;

	return size;
}

/**
 * @brief Pop and return latest added element of the stack.
 *
 * @param filp pointer to the file descriptor in use
 * @param buf destination buffer in user space
 * @param count maximum number of byte to read
 * @param ppos ignored
 *
 * @return Actual number of bytes read from internal buffer,
 *         or a negative error code
 */
static ssize_t stack_read(struct file *filp, char __user *buf, size_t count,
			  loff_t *ppos)
{
	ssize_t nb_values, stack_size, i;
	uint32_t *read_values;
	struct element *element;
	pr_info("Stack: Reading %d bytes\n", count);
	if (count % sizeof(uint32_t) != 0) {
		return -EINVAL;
	}

	nb_values = count / sizeof(uint32_t);
	stack_size = get_stack_size();

	if (stack_size == 0) {
		printk("Stack: Stack is empty\n");
		return 0;
	}

	// Check the current stack size and the number of values requested
	if (nb_values > stack_size) {
		nb_values = stack_size;
	}

	read_values = kmalloc(nb_values * sizeof(uint32_t), GFP_KERNEL);
	if (!read_values)
		return -ENOMEM;

	for (i = 0; i < nb_values; i++) {
		element = list_first_entry(&stack, struct element, list);
		read_values[i] = element->data;
		list_del(&element->list);
		kfree(element);
	}

	if (copy_to_user(buf, read_values, nb_values * sizeof(uint32_t)) != 0) {
		kfree(read_values);
		pr_err("Stack: Failed to copy data to user\n");
		return -EFAULT;
	}
	kfree(read_values);

	return nb_values * sizeof(uint32_t);
}

/**
 * @brief Push the element on the stack
 *
 * @param filp pointer to the file descriptor in use
 * @param buf source buffer in user space
 * @param count number of byte to write in the buffer
 * @param ppos ignored
 *
 * @return Actual number of bytes writen to internal buffer,
 *         or a negative error code
 */
static ssize_t stack_write(struct file *filp, const char __user *buf,
			   size_t count, loff_t *ppos)

{
	ssize_t nb_values;
	uint32_t *new_values;
	struct element *elements;
	ssize_t i;

	if (count % sizeof(uint32_t) != 0)
		return -EINVAL;

	nb_values = count / sizeof(uint32_t);
	new_values = kmalloc(count, GFP_KERNEL);

	if (!new_values)
		return -ENOMEM;

	if (copy_from_user(new_values, buf, count) != 0) {
		kfree(new_values);
		pr_err("Stack: Failed to copy buffer from user\n");
		return -EFAULT;
	}
	elements = kmalloc_array(nb_values, sizeof(struct element), GFP_KERNEL);
	if (!elements) {
		kfree(new_values);
		pr_err("Stack: Failed to allocate memory for stack elements\n");
		return -ENOMEM;
	}
	for (i = 0; i < nb_values; i++) {
		elements[i].data = new_values[i];
		list_add(&elements[i].list, &stack);
	}
	kfree(new_values);
	return count;
}

/**
 * @brief uevent callback to set the permission on the device file
 *
 * @param dev pointer to the device
 * @param env uevent environnement corresponding to the device
 */
static int stack_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	// Set the permissions of the device file
	add_uevent_var(env, "DEVMODE=%#o", 0666);
	return 0;
}

static const struct file_operations stack_fops = {
	.owner = THIS_MODULE,
	.read = stack_read,
	.write = stack_write,
};

static int __init stack_init(void)
{
	int err;

	printk("\nStack: Initializing\n");

	// Register the device
	err = register_chrdev_region(MAJMIN, 1, DEVICE_NAME);
	if (err != 0) {
		pr_err("Stack: Registering char device failed\n");
		return err;
	}

	cl = class_create(THIS_MODULE, DEVICE_NAME);
	if (cl == NULL) {
		pr_err("Stack: Error creating class\n");
		unregister_chrdev_region(MAJMIN, 1);
		return -1;
	}
	cl->dev_uevent = stack_uevent;

	if (device_create(cl, NULL, MAJMIN, NULL, DEVICE_NAME) == NULL) {
		pr_err("Stack: Error creating device\n");
		class_destroy(cl);
		unregister_chrdev_region(MAJMIN, 1);
		return -1;
	}

	cdev_init(&cdev, &stack_fops);
	err = cdev_add(&cdev, MAJMIN, 1);
	if (err < 0) {
		pr_err("Stack: Adding char device failed\n");
		device_destroy(cl, MAJMIN);
		class_destroy(cl);
		unregister_chrdev_region(MAJMIN, 1);
		return err;
	}

	pr_info("Stack ready!\n");
	return 0;
}

static void __exit stack_exit(void)
{
	struct element *element, *temp;
	list_for_each_entry_safe(element, temp, &stack, list) {
		list_del(&element->list);
		kfree(element);
	}

	// Unregister the device
	cdev_del(&cdev);
	device_destroy(cl, MAJMIN);
	class_destroy(cl);
	unregister_chrdev_region(MAJMIN, 1);

	pr_info("Stack done!\n");
}

MODULE_AUTHOR("REDS");
MODULE_LICENSE("GPL");

module_init(stack_init);
module_exit(stack_exit);
