/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * ep.c -- librpma endpoint-related implementations
 */

#include <errno.h>

#include "cmocka_alloc.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "rpma_err.h"

struct rpma_ep {
	/* parent peer object */
	struct rpma_peer *peer;
	/* CM ID dedicated to listening for incoming connections */
	struct rdma_cm_id *id;
	/* event channel of the CM ID */
	struct rdma_event_channel *evch;
};

/* public librpma API */

/*
 * rpma_ep_listen -- create a new event channel and a new CM ID attached to
 * the event channel. Bind the CM ID to the provided addr:service pair.
 * If everything succeeds a new endpoint is created encapsulating the event
 * channel and the CM ID.
 */
int
rpma_ep_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep_ptr)
{
	if (peer == NULL || addr == NULL || service == NULL || ep_ptr == NULL)
		return RPMA_E_INVAL;

	struct rdma_event_channel *evch = NULL;
	struct rdma_cm_id *id = NULL;
	struct rpma_info *info = NULL;
	struct rpma_ep *ep = NULL;
	int ret = 0;

	evch = rdma_create_event_channel();
	if (evch == NULL) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	if (rdma_create_id(evch, &id, NULL, RDMA_PS_TCP)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_destroy_event_channel;
	}

	ret = rpma_info_new(addr, service, RPMA_INFO_PASSIVE, &info);
	if (ret)
		goto err_destroy_id;

	ret = rpma_info_assign_addr(info, id);
	if (ret)
		goto err_info_delete;

	if (rdma_listen(id, 0 /* backlog */)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	ep = Malloc(sizeof(*ep));
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
	if (ep_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_ep *ep = *ep_ptr;
	if (ep == NULL)
		return 0;

	if (rdma_destroy_id(ep->id)) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	rdma_destroy_event_channel(ep->evch);

	Free(ep);
	*ep_ptr = NULL;

	return 0;
}

/*
 * rpma_ep_next_conn_req -- get the next event in the hope it will be
 * an RDMA_CM_EVENT_CONNECT_REQUEST. If so it orders the creation
 * of a connection request object based on the obtained request.
 * If succeeds it returns a newly created object.
 */
int
rpma_ep_next_conn_req(struct rpma_ep *ep, struct rpma_conn_req **req)
{
	if (ep == NULL || req == NULL)
		return RPMA_E_INVAL;

	int ret = 0;
	struct rdma_cm_event *event = NULL;

	/* get an event */
	if (rdma_get_cm_event(ep->evch, &event)) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	/* we expect only one type of events here */
	if (event->event != RDMA_CM_EVENT_CONNECT_REQUEST) {
		ret = RPMA_E_INVAL;
		goto err_ack;
	}

	ret = rpma_conn_req_from_cm_event(ep->peer, event, req);
	if (ret)
		goto err_ack;

	return 0;

err_ack:
	(void) rdma_ack_cm_event(event);
	return ret;

}
