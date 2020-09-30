// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-compare_and_swap.c -- compare_and_swap mocks
 */

#include "cmocka_headers.h"
#include "cmocka-compare_and_swap.h"

/*
 * _wrap_sync_bool_compare_and_swap_void -- mock of
 * sync_bool_compare_and_swap_void()
 */
bool
__wrap_sync_bool_compare_and_swap_void(void **ptr, void *oldval, void *newval)
{
	bool ret = mock_type(int);
	if (ret)
		*ptr = newval;
	return ret;
}

/*
 * _wrap_sync_bool_compare_and_swap_int --
 * mock of sync_bool_compare_and_swap_int()
 */
bool
__wrap_sync_bool_compare_and_swap_int(int *ptr, int oldval, int newval)
{
	bool ret = mock_type(int);
	if (ret)
		*ptr = newval;
	return ret;
}
