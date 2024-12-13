#ifndef DRIVIFY_SHARED_TYPES_H
#define DRIVIFY_SHARED_TYPES_H
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>

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

struct player {
	struct kfifo *playlist;
	void *__iomem hex_reg;
	void *__iomem led_reg;
	void *data;
	spinlock_t playlist_lock;
};

#endif // DRIVIFY_SHARED_TYPES_H
