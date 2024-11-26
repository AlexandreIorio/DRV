// SPDX-License-Identifier: GPL-2.0
/*
 * ex1 file
 */

#include "linux/init.h"
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "drivify"

///@brief the probe method called when the device is detected
///@param pdev the platform device detected
///@return 0 if no error
static int drivify_probe(struct platform_device *pdev);

///@brief the remove method called when the device is removed
///@param pdev the platform device removed
///@return 0 if no error
static int drivify_remove(struct platform_device *pdev);

///@brief the init method called when the module is loaded manually
///@return 0 if no error
static int __init drivify_init(void);

///@brief the exit method called when the module is removed manually
///@return 0 if no error
static void drivify_exit(void);

///@brief the uevent method called when the device is detected
///@param dev the device detected
///@param env the environment of the device
///@return 0 if no error
static int drivify_uevent(struct device *dev, struct kobj_uevent_env *env);

///@brief the matching table of the device
static const struct of_device_id drivify_of_match[] = {
	{
		.compatible = "drv2024",
	},
	{},
};

///@brief the platform driver that knows the probe and remove methods
static struct platform_driver drivify_driver = {
	.probe = drivify_probe,
	.remove = drivify_remove,
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = drivify_of_match,
	},
};

///@brief the file operations of the device
static struct file_operations drivify_fops = {
	.owner = THIS_MODULE,
};

///@brief the private structure of the device
struct priv {
	struct class *cl;
	struct device *dev;
	struct cdev *cdev;
	dev_t majmin;
};

static int drivify_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	// Set the permissions of the device file
	add_uevent_var(env, "DEVMODE=%#o", 0666);
	return 0;
}

static int __init drivify_init(void)
{
	int ret;
	pr_info("[%s]: Initializing\n", DEVICE_NAME);
	ret = platform_driver_register(&drivify_driver);
	return 0;
}

static void __exit drivify_exit(void)
{
	pr_info("[%s]: Exiting\n", DEVICE_NAME);
	platform_driver_unregister(&drivify_driver);
}

static int drivify_probe(struct platform_device *pdev)
{
	int err;
	struct priv *priv;

	pr_info("[%s]: Probing\n", DEVICE_NAME);

	priv = devm_kzalloc(&pdev->dev, sizeof(struct priv), GFP_KERNEL);
	if (!priv) {
		pr_err("[%s]: Error allocating private structure\n",
		       DEVICE_NAME);
		return -ENOMEM;
	}

	priv->cl = class_create(THIS_MODULE, DEVICE_NAME);
	if (!priv->cl) {
		pr_err("[%s]: Error creating class\n", DEVICE_NAME);
		goto ERR_CLASS;
	}

	priv->cl->dev_uevent = drivify_uevent;

	err = alloc_chrdev_region(&priv->majmin, 0, 1, DEVICE_NAME);
	if (err < 0) {
		pr_err("[%s]: Error allocating char device\n", DEVICE_NAME);
		goto ALLOC_CHRDEV;
	}

	priv->dev =
		device_create(priv->cl, NULL, priv->majmin, NULL, DEVICE_NAME);
	if (!priv->dev) {
		pr_err("[%s]: Error creating device\n", DEVICE_NAME);
		goto ERR_DEVICE;
	}

	priv->cdev = devm_kzalloc(&pdev->dev, sizeof(struct cdev), GFP_KERNEL);
	if (!priv->cdev) {
		pr_err("[%s]: Error initializing cdev\n", DEVICE_NAME);
		goto ERR_CDEV_INIT;
	}

	cdev_init(priv->cdev, &drivify_fops);
	err = cdev_add(priv->cdev, priv->majmin, 1);
	if (err < 0) {
		pr_err("[%s]: Adding char device failed\n", DEVICE_NAME);
		goto ERR_CDEV_ADD;
	}

	pr_info("Drivify ready!\n");
	return 0;

// error handling
ERR_CDEV_ADD:
	cdev_del(priv->cdev);
ERR_CDEV_INIT:
	device_destroy(priv->cl, priv->majmin);
ERR_DEVICE:
	unregister_chrdev_region(priv->majmin, 1);
ALLOC_CHRDEV:
	class_destroy(priv->cl);
ERR_CLASS:
	unregister_chrdev(0, DEVICE_NAME);

	return -1;
	kfree(priv);
}

static int drivify_remove(struct platform_device *pdev)
{
	struct priv *priv = platform_get_drvdata(pdev);

	pr_info("[%s]: Exiting\n", DEVICE_NAME);

	device_destroy(priv->cl, priv->majmin);
	unregister_chrdev(priv->majmin, DEVICE_NAME);
	class_destroy(priv->cl);
	kfree(priv);
	return 0;
}

module_init(drivify_init);
module_exit(drivify_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexandre Iorio");
MODULE_DESCRIPTION("Drivify - Ex1");
