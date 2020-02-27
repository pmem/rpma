/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * rpma.c -- entry points for librpma
 */

#include "librpma.h"

int
rpma_device_by_src_address(const char *addr, struct rpma_device **rdev)
{
	return RPMA_E_NOSUPP;
}

int
rpma_device_by_dst_address(const char *addr, struct rpma_device **rdev)
{
	return RPMA_E_NOSUPP;
}

int
rpma_device_delete(struct rpma_device **rdev)
{
	return RPMA_E_NOSUPP;
}

int
rpma_peer_cfg_new(struct rpma_peer_cfg **zcfg)
{
	return RPMA_E_NOSUPP;
}

int
rpma_peer_cfg_set_msg_buffer_alloc_funcs(struct rpma_peer_cfg *zcfg,
		rpma_malloc_func malloc_func, rpma_free_func free_func)
{
	return RPMA_E_NOSUPP;
}

int
rpma_peer_cfg_delete(struct rpma_peer_cfg **zcfg)
{
	return RPMA_E_NOSUPP;
}

int
rpma_peer_new(struct rpma_peer_cfg *zcfg, struct rpma_device *rdev,
		struct rpma_peer **peer)
{
	return RPMA_E_NOSUPP;
}

int
rpma_peer_delete(struct rpma_peer **peer)
{
	return RPMA_E_NOSUPP;
}

int
rpma_conn_cfg_new(struct rpma_conn_cfg **cfg)
{
	return RPMA_E_NOSUPP;
}

int
rpma_conn_cfg_set_setup_timeout(struct rpma_conn_cfg *cfg, int timeout)
{
	return RPMA_E_NOSUPP;
}

int
rpma_conn_cfg_set_op_timeout(struct rpma_conn_cfg *cfg, int timeout)
{
	return RPMA_E_NOSUPP;
}

int
rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg)
{
	return RPMA_E_NOSUPP;
}

int
rpma_connect(struct rpma_peer *peer, struct rpma_conn_cfg *ccfg,
		const char *addr, const char *service, struct rpma_conn **conn)
{
	return RPMA_E_NOSUPP;
}

int
rpma_accept(struct rpma_peer *peer, struct rpma_conn_cfg *ccfg,
		const char *addr, const char *service, struct rpma_conn **conn)
{
	return RPMA_E_NOSUPP;
}

int
rpma_conn_set_app_context(struct rpma_conn *conn, void *data)
{
	return RPMA_E_NOSUPP;
}

int
rpma_conn_get_app_context(struct rpma_conn *conn, void **data)
{
	return RPMA_E_NOSUPP;
}

int
rpma_disconnect(struct rpma_conn **conn, int flags)
{
	return RPMA_E_NOSUPP;
}

int
rpma_complete(struct rpma_conn *conn, int op_flags,
		void **op_context)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_new(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, struct rpma_memory **mem)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_get_ptr(struct rpma_memory *mem, void **ptr)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_get_size(struct rpma_memory *mem, size_t *size)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_get_id(struct rpma_memory *mem,
		struct rpma_memory_id *id)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_delete(struct rpma_memory **mem)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_remote_new(struct rpma_peer *peer, struct rpma_memory_id *id,
		struct rpma_memory_remote **rmem)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_remote_get_size(struct rpma_memory_remote *rmem, size_t *size)
{
	return RPMA_E_NOSUPP;
}

int
rpma_memory_remote_delete(struct rpma_memory_remote **rmem)
{
	return RPMA_E_NOSUPP;
}

int
rpma_read(struct rpma_conn *conn,
		struct rpma_memory *dst, size_t dst_off,
		struct rpma_memory_remote *src, size_t src_off, size_t length,
		void *op_context, int op_flags)
{
	return RPMA_E_NOSUPP;
}

int
rpma_write(struct rpma_conn *conn,
		struct rpma_memory_remote *dst, size_t dst_off,
		struct rpma_memory *src, size_t src_off, size_t length,
		void *op_context, int op_flags)
{
	return RPMA_E_NOSUPP;
}

int
rpma_commit(struct rpma_conn *conn, void *op_context, int op_flags)
{
	return RPMA_E_NOSUPP;
}
