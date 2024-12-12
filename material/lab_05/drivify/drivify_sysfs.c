#include "drivify_sysfs.h"
#include "linux/device.h"
#include "linux/platform_device.h"
#include "player.h"

static ssize_t drivify_current_title_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct priv *priv;

	priv = (struct priv *)dev_get_drvdata(dev);

	return sprintf(buf, "drivify current title\n");
}

static DEVICE_ATTR_RO(drivify_current_title);
// static DEVICE_ATTR_RO(drivify_current_artist);
// static DEVICE_ATTR_RW(drivify_play_cmd);
// static DEVICE_ATTR_RW(drivify_time_cmd);
// static DEVICE_ATTR_RW(drivify_current_song_duration);
// static DEVICE_ATTR_RO(drivify_playlist_total_songs);
// static DEVICE_ATTR_RO(drivify_playlist_total_duration);

void init_drivify_sysfs(struct device *dev)
{
	device_create_file(dev, &dev_attr_drivify_current_title);
}

void remove_drivify_sysfs(struct device *dev)
{
	device_remove_file(dev, &dev_attr_drivify_current_title);
}
