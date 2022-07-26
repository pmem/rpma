// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * mocks-rpma-srq.c -- librpma srq.c module mocks
 */

#include "librpma.h"

#include "cmocka_headers.h"

/*
 * rpma_srq_get_rcq -- rpma_srq_get_rcq() mock
 */
int
rpma_srq_get_rcq(const struct rpma_srq *srq, struct rpma_cq **rcq_ptr)
{
	check_expected_ptr(srq);

	*rcq_ptr = mock_type(struct rpma_cq *);

	return 0;
}

/*
 * rpma_srq_get_ibv_srq -- rpma_srq_get_ibv_srq() mock
 */
struct ibv_srq *
rpma_srq_get_ibv_srq(const struct rpma_srq *srq)
{
	check_expected_ptr(srq);

	return mock_type(struct ibv_srq *);
}
