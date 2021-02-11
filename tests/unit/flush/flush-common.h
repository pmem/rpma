/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2021, Intel Corporation */

/*
 * flush-common.h -- header of the common part of unit tests
 * of the flush module
 */

#ifndef FLUSH_COMMON_H
#define FLUSH_COMMON_H 1

#include "mocks-stdlib.h"

#define MOCK_RPMA_MR_REMOTE	(struct rpma_mr_remote *)0xC412
#define MOCK_RPMA_MR_LOCAL	(struct rpma_mr_local *)0xC411
#define MOCK_REMOTE_OFFSET	(size_t)0xC414
#define MOCK_LEN		(size_t)0xC415
#define MOCK_FLAGS		(int)0xC416
#define MOCK_OP_CONTEXT		(void *)0xC417
#define MOCK_RAW_LEN		8

/*
 * All the resources used between setup__flush_new and teardown__flush_delete.
 */
struct flush_test_state {
	struct rpma_flush *flush;
	struct mmap_args allocated_raw;
};

int setup__flush_new(void **fstate_ptr);
int teardown__flush_delete(void **fstate_ptr);

#endif /* FLUSH_COMMON_H */
