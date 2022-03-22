/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * utils.h -- helper functions for multithreaded tests
 */

#include <stddef.h>

#define DESCRIPTORS_MAX_SIZE 24

struct common_data {
	uint16_t data_offset;	/* user data offset */
	uint8_t mr_desc_size;	/* size of mr_desc in descriptors[] */
	uint8_t pcfg_desc_size;	/* size of pcfg_desc in descriptors[] */
	/* buffer containing mr_desc and pcfg_desc */
	char descriptors[DESCRIPTORS_MAX_SIZE];
};

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_conn *conn;
	struct rpma_mr_remote *mr_ptr;
	size_t mr_size;

	/* the expected value of the private data */
	struct rpma_conn_private_data pdata_exp;
};

void *malloc_aligned(size_t size);

void get_private_data(void *prestate, struct mtt_result *tr);
