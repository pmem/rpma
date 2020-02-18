// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * pmemcommon.h -- definitions for "common" module
 */

#ifndef PMEMCOMMON_H
#define PMEMCOMMON_H 1

#include "util.h"
#include "out.h"

/*
 * common_init -- common module initialization
 */
static inline void
common_init(const char *log_prefix, const char *log_level_var,
		const char *log_file_var, int major_version,
		int minor_version)
{
	util_init();
	out_init(log_prefix, log_level_var, log_file_var, major_version,
		minor_version);
}

/*
 * common_fini -- common module cleanup
 */
static inline void
common_fini(void)
{
	out_fini();
}

#endif
