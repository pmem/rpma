/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test-common.h -- the memory region unit tests's common definitions
 */

#ifndef MR_COMMON_H
#define MR_COMMON_H

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#define MOCK_PEER	(struct rpma_peer *)0x0AD0
#define MOCK_PTR	(void *)0x0001020304050607
#define MOCK_SIZE	(size_t)0x08090a0b0c0d0e0f
#define MOCK_RKEY	(uint32_t)0x10111213
#define MOCK_USAGE	(int)(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST)
#define MOCK_PLT	RPMA_MR_PLT_PERSISTENT
#define MOCK_ERRNO	(int)(123)
#define MOCK_OK		(int)0

/*
 * Calculate an expected value of required serialized memory region:
 * exp = raddr + size + rkey + plt
 *
 * Note: plt has two possible values so it fits perfectly into a single
 * uint8_t value.
 */
#define SERIALIZED_SIZE_EXP \
	(sizeof(uint64_t) * 2 + sizeof(uint32_t) + sizeof(uint8_t))

#define BUFF_EXP	"\x07\x06\x05\x04\x03\x02\x01\x00" \
			"\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08" \
			"\x13\x12\x11\x10" \
			"\x01"

/* setups & teardowns */

int setup__mr_reg_success(void **mr_ptr);

int teardown__mr_dereg_success(void **mr_ptr);

#endif /* MR_COMMON_H */
