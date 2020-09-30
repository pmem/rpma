/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

#ifndef LIBRPMA_CMOCKA_COMPARE_AND_SWAP
#define LIBRPMA_CMOCKA_COMPARE_AND_SWAP

#include <stdbool.h>
#include <stdint.h>

#ifdef TEST_MOCK_COMPARE_AND_SWAP
extern bool sync_bool_compare_and_swap_void(void **ptr, void *oldval,
		void *newval);
extern bool sync_bool_compare_and_swap_int(int *ptr, int oldval, int newval);
#else
#define sync_bool_compare_and_swap_void __sync_bool_compare_and_swap
#define sync_bool_compare_and_swap_int __sync_bool_compare_and_swap
#endif
#endif /* LIBRPMA_CMOCKA_COMPARE_AND_SWAP */
