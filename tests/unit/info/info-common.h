/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * info-common.h -- header of the common part of unit tests of the info module
 */

#ifndef INFO_COMMON_H
#define INFO_COMMON_H 1

#include <stdlib.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"

#include <infiniband/verbs.h>

#define MOCK_SRC_ADDR	(struct sockaddr *)0x0ADD
#define MOCK_DST_ADDR	(struct sockaddr *)0x0ADE

/*
 * All the resources used between setup__new_* and teardown__delete.
 */
struct info_state {
	struct rdma_addrinfo rai;
	struct rpma_info *info;
};

int setup__new_active(void **info_state_ptr);
int setup__new_passive(void **info_state_ptr);
int teardown__delete(void **info_state_ptr);

#endif /* INFO_COMMON_H */
