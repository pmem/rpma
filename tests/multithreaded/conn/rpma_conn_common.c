// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_common.c -- common implementations of multi-connection MT tests
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"
#include "rpma_conn_common.h"

/* the client's part */

static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	(void) mtt_client_peer_new(tr, pr->addr, &pr->peer);

	/* create peer configuration */
	(void) rpma_peer_cfg_new(&pr->pcfg);

	/* set direct write to pmem */
	(void) rpma_peer_cfg_set_direct_write_to_pmem(pr->pcfg, DIRECT_WRITE_TO_PMEM);
}

static void
thread_seq_init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	/* allocate a thread state */
	struct thread_state *ts = mtt_malloc_aligned(sizeof(struct thread_state), tr);
	/* save the thread state */
	*state_ptr = ts;
	if (ts == NULL)
		return;

	/* allocate a memory for local MR */
	ts->mr_local_size = MAX_STR_LEN;
	ts->local_ptr = mtt_malloc_aligned(ts->mr_local_size, tr);
	if (ts->local_ptr == NULL)
		goto err_free;

	/* fill the source memory with an initial content for write tests */
	memset(ts->local_ptr, 0, ts->mr_local_size);
	memcpy(ts->local_ptr, STRING_TO_WRITE_SEND, LEN_STRING_TO_WRITE_SEND);

	int ret = mtt_client_connect(tr, pr->addr, pr->port, pr->peer, &ts->conn, &ts->pdata);
	if (ret)
		goto err_free;

	/* register the memory */
	ret = rpma_mr_reg(pr->peer, ts->local_ptr, ts->mr_local_size,
			RPMA_MR_USAGE_READ_DST | RPMA_MR_USAGE_WRITE_SRC, &ts->mr_local_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_mr_reg", ret);
		goto err_conn_disconnect;
	};

	/*
	 * Create a remote memory registration structure from the received
	 * descriptor.
	 */
	struct common_data *dst_data = ts->pdata.ptr;
	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0], dst_data->mr_desc_size,
			&ts->mr_remote_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_mr_remote_from_descriptor", ret);
		goto err_mr_dereg;
	};

	ret = rpma_mr_remote_get_size(ts->mr_remote_ptr, &ts->mr_remote_size);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_mr_remote_get_size", ret);
		goto err_mr_remote_delete;
	};

	/* get the connection's main CQ */
	ret = rpma_conn_get_cq(ts->conn, &ts->cq);
	if (ret)
		goto err_mr_remote_delete;

	return;

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&ts->mr_remote_ptr);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&ts->mr_local_ptr);

err_conn_disconnect:
	mtt_client_err_disconnect(&ts->conn);

err_free:
	free(ts->local_ptr);
	free(ts);
}

/*
 * wait_and_validate_completion -- wait for the completion to be ready and validate it
 */
int
wait_and_validate_completion(struct rpma_cq *cq, enum ibv_wc_opcode expected_opcode,
				uint64_t wr_id, struct mtt_result *result)
{
	struct ibv_wc wc;

	/* wait for the completion to be ready */
	int ret = rpma_cq_wait(cq);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_cq_wait() failed", ret);
		return ret;
	}

	/* get a completion of the RDMA read */
	ret = rpma_cq_get_wc(cq, 1, &wc, NULL);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_cq_get_wc() failed", ret);
		return ret;
	}

	if (wc.status != IBV_WC_SUCCESS) {
		MTT_ERR_MSG(result, "completion status is different from IBV_WC_SUCCESS", -1);
		return -1;
	}

	if (wc.opcode != expected_opcode) {
		MTT_ERR_MSG(result, "unexpected wc.opcode value", -1);
		return -1;
	}

	if (wc.wr_id != wr_id) {
		MTT_ERR_MSG(result, "wrong work request ID", -1);
		return -1;
	}

	return 0;
}

/*
 * thread_seq_fini -- deregister and free the memory region, disconnect and delete the peer object
 */
static void
thread_seq_fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct thread_state *ts = (struct thread_state *)(*state_ptr);
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret;

	if ((ret = rpma_conn_disconnect(ts->conn))) {
		MTT_RPMA_ERR(tr, "rpma_conn_disconnect", ret);
	} else {
		/* wait for the connection to be closed */
		if ((ret = rpma_conn_next_event(ts->conn, &conn_event)))
			MTT_RPMA_ERR(tr, "rpma_conn_next_event", ret);
		else if (conn_event != RPMA_CONN_CLOSED)
			MTT_ERR_MSG(tr, "rpma_conn_next_event returned an unexpected event", -1);
	}

	if ((ret = rpma_conn_delete(&ts->conn)))
		MTT_RPMA_ERR(tr, "rpma_conn_delete", ret);

	/* delete the remote memory region's structure */
	if ((ret = rpma_mr_remote_delete(&ts->mr_remote_ptr)))
		MTT_RPMA_ERR(tr, "rpma_mr_remote_delete", ret);

	/* deregister the memory region */
	if ((ret = rpma_mr_dereg(&ts->mr_local_ptr)))
		MTT_RPMA_ERR(tr, "rpma_mr_dereg", ret);

	free(ts->local_ptr);
	free(ts);
}

/*
 * prestate_fini -- deregister and free the memory region, disconnect and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	/* delete the peer configuration structure */
	if ((ret = rpma_peer_cfg_delete(&pr->pcfg)))
		MTT_RPMA_ERR(tr, "rpma_peer_cfg_delete", ret);

	if ((ret = rpma_peer_delete(&pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

/* the server's part */

struct server_prestate {
	char *addr;
	unsigned port;

	/* the expected value of the private data */
	struct rpma_conn_private_data pdata;
};

/*
 * server_main -- the main function of the server
 */
int
server_main(char *addr, unsigned port);

/*
 * server_func -- the server function of this test
 */
int
server_func(void *prestate)
{
	struct server_prestate *pst = (struct server_prestate *)prestate;
	return server_main(pst->addr, pst->port);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate client_prestate = {args.addr, args.port};
	struct server_prestate server_prestate = {args.addr, args.port};

	struct mtt_test test = {
			&client_prestate,
			prestate_init,
			thread_seq_init,
			NULL,
			thread,
			NULL,
			thread_seq_fini,
			prestate_fini,
			server_func,
			&server_prestate
	};

	return mtt_run(&test, args.threads_num);
}
