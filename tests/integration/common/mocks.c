// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * mocks.c -- common mocks for integration tests
 */

#include <string.h>
#include <librpma.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "cmocka_headers.h"
#include "conn_cfg.h"
#include "mocks.h"

struct verbs_context Verbs_context;
struct ibv_comp_channel Ibv_comp_channel; /* mock IBV completion channel */
struct ibv_cq Ibv_cq;		/* mock IBV CQ */
struct ibv_mr Ibv_mr;		/* mock IBV MR */
struct ibv_mr Ibv_mr_raw;	/* mock IBV MR RAW */
struct ibv_mr Ibv_mr_flush;	/* mock IBV MR FLUSH */
struct rdma_cm_id Cm_id;	/* mock CM ID */
struct ibv_qp Ibv_qp;		/* mock IBV QP */

struct ibv_device Ibv_device = {{0}, 0, IBV_TRANSPORT_IB};
struct ibv_context Ibv_context = {&Ibv_device};
struct ibv_pd Ibv_pd = {&Ibv_context, 0};

#ifdef ON_DEMAND_PAGING_SUPPORTED
/* predefined IBV On-demand Paging caps */
struct ibv_odp_caps Ibv_odp_capable_caps = {
	.general_caps = IBV_ODP_SUPPORT,
	.per_transport_caps = {
			IBV_ODP_SUPPORT_WRITE | IBV_ODP_SUPPORT_READ,
			0,
			0
	},
};
#endif

/*
 * ibv_query_device -- ibv_query_device() mock
 */
int
ibv_query_device(struct ibv_context *ibv_ctx,
		struct ibv_device_attr *device_attr)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	assert_non_null(device_attr);

	int ret = mock_type(int);
	if (ret)
		return ret;

	memset(device_attr, 0, sizeof(struct ibv_device_attr));

	return 0;
}

#ifdef ON_DEMAND_PAGING_SUPPORTED
/*
 * ibv_query_device_ex_mock -- ibv_query_device_ex() mock
 */
int
ibv_query_device_ex_mock(struct ibv_context *ibv_ctx,
		const struct ibv_query_device_ex_input *input,
		struct ibv_device_attr_ex *attr,
		size_t attr_size)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	assert_null(input);
	assert_non_null(attr);
	/* attr_size is provided by ibverbs - no validation needed */

	struct ibv_odp_caps *caps = mock_type(struct ibv_odp_caps *);
	if (caps == NULL)
		return mock_type(int);

	memcpy(&attr->odp_caps, caps, sizeof(struct ibv_odp_caps));

	return 0;
}
#endif

/*
 * ibv_req_notify_cq_mock -- ibv_req_notify_cq() mock
 */
int
ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only)
{
	assert_ptr_equal(cq, MOCK_CQ);
	assert_int_equal(solicited_only, 0);

	return mock_type(int);
}

/*
 * ibv_get_cq_event -- ibv_get_cq_event() mock
 */
int
ibv_get_cq_event(struct ibv_comp_channel *channel, struct ibv_cq **cq,
		void **cq_context)
{
	check_expected_ptr(channel);
	assert_non_null(cq);
	assert_non_null(cq_context);

	errno = mock_type(int);
	if (!errno) {
		*cq = MOCK_CQ;
		*cq_context = NULL;
		return 0;
	}

	return -1;
}

/*
 * ibv_ack_cq_events -- ibv_ack_cq_events() mock
 */
void
ibv_ack_cq_events(struct ibv_cq *cq, unsigned nevents)
{
	check_expected_ptr(cq);
	assert_int_equal(nevents, 1);
}

/*
 * ibv_post_send_mock -- mock of ibv_post_send()
 */
int
ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr)
{
	assert_non_null(qp);
	assert_non_null(wr);
	assert_null(wr->next);
	assert_non_null(wr->sg_list);
	assert_non_null(wr->sg_list->addr);
	assert_non_null(wr->wr.rdma.remote_addr);
	assert_non_null(bad_wr);

	check_expected_ptr(qp);
	check_expected(wr->opcode);
	check_expected(wr->send_flags);
	check_expected(wr->wr_id);
	check_expected(wr->num_sge);
	check_expected(wr->sg_list->length);
	check_expected(wr->wr.rdma.remote_addr);

	uint64_t *pdst_addr = mock_type(uint64_t *);
	uint64_t src_offset = mock_type(uint64_t);
	assert_int_equal(wr->sg_list->addr, *pdst_addr + src_offset);

	memcpy((void *)wr->sg_list->addr,
		(void *)wr->wr.rdma.remote_addr, wr->sg_list->length);

	return mock_type(int);
}

