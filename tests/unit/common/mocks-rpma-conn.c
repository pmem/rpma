// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-conn.c -- librpma conn.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-cq.h"

/*
 * rpma_conn_new -- rpma_conn_new()  mock
 */
int
rpma_conn_new(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct rpma_cq *cq, struct rpma_cq *rcq,
		struct rpma_conn **conn_ptr)
{
	assert_ptr_equal(peer, MOCK_PEER);
	check_expected_ptr(id);
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
	check_expected_ptr(rcq);

	assert_non_null(conn_ptr);

	struct rpma_conn *conn = mock_type(struct rpma_conn *);
	if (!conn) {
		int result = mock_type(int);
		/* XXX validate the errno handling */
		if (result == RPMA_E_PROVIDER)
			errno = mock_type(int);

		return result;
	}

	*conn_ptr = conn;
	return 0;
}

/*
 * rpma_conn_delete -- rpma_conn_delete()  mock
 */
int
rpma_conn_delete(struct rpma_conn **conn_ptr)
{
	assert_non_null(conn_ptr);

	struct rpma_conn *conn = *conn_ptr;
	check_expected_ptr(conn);

	int result = mock_type(int);
	if (result) {
		/* XXX validate the errno handling */
		if (result == RPMA_E_PROVIDER)
			errno = mock_type(int);

		return result;
	}

	*conn_ptr = NULL;
	return 0;
}

/*
 * rpma_conn_transfer_private_data -- rpma_conn_transfer_private_data() mock
 */
void
rpma_conn_transfer_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata)
{
	assert_non_null(conn);
	assert_non_null(pdata);
	check_expected(conn);
	check_expected(pdata->ptr);
	check_expected(pdata->len);
}
