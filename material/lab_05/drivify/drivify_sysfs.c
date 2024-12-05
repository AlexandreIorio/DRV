#include "drivify_sysfs.h"
#include "linux/device.h"

static ssize_t drivify_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	return sprintf(buf, "drivify\n");
}

DEVICE_ATTR_RO(drivify);

void init_drivify_sysfs(struct device *dev)
{
	device_create_file(dev, &dev_attr_drivify);
}