/*
 * ibv_poll_cq_mock -- ibv_poll_cq() mock
 */
int
ibv_poll_cq_mock(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc)
{
	check_expected_ptr(cq);
	assert_int_equal(num_entries, 1);
	assert_non_null(wc);

	int result = mock_type(int);
	if (result != 1)
		return result;

	struct ibv_wc *wc_ret = mock_type(struct ibv_wc *);
	memcpy(wc, wc_ret, sizeof(struct ibv_wc));

	return 1;
}

/*
 * ibv_dereg_mr -- a mock of ibv_dereg_mr()
 */
int
ibv_dereg_mr(struct ibv_mr *mr)
{
	check_expected_ptr(mr);
	return mock_type(int); /* errno */
}

/*
 * rdma_create_id -- rdma_create_id() mock
 */
int
rdma_create_id(struct rdma_event_channel *channel,
		struct rdma_cm_id **id, void *context,
		enum rdma_port_space ps)
{
	assert_non_null(id);
	assert_null(context);
	assert_int_equal(ps, RDMA_PS_TCP);

	/* allocate (struct rdma_cm_id *) */
	*id = mock_type(struct rdma_cm_id *);
	if (*id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	memset(*id, 0, sizeof(struct rdma_cm_id));

	return 0;
}

/*
 * rdma_destroy_id -- rdma_destroy_id() mock
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected(id);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * ibv_alloc_pd -- ibv_alloc_pd() mock
 */
struct ibv_pd *
ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	check_expected_ptr(ibv_ctx);

	struct ibv_pd *pd = mock_type(struct ibv_pd *);

	if (pd != NULL)
		return pd;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, in librpma we try to deduce what really happened using
	 * the errno value.
	 */
	errno = mock_type(int);

	return NULL;
}

/*
 * ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	check_expected_ptr(pd);

	return mock_type(int);
}

/*
 * ibv_create_cq -- ibv_create_cq() mock
 */
struct ibv_cq *
ibv_create_cq(struct ibv_context *ibv_ctx, int cqe, void *cq_context,
		struct ibv_comp_channel *channel, int comp_vector)
{
	int cqe_default;
	(void) rpma_conn_cfg_get_cqe(rpma_conn_cfg_default(), &cqe_default);

	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	assert_int_equal(cqe, cqe_default);
	assert_ptr_equal(channel, MOCK_COMP_CHANNEL);
	assert_int_equal(comp_vector, 0);

	struct ibv_cq *cq = mock_type(struct ibv_cq *);
	if (!cq) {
		errno = mock_type(int);
		return NULL;
	}

	cq->channel = channel;

	return cq;
}

/*
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	check_expected_ptr(cq);

	return mock_type(int);
}

/*
 * ibv_create_comp_channel -- ibv_create_comp_channel() mock
 */
struct ibv_comp_channel *
ibv_create_comp_channel(struct ibv_context *ibv_ctx)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);

	struct ibv_comp_channel *channel = mock_type(struct ibv_comp_channel *);
	if (!channel) {
		errno = mock_type(int);
		return NULL;
	}

	return channel;
}

/*
 * ibv_destroy_comp_channel -- ibv_destroy_comp_channel() mock
 */
int
ibv_destroy_comp_channel(struct ibv_comp_channel *channel)
{
	assert_ptr_equal(channel, MOCK_COMP_CHANNEL);

	return mock_type(int);
}

/*
 * rdma_accept -- rdma_accept() mock
 */
int
rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected(id);
	assert_non_null(conn_param);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_connect -- rdma_connect() mock
 */
int
rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected(id);
	assert_non_null(conn_param);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_disconnect -- rdma_disconnect() mock
 */
