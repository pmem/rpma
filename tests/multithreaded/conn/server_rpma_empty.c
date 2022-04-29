// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * server_rpma_empty.c -- empty implementation of test-specific functions for common server
 *                        of multi-connection MT tests
 */

#include "server_rpma_common.h"

/*
 * client_handle_completion -- empty implementation of callback on completion is ready
 */
void
client_handle_completion(struct custom_event *ce)
{
}

/*
 * client_handle_is_ready -- empty implementation of callback on connection is established
 */
void
client_handle_is_ready(struct client_res *clnt)
{
}
