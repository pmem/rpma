/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * device.c -- entry points for librpma device
 */

#include <rdma/rdma_cma.h>

#include <librpma.h>

#include "alloc.h"
#include "device.h"
#include "info.h"
#include "rpma_utils.h"

static int
device_by_info(struct rdma_addrinfo *rai, struct ibv_context **device)
{
	struct rdma_cm_id *temp_id;
	int ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret)
		return RPMA_E_ERRNO;

	/* either bind or resolve the address */
	if (rai->ai_flags & RAI_PASSIVE) {
		ret = rdma_bind_addr(temp_id, rai->ai_src_addr);
		if (ret) {
			ret = RPMA_E_ERRNO;
			goto err_bind_addr;
		}
	} else {
		ret = rdma_resolve_addr(temp_id, rai->ai_src_addr,
				rai->ai_dst_addr, RPMA_DEFAULT_TIMEOUT);
		if (ret) {
			ret = RPMA_E_ERRNO;
			goto err_resolve_addr;
		}
	}

	/* obtain the device */
	*device = temp_id->verbs;

err_bind_addr:
err_resolve_addr:
	(void) rdma_destroy_id(temp_id);
	return ret;
}

static int
device_by_address(const char *addr, const char *service, int passive,
		struct ibv_context **device)
{
	struct rpma_info info;
	info.addr = addr;
	info.service = service;
	info.passive = passive;

	/* translate address */
	int ret = info_resolve(&info);
	if (ret)
		return ret;

	/* obtain a device by address */
	ret = device_by_info(info.rai, device);
	if (ret)
		goto err_device_by_info;

err_device_by_info:
	/* release translation info */
	info_free(&info);
	return ret;
}

int
rpma_device_by_src_address(const char *addr, struct rpma_device **rdev)
{
	/* looking for device */
	struct ibv_context *device = NULL;
	int ret = device_by_address(addr, NULL, RPMA_INFO_PASSIVE, &device);
	if (ret)
		return ret;

	ASSERTne(device, NULL);

	/* allocate a device object */
	struct rpma_device *tmp = Malloc(sizeof(*tmp));
	if (!tmp)
		return RPMA_E_ERRNO;

	tmp->device = device;
	*rdev = tmp;

	return 0;
}

int
rpma_device_by_dst_address(const char *addr, struct rpma_device **rdev)
{
	/* looking for device */
	struct ibv_context *device = NULL;
	int ret = device_by_address(addr, NULL, RPMA_INFO_ACTIVE, &device);
	if (ret)
		return ret;

	ASSERTne(device, NULL);

	/* allocate a device object */
	struct rpma_device *tmp = Malloc(sizeof(*tmp));
	if (!tmp)
		return RPMA_E_ERRNO;

	tmp->device = device;
	*rdev = tmp;

	return 0;
}

int
rpma_device_delete(struct rpma_device **rdev)
{
	Free(*rdev);
	*rdev = NULL;
	return 0;
}
