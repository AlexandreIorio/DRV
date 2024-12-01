#include "keys.h"
#include "hex.h"
#include "music.h"
#include "player.h"
#include "playlist.h"
#include "drivify_shared_types.h"
#include <linux/init.h>
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
#include <linux/kfifo.h>

#define DEVICE_NAME "drivify"
#define USED_KEYS_MASK 0x07
#define LED_OFFSET 0x00
#define HEX_OFFSET_0_3 0x20
#define HEX_OFFSET_4_5 0x30
struct priv;

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
static void __exit drivify_exit(void);

///@brief the uevent method called when the device is detected
///@param dev the device detected
///@param env the environment of the device
///@return 0 if no error
static int drivify_uevent(struct device *dev, struct kobj_uevent_env *env);

///@brief the open method called when the device is opened
///@param inode the inode of the device
///@param filp the file pointer
///@return 0 if no error
static int drivify_open(struct inode *inode, struct file *filp);

///@brief the release method called when the device is closed
///@param inode the inode of the device
///@param filp the file pointer
///@return 0 if no error
static int drivify_release(struct inode *inode, struct file *filp);

///@brief the write method called when the device is written
///@param filp the file pointer
///@param buf the buffer to write
///@param count the number of bytes to write
///@param ppos the position in the file
///@return the number of bytes written or a negative error code
static ssize_t drivify_write(struct file *filp, const char __user *buf,
			     size_t count, loff_t *ppos);

///@brief the read method called when the device is read
///@param filp the file pointer
///@param buf the buffer to read
///@param count the number of bytes to read
///@param ppos the position in the file
///@return the number of bytes read
static ssize_t drivify_read(struct file *filp, char __user *buf, size_t count,
			    loff_t *ppos);
///@brief method to setup the irq
///@param priv the private structure of the device
///@param pdev the platform device
///@return 0 if no error
static int setup_irq(struct priv *priv, struct platform_device *pdev);

///@brief the handler of the irq
///@param irq the irq number
///@param dev_id the device id
static irqreturn_t irq_handler(int irq, void *dev_id);

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
	.open = drivify_open,
	.release = drivify_release,
	.read = drivify_read,
	.write = drivify_write,
};

///@brief the structure of the hardware registers
struct hw_registers {
	void __iomem *keys_reg;
	void __iomem *hex_0_3_reg;
	void __iomem *hex_4_5_reg;
	void __iomem *led_reg;
};

///@brief the private structure of the device
struct priv {
	struct class *cl;
	struct device *dev;
	struct cdev cdev;
	struct hw_registers *regs;
	struct player *player;
	dev_t majmin;
	bool is_open;
	bool is_running;
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

static int drivify_open(struct inode *inode, struct file *filp)
{
	struct priv *priv;

	priv = container_of(filp->f_inode->i_cdev, struct priv, cdev);
	if (!priv) {
		pr_err("[%s]: priv is NULL in open\n", DEVICE_NAME);
		return -EINVAL;
	}

	if (priv->is_open) {
		pr_err("[%s]: Device already open\n", DEVICE_NAME);
		return -EBUSY;
	}

	priv->is_open = true;
	filp->private_data = priv;
	pr_info("[%s]: Opening\n", DEVICE_NAME);

	return 0;
}

static int drivify_release(struct inode *inode, struct file *filp)
{
	struct priv *priv;

	priv = (struct priv *)filp->private_data;
	if (!priv) {
		pr_err("[%s]: priv is NULL in close\n", DEVICE_NAME);
		return -EINVAL;
	}

	priv->is_open = false;
	pr_info("[%s]: Releasing\n", DEVICE_NAME);
	return 0;
}

static ssize_t drivify_read(struct file *filp, char __user *buf, size_t count,
			    loff_t *ppos)
{
	pr_info("[%s]: Reading\n", DEVICE_NAME);
	return 0;
}

static ssize_t drivify_write(struct file *filp, const char __user *buf,
			     size_t count, loff_t *ppos)
{
	struct music music;
	struct priv *priv;

	priv = (struct priv *)filp->private_data;

	if (count > 100) {
		pr_err("[%s]: Buffer too big\n", DEVICE_NAME);
		return -EINVAL;
	}

	if (!priv) {
		pr_err("[%s]: priv is NULL in write\n", DEVICE_NAME);
		return -EINVAL;
	}

	pr_info("[%s]: Writing\n", DEVICE_NAME);
	if (copy_from_user(&music, buf, count) != 0) {
		pr_err("Failed to copy data from user\n");
		return -EFAULT;
	}

	set_music_to_playlist(priv->player->playlist, &music,
			      &priv->player->playlist_lock);

	wake_up_player(priv->player);

	return count;
}

static int drivify_probe(struct platform_device *pdev)
{
	int err;
	struct priv *priv;
	struct resource *res;
	void __iomem *base_addr;

	pr_info("[%s]: Probing\n", DEVICE_NAME);
	priv = devm_kzalloc(&pdev->dev, sizeof(struct priv), GFP_KERNEL);
	if (!priv) {
		pr_err("[%s]: Error allocating private structure\n",
		       DEVICE_NAME);
		return -ENOMEM;
	}
	priv->is_open = false;

	platform_set_drvdata(pdev, priv);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("[%s]: Error getting resource\n", DEVICE_NAME);
		return -ENODEV;
	}

	base_addr = devm_ioremap_resource(&pdev->dev, res);
	if (!base_addr) {
		pr_err("[%s]: Error mapping base address\n", DEVICE_NAME);
		return -ENOMEM;
	}

	priv->regs = devm_kzalloc(&pdev->dev, sizeof(struct hw_registers),
				  GFP_KERNEL);

