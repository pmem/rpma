/*
 * Copyright 2019-2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * memory.c -- entry points for librpma memory
 */

#include <infiniband/verbs.h>

#include "alloc.h"
#include "memory.h"
#include "out.h"
#include "rpma_utils.h"
#include "zone.h"

static int
usage_to_access(int usage)
{
	int access = 0;

	if (usage & RPMA_MR_READ_SRC) {
		RPMA_FLAG_ON(access, IBV_ACCESS_REMOTE_READ);
		RPMA_FLAG_OFF(usage, RPMA_MR_READ_SRC);
	}

	if (usage & RPMA_MR_READ_DST) {
		RPMA_FLAG_ON(access, 0); /* XXX */
		RPMA_FLAG_OFF(usage, RPMA_MR_READ_DST);
	}

	if (usage & RPMA_MR_WRITE_SRC) {
		RPMA_FLAG_ON(access, IBV_ACCESS_LOCAL_WRITE);
		RPMA_FLAG_OFF(usage, RPMA_MR_WRITE_SRC);
	}

	if (usage & RPMA_MR_WRITE_DST) {
		/*
		 * if IBV_ACCESS_REMOTE_WRITE is set, then
		 * IBV_ACCESS_LOCAL_WRITE must be set too.
		 */
		RPMA_FLAG_ON(access,
			     IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE);
		RPMA_FLAG_OFF(usage, RPMA_MR_WRITE_DST);
	}

	ASSERTeq(usage, 0);

	return access;
}

int
rpma_memory_local_new_internal(struct rpma_zone *zone, void *ptr, size_t size,
			       int access, struct rpma_memory_local **mem_ptr)
{
	int ret;

	struct ibv_mr *mr = ibv_reg_mr(zone->pd, ptr, size, access);
	if (!mr) {
		return RPMA_E_ERRNO;
	}

	struct rpma_memory_local *mem = Malloc(sizeof(*mem));
	if (!mem) {
		ret = RPMA_E_ERRNO;
		goto err_malloc;
	}

	mem->ptr = ptr;
	mem->size = size;
	mem->mr = mr;

	*mem_ptr = mem;

	return 0;

err_malloc:
	(void)ibv_dereg_mr(mr);
	return ret;
}

int
rpma_memory_local_new(struct rpma_zone *zone, void *ptr, size_t size, int usage,
		      struct rpma_memory_local **mem_ptr)
{
	int access = usage_to_access(usage);
	return rpma_memory_local_new_internal(zone, ptr, size, access, mem_ptr);
}

int
rpma_memory_local_get_ptr(struct rpma_memory_local *mem, void **ptr)
{
	*ptr = mem->ptr;

	return 0;
}

int
rpma_memory_local_get_size(struct rpma_memory_local *mem, size_t *size)
{
	*size = mem->size;

	return 0;
}

static void
memory_id_internal_hton(rpma_memory_id_internal *id)
{
	COMPILE_ERROR_ON(sizeof(id->size) != sizeof(uint64_t));

	id->raddr = htobe64(id->raddr);
	id->rkey = htobe32(id->rkey);
	id->size = htobe64(id->size);
}

static void
memory_id_internal_ntoh(rpma_memory_id_internal *id)
{
	id->raddr = be64toh(id->raddr);
	id->rkey = be32toh(id->rkey);
	id->size = be64toh(id->size);
}

int
rpma_memory_local_get_id(struct rpma_memory_local *mem,
			 struct rpma_memory_id *id)
{
	ASSERTne(id, NULL);

	rpma_memory_id_internal id_internal;
	id_internal.raddr = (uint64_t)mem->ptr;
	id_internal.rkey = mem->mr->rkey;
	id_internal.size = mem->size;
	memory_id_internal_hton(&id_internal);

	COMPILE_ERROR_ON(sizeof(id_internal) != sizeof(*id));

	memcpy(id, &id_internal, sizeof(*id));

	return 0;
}

int
rpma_memory_local_delete(struct rpma_memory_local **mem)
{
	struct rpma_memory_local *ptr = *mem;
	if (!ptr)
		return 0;

	int ret = ibv_dereg_mr(ptr->mr);
	if (!ret)
		return -ret; /* XXX wrap this into macro? */

	Free(ptr);
	*mem = NULL;

	return 0;
}

int
rpma_memory_remote_new(struct rpma_zone *zone, struct rpma_memory_id *id,
		       struct rpma_memory_remote **rmem)
{
	rpma_memory_id_internal id_internal;
	memcpy(&id_internal, id, sizeof(*id));

	memory_id_internal_ntoh(&id_internal);

	struct rpma_memory_remote *ptr = Malloc(sizeof(*ptr));
	if (!ptr)
		return RPMA_E_ERRNO;

	memcpy(ptr, &id_internal, sizeof(*ptr));

	*rmem = ptr;

	return 0;
}

int
rpma_memory_remote_get_size(struct rpma_memory_remote *rmem, size_t *size)
{
	*size = rmem->size;

	return 0;
}

int
rpma_memory_remote_delete(struct rpma_memory_remote **rmem)
{
	struct rpma_memory_remote *ptr = *rmem;

	Free(ptr);
	*rmem = NULL;

	return 0;
}
