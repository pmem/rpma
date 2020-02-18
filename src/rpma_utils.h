/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * rpma_utils.h -- librpma utilities functions
 */

#ifndef RPMA_UTILS_H
#define RPMA_UTILS_H 1

#include <errno.h>

#include "out.h"

#define RPMA_E_ERRNO (-errno)
#define RPMA_E_FI_ERRNO (errno)

#define ERR_STR(e, fmt, args...)                                               \
	ERR(fmt ": %s", ##args, strerror(abs((int)(e))))

#define RPMA_CM_EVENT_TYPE_INVALID UINT32_MAX
#define RPMA_FD_INVALID (-1)

#define RPMA_DEFAULT_TIMEOUT 1000

#define RPMA_FLAG_ON(set, flag) (set) |= (flag)
#define RPMA_FLAG_OFF(set, flag) (set) &= ~(flag)

void rpma_utils_wait_start(uint64_t *waiting);
void rpma_utils_wait_break(uint64_t *waiting);
uint64_t rpma_utils_is_waiting(uint64_t *waiting);

int rpma_utils_fd_set_nonblock(int fd);

#endif /* RPMA_UTILS_H */
