/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * mr-common.h -- the memory region unit tests's common definitions
 */

#ifndef MR_COMMON_H
#define MR_COMMON_H

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#define MOCK_USAGE \
	((int)(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST |\
	RPMA_MR_USAGE_WRITE_SRC | RPMA_MR_USAGE_WRITE_DST))
#define MOCK_ACCESS \
	((int)(IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE |\
	IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE))
#define MOCK_PLT	RPMA_MR_PLT_PERSISTENT

#define DESC_EXP_PMEM	{0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, \
			0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, \
			0x13, 0x12, 0x11, 0x10, \
			0x01}
#define DESC_EXP_DRAM	{0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, \
			0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, \
			0x13, 0x12, 0x11, 0x10, \
			0x00}

#define MR_DESC_SIZE	21 /* sizeof(DESC_EXP_PMEM) */
#define INVALID_MR_DESC_SIZE	1

#define MOCK_DST_OFFSET		(size_t)0xC413
#define MOCK_SRC_OFFSET		(size_t)0xC414
#define MOCK_LEN		(size_t)0xC415
#define MOCK_OP_CONTEXT		(void *)0xC417
#define MOCK_DESC		(void *)0xC418

/* a state used for rpma_mr_read/_write tests */
struct mrs {
	struct rpma_mr_local *local;
	struct rpma_mr_remote *remote;
};

/* prestate structure passed to unit test functions */
struct prestate {
	int usage;
	int access;
	/* mr passed from setup to test and to teardown */
	struct rpma_mr_local *mr;
};

extern const char Desc_exp_pmem[];
extern const char Desc_exp_dram[];

/* setups & teardowns */

int setup__reg_success(void **pprestate);
int teardown__dereg_success(void **pprestate);

int setup__mr_local_and_remote(void **mrs_ptr);
int teardown__mr_local_and_remote(void **mrs_ptr);

int setup__mr_remote(void **mr_ptr);
int teardown__mr_remote(void **mr_ptr);

#endif /* MR_COMMON_H */
