// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * serialize.h -- librpma serialization-related internal definitions
 */

#ifndef LIBRPMA_SERIALIZATION_H
#define LIBRPMA_SERIALIZATION_H

#include <stddef.h>
#include <stdint.h>

size_t rpma_ser_mr_get_size();

/*
 * ASSUMPTIONS
 * - buff != NULL
 */
void rpma_ser_mr_serialize(char *buff, uint64_t addr, uint64_t length,
		uint32_t rkey, uint8_t plt);

/*
 * ASSUMPTIONS
 * - buff != NULL && addr != NULL && length != NULL && rkey != NULL &&
 *     plt != NULL
 */
void rpma_ser_mr_deserialize(char *buff, uint64_t *addr, uint64_t *length,
		uint32_t *rkey, uint8_t *plt);

#endif /* LIBRPMA_SERIALIZATION_H */
