/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * rpma_utils.c -- entry points for librpma RPMA utils
 */

#include <fcntl.h>

#include <librpma.h>

#include "rpma_utils.h"

void
rpma_utils_wait_start(uint64_t *waiting)
{
	/*
	 * load and store without barriers should be good enough here.
	 * fetch_and_or are used as workaround for helgrind issue.
	 */
	util_fetch_and_or64(waiting, 1);
}

void
rpma_utils_wait_break(uint64_t *waiting)
{
	/*
	 * load and store without barriers should be good enough here.
	 * fetch_and_or are used as workaround for helgrind issue.
	 */
	util_fetch_and_and32(waiting, 0);
}

uint64_t
rpma_utils_is_waiting(uint64_t *waiting)
{
	uint64_t is_waiting;
	util_atomic_load_explicit64(waiting, &is_waiting, memory_order_acquire);
	return is_waiting;
}

int
rpma_utils_fd_set_nonblock(int fd)
{
	int ret;

	ret = fcntl(fd, F_GETFL);
	if (ret < 0)
		return RPMA_E_ERRNO;

	int flags = ret | O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret < 0)
		return RPMA_E_ERRNO;

	return 0;
}
