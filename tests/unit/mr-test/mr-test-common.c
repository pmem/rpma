/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test-common.c -- the memory region unit tests's common functions
 */

#include "mr-test-common.h"

const rpma_mr_descriptor Desc_exp = DESC_EXP;
struct ibv_mr Ibv_mr = {0};
struct ibv_qp Ibv_qp;
struct ibv_context Ibv_context;

/*
 * rpma_peer_mr_reg -- a mock of rpma_peer_mr_reg()
 */
int
rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr, void *addr,
	size_t length, int access)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	struct rpma_peer_mr_reg_args *args =
				mock_type(struct rpma_peer_mr_reg_args *);
	assert_ptr_equal(peer, MOCK_PEER);
	assert_ptr_equal(addr, MOCK_PTR);
	assert_int_equal(length, MOCK_SIZE);
	assert_int_equal(access, args->access);

	*ibv_mr = args->mr;
	if (*ibv_mr == NULL) {
		Rpma_provider_error = args->verrno;
		return RPMA_E_PROVIDER;
	}

	(*ibv_mr)->addr = addr;
	(*ibv_mr)->length = length;
	(*ibv_mr)->rkey = MOCK_RKEY;

	return 0;
}

/*
 * ibv_dereg_mr -- a mock of ibv_dereg_mr()
 */
int
ibv_dereg_mr(struct ibv_mr *mr)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called.
	 * ibv_dereg_mr() will be called in rpma_mr_reg() only if:
	 * 1) rpma_peer_mr_reg() succeeded and
	 * 2) malloc() failed.
	 * In the opposite case, when:
	 * 1) malloc() succeeded and
	 * 2) rpma_peer_mr_reg() failed,
	 * ibv_dereg_mr() will not be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_int_equal(mr, MOCK_MR);

	return mock_type(int); /* errno */
}

/*
 * ibv_post_send_mock -- mock of ibv_post_send()
 */
int
ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr)
{
	struct ibv_post_send_mock_args *args =
		mock_type(struct ibv_post_send_mock_args *);

	assert_non_null(qp);
	assert_non_null(wr);
	assert_non_null(bad_wr);

	assert_int_equal(qp, args->qp);
	assert_int_equal(wr->opcode, args->opcode);
	assert_int_equal(wr->send_flags, args->send_flags);
	assert_int_equal(wr->wr_id, args->wr_id);
	assert_null(wr->next);

	return args->ret;
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	errno = mock_type(int);

	if (errno)
		return NULL;

	return __real__test_malloc(size);
}

/* common setups & teardowns */

/*
 * setup__mr_local_and_remote -- create a local and a remote
 * memory region structures
 */
int
setup__mr_local_and_remote(void **mrs_ptr)
{
	static struct mrs mrs = {0};
	int ret;

	struct prestate prestate = {MOCK_USAGE, MOCK_ACCESS, NULL};
	struct prestate *pprestate = &prestate;

	/* create a local memory region structure */
	ret = setup__reg_success((void **)&pprestate);
	mrs.local = prestate.mr;

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	/* create a remote memory region structure */
	ret = setup__mr_remote((void **)&mrs.remote);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	*mrs_ptr = &mrs;

	return 0;
}

/*
 * teardown__mr_local_and_remote -- delete a local and a remote
 * memory region structures
 */
int
teardown__mr_local_and_remote(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;
	int ret;

	struct prestate prestate = {0};
	struct prestate *pprestate = &prestate;
	prestate.mr = mrs->local;

	/* create a local memory region structure */
	ret = teardown__dereg_success((void **)&pprestate);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	/* create a remote memory region structure */
	ret = teardown__mr_remote((void **)&mrs->remote);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	return 0;
}


/*
 * setup__reg_success -- create a local memory registration object
 */
int
setup__reg_success(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* configure mocks */
	struct rpma_peer_mr_reg_args mr_reg_args;
	mr_reg_args.usage = prestate->usage;
	mr_reg_args.access = prestate->access;
	mr_reg_args.mr = MOCK_MR;
	will_return_maybe(rpma_peer_mr_reg, &mr_reg_args);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	/* pass mr to the test */
	prestate->mr = mr;

	return 0;
}

/*
 * teardown__dereg_success -- delete the local memory registration object
 */
int
teardown__dereg_success(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* configure mocks */
	will_return(ibv_dereg_mr, MOCK_OK);

	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	return 0;
}

/*
 * setup__mr_remote -- create a remote memory region structure based
 * on a pre-prepared memory region's descriptor
 */
int
setup__mr_remote(void **mr_ptr)
{
	/* configure mock */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/*
	 * create a remote memory structure based on a pre-prepared descriptor
	 */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	*mr_ptr = mr;

	return 0;
}

/*
 * teardown__mr_remote -- delete the remote memory region's structure
 */
int
teardown__mr_remote(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* delete the remote memory region's structure */
	int ret = rpma_mr_remote_delete(&mr);
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

	return 0;
}
