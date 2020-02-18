/*
 * Copyright 2019-2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
