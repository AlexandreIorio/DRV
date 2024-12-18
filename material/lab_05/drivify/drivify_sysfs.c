#include "drivify_sysfs.h"
#include "linux/device.h"
#include "player.h"

#define LIB_NAME "drivify_sysfs"

static ssize_t drivify_current_title_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct priv *priv;
	struct music current_song;
	priv = (struct priv *)dev_get_drvdata(dev);

	get_current_song(priv->player, &current_song);

	if (current_song.name[0] == '\0') {
		return sprintf(buf, "No song is playing\n");
	}

	return sprintf(buf, current_song.name);
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
	pr_info("[%s]: init sysfs \n", LIB_NAME);
	device_create_file(dev, &dev_attr_drivify_current_title);
}

void remove_drivify_sysfs(struct device *dev)
{
	pr_info("[%s]: remove sysfs\n", LIB_NAME);
	device_remove_file(dev, &dev_attr_drivify_current_title);
}
