#include "drivify_sysfs.h"
#include "linux/device.h"
#include "player.h"

#define LIB_NAME "drivify_sysfs"

static ssize_t drivify_current_title_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct music current_song;
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	get_current_song(priv->player, &current_song);

	if (current_song.name[0] == '\0') {
		return sprintf(buf, "No song is playing\n");
	}

	return sprintf(buf, "%s\n", current_song.name);
}

static ssize_t drivify_current_artist_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct music current_song;
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	get_current_song(priv->player, &current_song);

	if (current_song.artist[0] == '\0') {
		return sprintf(buf, "No song is playing\n");
	}

	return sprintf(buf, "%s\n", current_song.artist);
}

static ssize_t drivify_current_duration_show(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	struct music current_song;
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	get_current_song(priv->player, &current_song);

	if (current_song.duration == 0) {
		return sprintf(buf, "No song is playing\n");
	}

	return sprintf(buf, "%d\n", current_song.duration);
}

static ssize_t drivify_playlist_total_songs_show(struct device *dev,
						 struct device_attribute *attr,
						 char *buf)
{
	uint8_t nb_songs;
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	get_nb_songs(priv->player, &nb_songs);

	return sprintf(buf, "%d\n", nb_songs);
}

static ssize_t
drivify_playlist_total_duration_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	uint32_t total_duration;
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	get_total_duration(priv->player, &total_duration);

	return sprintf(buf, "%d\n", total_duration);
}

static ssize_t drivify_play_cmd_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", get_player_state(priv->player));
}

static ssize_t drivify_play_cmd_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct priv *priv;
	priv = (struct priv *)dev_get_drvdata(dev);

	pr_info("[%s]: cmd received: %s\n", LIB_NAME, buf);

	if (strncmp(buf, "0", 1) == 0) {
		pr_info("[%s]: cmd Pause sent to player\n", LIB_NAME);
		do_pause(priv->player);
	} else if (strncmp(buf, "1", 1) == 0) {
		pr_info("[%s]: cmd Play sent to player\n", LIB_NAME);
		do_play(priv->player);
	} else {
		pr_err("[%s]: Invalid command\n", LIB_NAME);
		return -EINVAL;
	}
	return count;
}

static ssize_t drivify_time_cmd_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct priv *priv;
	uint32_t current_duration;

	priv = (struct priv *)dev_get_drvdata(dev);

	get_current_duration(priv->player, &current_duration);

	return sprintf(buf, "%d\n", current_duration);
}

static ssize_t drivify_time_cmd_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct priv *priv;
	uint32_t time;
	priv = (struct priv *)dev_get_drvdata(dev);

	if (kstrtou32(buf, 10, &time) != 0) {
		pr_err("[%s]: Invalid time\n", LIB_NAME);
		return -EINVAL;
	}

	if (set_current_duration(priv->player, time) != 0) {
		pr_err("[%s]: Invalid time\n", LIB_NAME);
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR_RO(drivify_current_title);
static DEVICE_ATTR_RO(drivify_current_artist);
static DEVICE_ATTR_RO(drivify_current_duration);
static DEVICE_ATTR_RO(drivify_playlist_total_songs);
static DEVICE_ATTR_RO(drivify_playlist_total_duration);
static DEVICE_ATTR_RW(drivify_play_cmd);
static DEVICE_ATTR_RW(drivify_time_cmd);

void init_drivify_sysfs(struct device *dev)
{
	pr_info("[%s]: init sysfs \n", LIB_NAME);
	device_create_file(dev, &dev_attr_drivify_current_title);
	device_create_file(dev, &dev_attr_drivify_current_artist);
	device_create_file(dev, &dev_attr_drivify_current_duration);
	device_create_file(dev, &dev_attr_drivify_playlist_total_songs);
	device_create_file(dev, &dev_attr_drivify_playlist_total_duration);
	device_create_file(dev, &dev_attr_drivify_play_cmd);
	device_create_file(dev, &dev_attr_drivify_time_cmd);
}

void remove_drivify_sysfs(struct device *dev)
{
	pr_info("[%s]: remove sysfs\n", LIB_NAME);
	device_remove_file(dev, &dev_attr_drivify_current_title);
	device_remove_file(dev, &dev_attr_drivify_current_artist);
	device_remove_file(dev, &dev_attr_drivify_current_duration);
	device_remove_file(dev, &dev_attr_drivify_playlist_total_songs);
	device_remove_file(dev, &dev_attr_drivify_playlist_total_duration);
	device_remove_file(dev, &dev_attr_drivify_play_cmd);
	device_remove_file(dev, &dev_attr_drivify_time_cmd);
}
