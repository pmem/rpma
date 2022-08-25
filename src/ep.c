// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * ep.c -- librpma endpoint-related implementations
 */

#include <errno.h>
#include <stdlib.h>

#include "conn_cfg.h"
#include "conn_req.h"
#include "debug.h"
#include "info.h"
#include "librpma.h"
#include "log_internal.h"

struct rpma_ep {
	/* parent peer object */
	struct rpma_peer *peer;
	/* CM ID dedicated to listening for incoming connections */
	struct rdma_cm_id *id;
	/* event channel of the CM ID */
	struct rdma_event_channel *evch;
};

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/* public librpma API */

/*
 * rpma_ep_listen -- create a new event channel and a new CM ID attached to the event channel.
 * Bind the CM ID to the provided addr:port pair. If everything succeeds a new endpoint is created
 * encapsulating the event channel and the CM ID.
 */
int
rpma_ep_listen(struct rpma_peer *peer, const char *addr, const char *port, struct rpma_ep **ep_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});

	if (peer == NULL || addr == NULL || port == NULL || ep_ptr == NULL)
		return RPMA_E_INVAL;

	struct rdma_event_channel *evch = NULL;
	struct rdma_cm_id *id = NULL;
	struct rpma_info *info = NULL;
	struct rpma_ep *ep = NULL;
	int ret = 0;

	evch = rdma_create_event_channel();
	if (evch == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_create_event_channel()");
		return RPMA_E_PROVIDER;
	}

	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_destroy_event_channel);

	if (rdma_create_id(evch, &id, NULL, RDMA_PS_TCP)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_create_id()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_event_channel;
	}

	ret = rpma_info_new(addr, port, RPMA_INFO_PASSIVE, &info);
	if (ret)
		goto err_destroy_id;

	ret = rpma_info_bind_addr(info, id);
	if (ret)
		goto err_info_delete;

	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_info_delete);

	if (rdma_listen(id, 0 /* backlog */)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_listen()");
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	ep = malloc(sizeof(*ep));
	if (ep == NULL) {
		/* according to malloc(3) it can fail only with ENOMEM */
		ret = RPMA_E_NOMEM;
		goto err_info_delete;
	}

	ep->peer = peer;
	ep->evch = evch;
	ep->id = id;
	*ep_ptr = ep;

	/* an error at this step should not affect the final result */
	(void) rpma_info_delete(&info);

	RPMA_LOG_NOTICE("Waiting for incoming connection on %s:%s", addr, port);

	return ret;

err_info_delete:
	(void) rpma_info_delete(&info);
err_destroy_id:
	(void) rdma_destroy_id(id);
err_destroy_event_channel:
	rdma_destroy_event_channel(evch);
	return ret;
}

/*
 * rpma_ep_shutdown -- destroy the encapsulated CM ID and event channel.
 * When done delete the endpoint.
 */
int
rpma_ep_shutdown(struct rpma_ep **ep_ptr)
{
	RPMA_DEBUG_TRACE;

	int ret = 0;

	if (ep_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_ep *ep = *ep_ptr;
	if (ep == NULL)
		return 0;

	if (rdma_destroy_id(ep->id)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_destroy_id()");
		ret = RPMA_E_PROVIDER;
	}

	rdma_destroy_event_channel(ep->evch);

	free(ep);
	*ep_ptr = NULL;

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	return ret;
}

/*
 * rpma_ep_get_fd -- get a file descriptor of the event channel associated with the endpoint
 */
int
rpma_ep_get_fd(const struct rpma_ep *ep, int *fd)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (ep == NULL || fd == NULL)
		return RPMA_E_INVAL;

	*fd = ep->evch->fd;

	return 0;
}

/*
 * rpma_ep_next_conn_req -- get the next event in the hope it will be
 * an RDMA_CM_EVENT_CONNECT_REQUEST. If so it orders the creation of a connection request object
 * based on the obtained request. If succeeds it returns a newly created object.
 */
int
rpma_ep_next_conn_req(struct rpma_ep *ep, const struct rpma_conn_cfg *cfg,
		struct rpma_conn_req **req_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	RPMA_FAULT_INJECTION(RPMA_E_NO_EVENT,
	{
		errno = ENODATA;
	});

	if (ep == NULL || req_ptr == NULL)
		return RPMA_E_INVAL;

	if (cfg == NULL)
		cfg = rpma_conn_cfg_default();

	int ret = 0;
	struct rdma_cm_event *event = NULL;

	/* get an event */
	if (rdma_get_cm_event(ep->evch, &event)) {
		if (errno == ENODATA)
			return RPMA_E_NO_EVENT;

		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_get_cm_event()");
		return RPMA_E_PROVIDER;
	}

	/* we expect only one type of events here */
	RPMA_FAULT_INJECTION_GOTO(RPMA_E_INVAL, err_ack);
	if (event->event != RDMA_CM_EVENT_CONNECT_REQUEST) {
		RPMA_LOG_ERROR("Unexpected event received: %s", rdma_event_str(event->event));
		ret = RPMA_E_INVAL;
		goto err_ack;
	}

	ret = rpma_conn_req_new_from_cm_event(ep->peer, event, cfg, req_ptr);
	if (ret)
		goto err_ack;

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER,
	{
		(void) rpma_conn_req_delete(req_ptr);
		goto err_ack;
	});

	/* ACK the connection request event */
	if (rdma_ack_cm_event(event)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_ack_cm_event()");
		(void) rpma_conn_req_delete(req_ptr);
		return RPMA_E_PROVIDER;
	}

	return 0;

err_ack:
	(void) rdma_ack_cm_event(event);
	return ret;

}
