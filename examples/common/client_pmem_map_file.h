/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * client_pmem_map_file.h -- a header file for both client_pmem_map_file.c
 * and client_pmem2_map_file.c
 *
 * Please see README.md for a detailed description of this example.
 */

#include "common-pmem.h"

int client_pmem_map_file(char *path, int argc, struct example_mem *mem);

void client_pmem_unmap_file(struct example_mem *mem);
