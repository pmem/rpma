/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-client_pmem_signature_check.h -- a header file for common-client_pmem_signature_check.c
 *
 * Please see README.md for a detailed description of this example.
 */

#ifndef COMMON_MAP_FILE_WITH_SIGNATURE_CHECK_H
#define COMMON_MAP_FILE_WITH_SIGNATURE_CHECK_H

#include <stdio.h>
#include "common-conn.h"
#include "common-pmem_map_file.h"
#include "common-hello.h"

int pmem_signature_check(char *path, struct hello_t *hello, struct example_mem *mem, size_t size);


#endif /* COMMON_MAP_FILE_WITH_SIGNATURE_CHECK_H */
