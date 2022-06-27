/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-map_file_with_signature_check.h - a header file for
 * common-map_file_with_signature_check.c
 */

#ifndef COMMON_MAP_FILE_WITH_SIGNATURE_CHECK_H
#define COMMON_MAP_FILE_WITH_SIGNATURE_CHECK_H

#include "common-pmem_map_file.h"

int common_pmem_map_file_with_signature_check(char *path, size_t size, struct common_mem *mem);

#endif /* COMMON_MAP_FILE_WITH_SIGNATURE_CHECK_H */
