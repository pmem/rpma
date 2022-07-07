// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * server_rpma_empty.c -- empty implementation of test-specific functions for common server
 *                        of multi-connection MT tests
 */

#include "server_rpma_conn_common.h"

/*
 * client_completion_event_handle -- empty implementation of callback on completion is ready
 */
void
client_completion_event_handle(struct custom_event *ce)
{
}

/*
 * client_is_ready_handle -- empty implementation of callback on connection is established
 */
void
client_is_ready_handle(struct client_res *clnt)
{
}
