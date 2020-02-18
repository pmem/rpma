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
 * dispatcher.c -- entry points for librpma dispatcher
 */

#include <base.h>

#include "alloc.h"
#include "connection.h"
#include "dispatcher.h"
#include "os_thread.h"
#include "rpma_utils.h"
#include "sys/queue.h"
#include "zone.h"

static int
dispatcher_init(struct rpma_dispatcher *disp)
{
	PMDK_TAILQ_INIT(&disp->conn_set);
	PMDK_TAILQ_INIT(&disp->queue_wce);
	PMDK_TAILQ_INIT(&disp->queue_func);

	os_mutex_init(&disp->queue_func_mtx);

	return 0;
}

static void
dispatcher_fini(struct rpma_dispatcher *disp)
{
	os_mutex_destroy(&disp->queue_func_mtx);

	/* XXX is it ok? */
	while (!PMDK_TAILQ_EMPTY(&disp->queue_wce)) {
		struct rpma_dispatcher_wc_entry *e =
			PMDK_TAILQ_FIRST(&disp->queue_wce);
		PMDK_TAILQ_REMOVE(&disp->queue_wce, e, next);
		Free(e);
	}

	while (!PMDK_TAILQ_EMPTY(&disp->queue_func)) {
		struct rpma_dispatcher_func_entry *e =
			PMDK_TAILQ_FIRST(&disp->queue_func);
		PMDK_TAILQ_REMOVE(&disp->queue_func, e, next);
		Free(e);
	}

	while (!PMDK_TAILQ_EMPTY(&disp->conn_set)) {
		struct rpma_dispatcher_conn *e =
			PMDK_TAILQ_FIRST(&disp->conn_set);
		PMDK_TAILQ_REMOVE(&disp->conn_set, e, next);
		Free(e);
	}
}

int
rpma_dispatcher_new(struct rpma_zone *zone, struct rpma_dispatcher **disp)
{
	struct rpma_dispatcher *ptr = Malloc(sizeof(*ptr));
	if (!ptr)
		return RPMA_E_ERRNO;

	ptr->zone = zone;
	ptr->waiting = 0;

	int ret = dispatcher_init(ptr);
	if (ret)
		goto err_init;

	*disp = ptr;

	return 0;

err_init:
	Free(ptr);
	return ret;
}

int
rpma_dispatcher_delete(struct rpma_dispatcher **ptr)
{
	struct rpma_dispatcher *disp = *ptr;
	if (!disp)
		return 0;

	dispatcher_fini(disp);

	Free(disp);
	*ptr = NULL;

	return 0;
}

int
rpma_dispatcher_attach_connection(struct rpma_dispatcher *disp,
				  struct rpma_connection *conn)
{
	struct rpma_dispatcher_conn *entry = Malloc(sizeof(*entry));
	if (!entry)
		return RPMA_E_ERRNO;

	entry->conn = conn;

	PMDK_TAILQ_INSERT_TAIL(&disp->conn_set, entry, next);

	return 0;
}

int
rpma_dispatcher_detach_connection(struct rpma_dispatcher *disp,
				  struct rpma_connection *conn)
{
	struct rpma_dispatcher_conn *e = PMDK_TAILQ_FIRST(&disp->conn_set);

	while (e != NULL) {
		if (e->conn == conn) {
			PMDK_TAILQ_REMOVE(&disp->conn_set, e, next);
			Free(e);
			return 0;
		}

		e = PMDK_TAILQ_NEXT(e, next);
	}

	ASSERT(0);

	return RPMA_E_UNKNOWN_CONNECTION;
}

static int
dispatcher_cqs_process(struct rpma_dispatcher *disp)
{
	struct rpma_dispatcher_conn *e = PMDK_TAILQ_FIRST(&disp->conn_set);
	int ret = 0;

	while (e != NULL) {
		ret = rpma_connection_cq_process(e->conn);
		if (ret)
			return ret;

		e = PMDK_TAILQ_NEXT(e, next);
	}

	return ret;
}

int
rpma_dispatch(struct rpma_dispatcher *disp)
{
	struct rpma_dispatcher_wc_entry *wce;
	struct rpma_dispatcher_func_entry *funce;
	int ret;

	uint64_t *waiting = &disp->waiting;
	rpma_utils_wait_start(waiting);

	while (rpma_utils_is_waiting(waiting)) {
		ret = dispatcher_cqs_process(disp);
		if (ret)
			return ret;

		/* process cached CQ entries */
		while (!PMDK_TAILQ_EMPTY(&disp->queue_wce)) {
			wce = PMDK_TAILQ_FIRST(&disp->queue_wce);
			PMDK_TAILQ_REMOVE(&disp->queue_wce, wce, next);

			ret = rpma_connection_cq_entry_process(wce->conn,
							       NULL /* wc */);
			ASSERTeq(ret, 0); /* XXX */
			Free(wce);
		}

		while (!PMDK_TAILQ_EMPTY(&disp->queue_func)) {
			funce = PMDK_TAILQ_FIRST(&disp->queue_func);
			PMDK_TAILQ_REMOVE(&disp->queue_func, funce, next);

			ret = funce->func(funce->conn, funce->arg);
			ASSERTeq(ret, 0); /* XXX */
			Free(funce);
		}
	}

	return 0;
}

int
rpma_dispatch_break(struct rpma_dispatcher *disp)
{
	rpma_utils_wait_break(&disp->waiting);
	return 0;
}

int
rpma_dispatcher_enqueue_cq_entry(struct rpma_dispatcher *disp,
				 struct rpma_connection *conn,
				 struct ibv_wc *wc)
{
	struct rpma_dispatcher_wc_entry *entry = Malloc(sizeof(*entry));
	if (!entry)
		return RPMA_E_ERRNO;

	entry->conn = conn;
	memcpy(&entry->wc, wc, sizeof(*wc));

	PMDK_TAILQ_INSERT_TAIL(&disp->queue_wce, entry, next);

	return 0;
}

int
rpma_dispatcher_enqueue_func(struct rpma_dispatcher *disp,
			     struct rpma_connection *conn, rpma_queue_func func,
			     void *arg)
{
	struct rpma_dispatcher_func_entry *entry = Malloc(sizeof(*entry));
	entry->conn = conn;
	entry->func = func;
	entry->arg = arg;

	os_mutex_lock(&disp->queue_func_mtx);
	PMDK_TAILQ_INSERT_TAIL(&disp->queue_func, entry, next);
	os_mutex_unlock(&disp->queue_func_mtx);

	return 0;
}