	if (!priv->regs) {
		pr_err("[%s]: Error allocating registers\n", DEVICE_NAME);
		goto ERR_REGS;
	}
	priv->regs->keys_reg = (uint8_t *)base_addr + KEYS_OFFSET;
	priv->regs->hex_0_3_reg = (uint8_t *)base_addr + HEX_OFFSET_0_3;
	priv->regs->hex_4_5_reg = (uint8_t *)base_addr + HEX_OFFSET_4_5;
	priv->regs->led_reg = (uint8_t *)base_addr + LED_OFFSET;

	err = setup_irq(priv, pdev);
	if (err != 0) {
		pr_err("[%s]: Failed to setup IRQ\n", DEVICE_NAME);
		goto ERR_IRQ;
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
		device_create(priv->cl, NULL, priv->majmin, priv, DEVICE_NAME);
	if (!priv->dev) {
		pr_err("[%s]: Error creating device\n", DEVICE_NAME);
		goto ERR_DEVICE;
	}

	cdev_init(&priv->cdev, &drivify_fops);
	err = cdev_add(&priv->cdev, priv->majmin, 1);
	if (err < 0) {
		pr_err("[%s]: Adding char device failed\n", DEVICE_NAME);
		goto ERR_CDEV_ADD;
	}

	priv->player =
		devm_kzalloc(&pdev->dev, sizeof(struct player), GFP_KERNEL);
	if (!priv->player) {
		pr_err("[%s]: Error allocating kfifo\n", DEVICE_NAME);
		goto ERR_PLAYER_ALLOC;
	}

	priv->player->playlist =
		devm_kzalloc(&pdev->dev, sizeof(struct kfifo), GFP_KERNEL);

	if (!priv->player->playlist) {
		pr_err("[%s]: Error allocating playlist\n", DEVICE_NAME);
		goto ERR_KFIFO_HEAD;
	}

	err = kfifo_alloc(priv->player->playlist,
			  PLAYLIST_SIZE * sizeof(struct music), GFP_KERNEL);
	spin_lock_init(&priv->player->playlist_lock);

	if (err) {
		pr_err("[%s]: Error allocating kfifo\n", DEVICE_NAME);
		goto ERR_KFIFO_FULL_ALLOC;
	}

	keys_enable_interrupts(priv->regs->keys_reg, USED_KEYS_MASK);
	keys_clear_edge_reg(priv->regs->keys_reg, USED_KEYS_MASK);

	priv->player->hex_reg = priv->regs->hex_0_3_reg;
	priv->player->led_reg = priv->regs->led_reg;
	initialize_player(priv->player);

	pr_info("[%s]: Module ready!\n", DEVICE_NAME);
	return 0;

// error handling
ERR_KFIFO_FULL_ALLOC:
	kfifo_free(priv->player->playlist);
ERR_KFIFO_HEAD:
ERR_PLAYER_ALLOC:

ERR_CDEV_ADD:
	cdev_del(&priv->cdev);
	device_destroy(priv->cl, priv->majmin);
ERR_DEVICE:
	unregister_chrdev_region(priv->majmin, 1);
ALLOC_CHRDEV:
	class_destroy(priv->cl);
ERR_CLASS:
	keys_disable_interrupts(priv->regs->keys_reg);
ERR_IRQ:
ERR_REGS:
	platform_driver_unregister(&drivify_driver);
	return -1;
}

static int drivify_remove(struct platform_device *pdev)
{
	struct priv *priv;

	pr_info("[%s]: remove\n", DEVICE_NAME);

	priv = platform_get_drvdata(pdev);

	if (!priv) {
		pr_err("[%s]: priv is NULL in remove\n", DEVICE_NAME);
		return -EINVAL;
	}

	keys_disable_interrupts(priv->regs->keys_reg);
	stop_player(priv->player);
	kfifo_free(priv->player->playlist);
	cdev_del(&priv->cdev);
	device_destroy(priv->cl, priv->majmin);
	class_destroy(priv->cl);
	unregister_chrdev_region(priv->majmin, 1);
	pr_info("[%s]: remove completed\n", DEVICE_NAME);

	return 0;
}

static irqreturn_t irq_handler(int irq, void *dev_id)
{
	struct priv *priv;
	uint8_t key_index;

	priv = (struct priv *)dev_id;
	key_index = read_keys(priv->regs->keys_reg);
	pr_info("[%s]: Key %d pressed\n", DEVICE_NAME, key_index);
	switch (key_index) {
	case 1:
		play_pause_song(priv->player);
		break;
	case 2:
		rewind_song(priv->player);
		break;
	case 4:
		next_song(priv->player);
		break;
	default:
		break;
	}

	wake_up_player(priv->player);

	keys_clear_edge_reg(priv->regs->keys_reg, USED_KEYS_MASK);
	return IRQ_HANDLED;
}

static int setup_irq(struct priv *priv, struct platform_device *pdev)
{
	int irq_num;
	int ret;

	pr_info("[%s]: Registering IRQ\n", DEVICE_NAME);
	irq_num = platform_get_irq(pdev, 0);
	if (irq_num < 0) {
		pr_err("[%s]: Failed to get IRQ number\n", DEVICE_NAME);
		return -EINVAL;
	}

	ret = devm_request_irq(&pdev->dev, irq_num, irq_handler, 0, "drivify",
			       priv);
	if (ret < 0) {
		pr_err("[%s]: Failed to request IRQ\n", DEVICE_NAME);
		return ret;
	}
	return 0;
}

module_init(drivify_init);
module_exit(drivify_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexandre Iorio");
MODULE_DESCRIPTION("Drivify - Ex1");
