/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * mtt_utils.h -- header for helper functions for multihreaded tests
 */

#define DESCRIPTORS_MAX_SIZE 24

#include <stddef.h>

struct common_data {
	uint16_t data_offset;	/* user data offset */
	uint8_t mr_desc_size;	/* size of mr_desc in descriptors[] */
	uint8_t pcfg_desc_size;	/* size of pcfg_desc in descriptors[] */
	/* buffer containing mr_desc and pcfg_desc */
	char descriptors[DESCRIPTORS_MAX_SIZE];
};

void *malloc_aligned(size_t size);
