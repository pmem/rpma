/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * private_data-common.h -- the private_data unit tests common functions
 */

#ifndef PRIVATE_DATA_COMMON
#define PRIVATE_DATA_COMMON

#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <string.h>

#include "cmocka_headers.h"
#include "private_data.h"
#include "librpma.h"
#include "test-common.h"

/*
 * Both RDMA_CM_EVENT_CONNECT_REQUEST and RDMA_CM_EVENT_ESTABLISHED are valid.
 * After the event type check, the behavior should not differ.
 */
#define CM_EVENT_VALID	RDMA_CM_EVENT_CONNECT_REQUEST

#define DEFAULT_VALUE	"The default one"
#define DEFAULT_LEN	(strlen(DEFAULT_VALUE) + 1)

int setup__cm_event(void **edata_ptr);

int setup__private_data(void **pdata_ptr);
int teardown__private_data(void **pdata_ptr);

#endif /* PRIVATE_DATA_COMMON */
