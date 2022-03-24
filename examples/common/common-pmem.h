/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-pmem.h -- common PMem definitions
 */

#ifndef EXAMPLES_COMMON_PMEM
#define EXAMPLES_COMMON_PMEM

#if defined USE_LIBPMEM2
#include <libpmem2.h>
#elif defined USE_LIBPMEM
#include <libpmem.h>
#endif

#if defined USE_LIBPMEM2 || defined USE_LIBPMEM
#define USE_PMEM 1
#else
#undef USE_PMEM
#endif /* USE_LIBPMEM2 || USE_LIBPMEM */

#endif /* EXAMPLES_COMMON_PMEM */