int
rdma_disconnect(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_resolve_route -- rdma_resolve_route() mock
 */
int
rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms)
{
	check_expected(id);
	assert_int_equal(timeout_ms, MOCK_TIMEOUT);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_get_cm_event -- rdma_get_cm_event() mock
 */
int
rdma_get_cm_event(struct rdma_event_channel *channel,
		struct rdma_cm_event **event_ptr)
{
	check_expected_ptr(channel);
	assert_non_null(event_ptr);

	struct rdma_cm_event *event = mock_type(struct rdma_cm_event *);
	if (!event) {
		errno = mock_type(int);
		return -1;
	}

	*event_ptr = event;
	return 0;
}

/*
 * rdma_ack_cm_event -- rdma_ack_cm_event() mock
 */
int
rdma_ack_cm_event(struct rdma_cm_event *event)
{
	check_expected_ptr(event);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_create_event_channel -- rdma_create_event_channel() mock
 */
struct rdma_event_channel *
rdma_create_event_channel(void)
{
	struct rdma_event_channel *evch =
		mock_type(struct rdma_event_channel *);
	if (!evch) {
		errno = mock_type(int);
		return NULL;
	}

	return evch;
}

/*
 * rdma_destroy_event_channel -- rdma_destroy_event_channel() mock
 */
void
rdma_destroy_event_channel(struct rdma_event_channel *channel)
{
	check_expected_ptr(channel);
}

/*
 * rdma_create_qp -- rdma_create_qp() mock
 */
int
rdma_create_qp(struct rdma_cm_id *id, struct ibv_pd *pd,
		struct ibv_qp_init_attr *qp_init_attr)
{
	check_expected_ptr(id);
	check_expected_ptr(pd);
	assert_non_null(qp_init_attr);
	assert_int_equal(qp_init_attr->qp_context, NULL);
	assert_int_equal(qp_init_attr->send_cq, MOCK_CQ);
	assert_int_equal(qp_init_attr->recv_cq, MOCK_CQ);
	assert_int_equal(qp_init_attr->srq, NULL);
	assert_int_equal(qp_init_attr->cap.max_send_wr, MOCK_DEFAULT_Q_SIZE);
	assert_int_equal(qp_init_attr->cap.max_recv_wr, MOCK_DEFAULT_Q_SIZE);
	assert_int_equal(qp_init_attr->cap.max_send_sge, MOCK_MAX_SGE);
	assert_int_equal(qp_init_attr->cap.max_recv_sge, MOCK_MAX_SGE);
	assert_int_equal(qp_init_attr->cap.max_inline_data,
		MOCK_MAX_INLINE_DATA);
	assert_int_equal(qp_init_attr->qp_type, IBV_QPT_RC);
	assert_int_equal(qp_init_attr->sq_sig_all, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

	id->qp = mock_type(struct ibv_qp *);

	return 0;
}

/*
 * rdma_destroy_qp -- rdma_destroy_qp() mock
 */
void
rdma_destroy_qp(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
}

#if defined(ibv_reg_mr)
/*
 * Since rdma-core v27.0-105-g5a750676
 * ibv_reg_mr() has been defined as a macro
 * in <infiniband/verbs.h>:
 *
 * https://github.com/linux-rdma/rdma-core/commit/5a750676e8312715100900c6336bbc98577e082b
 *
 * In order to mock the ibv_reg_mr() function
 * the `ibv_reg_mr` symbol has to be undefined first
 * and the additional ibv_reg_mr_iova2() function
 * has to be mocked, because it is called
 * by the 'ibv_reg_mr' macro.
 */
#undef ibv_reg_mr
/*
 * ibv_reg_mr_iova2 -- ibv_reg_mr_iova2() mock
 */
struct ibv_mr *
ibv_reg_mr_iova2(struct ibv_pd *pd, void *addr, size_t length,
			uint64_t iova, unsigned access)
{
	return ibv_reg_mr(pd, addr, length, (int)access);
}

#endif

/*
 * ibv_reg_mr -- ibv_reg_mr() mock
 */
struct ibv_mr *
ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access)
{
	check_expected_ptr(pd);
	check_expected(length);
	check_expected(access);

	void **paddr = mock_type(void **);
	assert_ptr_equal(addr, *paddr);

	struct ibv_mr *mr = mock_type(struct ibv_mr *);
	if (mr == NULL) {
		errno = mock_type(int);
		return NULL;
	}

	mr->addr = addr;
	mr->length = length;

	return mr;
}

/*
 * ibv_wc_status_str -- ibv_wc_status_str() mock
 */
const char *
ibv_wc_status_str(enum ibv_wc_status status)
{
	return "";
}

/*
 * rdma_migrate_id -- rdma_migrate_id() mock
 */
int
rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel)
{
	check_expected_ptr(id);
	check_expected_ptr(channel);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_reject -- rdma_reject() mock
 */
int
rdma_reject(struct rdma_cm_id *id, const void *private_data,
		uint8_t private_data_len)
{
	assert_true(0);
	return 0;
}

/*
 * rdma_listen -- rdma_listen() mock
 */
int
rdma_listen(struct rdma_cm_id *id, int backlog)
{
	check_expected_ptr(id);
	assert_int_equal(backlog, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_getaddrinfo -- rdma_getaddrinfo() mock
 */
#ifdef RDMA_GETADDRINFO_OLD_SIGNATURE
int
rdma_getaddrinfo(char *node, char *port,
		struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
#else
int
rdma_getaddrinfo(const char *node, const char *port,
		const struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
#endif
{
	assert_string_equal(node, MOCK_ADDR);

	errno = mock_type(int);
	if (errno)
		return -1;

	struct rdma_addrinfo *buf = mock_type(struct rdma_addrinfo *);
	if (port == NULL) {
		buf->ai_src_addr = NULL;
		buf->ai_dst_addr = NULL;
	} else {
		assert_string_equal(port, MOCK_PORT);
		buf->ai_src_addr = mock_type(struct sockaddr *);
		buf->ai_dst_addr = mock_type(struct sockaddr *);
	}

	*res = buf;
	return 0;
}

/*
 * rdma_freeaddrinfo -- rdma_freeaddrinfo() mock
 */
void
rdma_freeaddrinfo(struct rdma_addrinfo *res)
{
	check_expected_ptr(res);
}

/*
 * rdma_resolve_addr -- rdma_resolve_addr() mock
 */
int
rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		struct sockaddr *dst_addr, int timeout_ms)
{
	check_expected_ptr(id);
	check_expected_ptr(src_addr);
	check_expected_ptr(dst_addr);
	check_expected(timeout_ms);

	id->verbs = mock_type(struct ibv_context *);
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_bind_addr -- rdma_bind_addr() mock
 * Note: CM ID is not modified.
 */
int
rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr)
{
	check_expected_ptr(id);
	check_expected_ptr(addr);

	id->verbs = mock_type(struct ibv_context *);
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_event_str -- rdma_event_str() mock
 */
const char *
rdma_event_str(enum rdma_cm_event_type event)
{
	return "";
}

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real__test_malloc(size);
}

/*
 * __wrap_posix_memalign -- posix_memalign() mock
 */
int
__wrap_posix_memalign(void **memptr, size_t alignment, size_t size)
{
	struct posix_memalign_args *args =
		mock_type(struct posix_memalign_args *);

	*memptr = __real__test_malloc(size);

	/* save the address of the allocated memory to verify it later */
	args->ptr = *memptr;

	if (*memptr == NULL)
		return ENOMEM;

	return 0;
}

/*
 * fprintf -- fprintf() mock
 */
int
__wrap_fprintf(FILE *__restrict __stream, const char *__restrict __format, ...)
{
	return 0;
}

/*
 * create_descriptor -- create a descriptor from the given values
 */
int
create_descriptor(void *desc,
	uint64_t raddr, uint64_t size, uint32_t rkey, uint8_t usage)
{
	char *buff = (char *)desc;

	uint64_t addr = htole64(raddr);
	memcpy(buff, &addr, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	uint64_t length = htole64(size);
	memcpy(buff, &length, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	uint32_t key = htole32(rkey);
	memcpy(buff, &key, sizeof(uint32_t));
	buff += sizeof(uint32_t);

	*((uint8_t *)buff) = usage;

	return 0;
}

/*
 * __wrap_mmap -- mmap() mock
 */
void *
__wrap_mmap(void *__addr, size_t __len, int __prot,
		int __flags, int __fd, off_t __offset)
{
	void *ret = mock_type(void *);
	if (ret != (void *)MOCK_OK)
		return MAP_FAILED;

	struct mmap_args *args = mock_type(struct mmap_args *);

	void *memptr = __real__test_malloc(__len);

	/*
	 * Save the address and length of the allocated memory
	 * in order to verify it later.
	 */
	args->addr = memptr;
	args->len = __len;

	return memptr;
}

/*
 * __wrap_munmap -- munmap() mock
 */
int
__wrap_munmap(void *__addr, size_t __len)
{
	struct mmap_args *args = mock_type(struct mmap_args *);
	assert_ptr_equal(__addr, args->addr);
	assert_int_equal(__len, args->len);

	test_free(__addr);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}
