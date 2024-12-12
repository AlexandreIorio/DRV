#include "drivify_sysfs.h"
#include "linux/device.h"

static ssize_t drivify_current_title_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	return sprintf(buf, "drivify current title\n");
}

static DEVICE_ATTR_RO(drivify_current_title);

void init_drivify_sysfs(struct device *dev)
{
	device_create_file(dev, &dev_attr_drivify_current_title);
}

void remove_drivify_sysfs(struct device *dev)
{
	device_remove_file(dev, &dev_attr_drivify_current_title);
}
