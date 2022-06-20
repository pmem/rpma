/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-pmem.h -- common PMem definitions
 */

#ifndef COMMON_PMEM_H
#define COMMON_PMEM_H

#ifdef USE_LIBPMEM2
#include <libpmem2.h>
#elif defined USE_LIBPMEM
#include <libpmem.h>
#endif

#if defined USE_LIBPMEM2 || defined USE_LIBPMEM
#define USE_PMEM 1
#else
#undef USE_PMEM
#endif /* USE_LIBPMEM2 || USE_LIBPMEM */

#endif /* COMMON_PMEM_H */
