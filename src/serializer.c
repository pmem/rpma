// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * serializer.c -- librpma serialization implementations
 */

#include <endian.h>

#include "serializer.h"

/* internal librpma API */

/*
 * rpma_ser_mr_get_size -- size of the buffer for serialized memory region data
 */
size_t
rpma_ser_mr_get_size()
{
	/*
	 * Calculate a sum of rpma_mr_remote structure's fields size:
	 * sum = raddr + size + rkey + plt
	 *
	 * Note: plt has two possible values so it fits perfectly into a single
	 * uint8_t value.
	 */
	return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t) +
			sizeof(uint8_t);
}

/*
 * rpma_ser_mr_serialize -- serialize a memory region data
 */
void
rpma_ser_mr_serialize(char *buff, uint64_t addr, uint64_t length, uint32_t rkey,
		uint8_t plt)
{
	*((uint64_t *)buff) = htole64(addr);
	buff += sizeof(uint64_t);
	*((uint64_t *)buff) = htole64(length);
	buff += sizeof(uint64_t);
	*((uint32_t *)buff) = htole32(rkey);
	buff += sizeof(uint32_t);
	*((uint8_t *)buff) = plt;
}

/*
 * rpma_ser_mr_deserialize -- deserialize a memory region data
 */
void
rpma_ser_mr_deserialize(char *buff, uint64_t *addr, uint64_t *length,
		uint32_t *rkey, uint8_t *plt)
{
	*addr = le64toh(*(uint64_t *)buff);
	buff += sizeof(uint64_t);
	*length = le64toh(*(uint64_t *)buff);
	buff += sizeof(uint64_t);
	*rkey = le32toh(*(uint32_t *)buff);
	buff += sizeof(uint32_t);
	*plt = *(uint8_t *)buff;
}
