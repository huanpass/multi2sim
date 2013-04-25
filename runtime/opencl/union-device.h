#ifndef __UNION_DEVICE_H__
#define __UNION_DEVICE_H__

#include "list.h"
#include "device.h"

struct opencl_union_device_t
{
	struct opencl_device_t *parent;
	struct list_t *devices;
};

struct opencl_union_device_t *opencl_union_device_create(struct opencl_device_t *parent, cl_uint num_devices, cl_device_id *devices);



#endif