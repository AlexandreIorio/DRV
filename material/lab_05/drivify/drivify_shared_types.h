#ifndef DRIVIFY_SHARED_TYPES_H
#define DRIVIFY_SHARED_TYPES_H
#include <linux/kfifo.h>

struct player {
	struct kfifo *playlist;
	void *__iomem hex_reg;
	void *__iomem led_reg;
	void *parent;
	spinlock_t playlist_lock;
};

#endif // DRIVIFY_SHARED_TYPES_H
