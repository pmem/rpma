/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

#ifndef LIBRPMA_CMOCKA_COMPARE_AND_SWAP
#define LIBRPMA_CMOCKA_COMPARE_AND_SWAP

#include <stdbool.h>

extern bool sync_bool_compare_and_swap_void(void **ptr, void *oldval,
		void *newval);
extern bool sync_bool_compare_and_swap_int(int *ptr, int oldval, int newval);

#endif /* LIBRPMA_CMOCKA_COMPARE_AND_SWAP */
