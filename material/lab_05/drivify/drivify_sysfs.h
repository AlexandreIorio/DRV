#ifndef DRIVIFY_SYSFS_H
#define DRIVIFY_SYSFS_H

#include <linux/device.h>

/// @brief Initialize the drivify sysfs
/// @param dev The device to add the sysfs to
void init_drivify_sysfs(struct device *dev);

/// @brief Remove the drivify sysfs
/// @param dev The device to remove the sysfs from
void remove_drivify_sysfs(struct device *dev);

#endif // DRIVIFY_SYSFS_H
