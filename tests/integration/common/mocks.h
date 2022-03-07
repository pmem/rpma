/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks.h -- common mocks for integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdio.h>

#define MOCK_ADDR		"127.0.0.1"
#define MOCK_PORT		"1234"
#define MOCK_MR			(&Ibv_mr)
#define MOCK_MR_RAW		(&Ibv_mr_raw)
#define MOCK_MR_FLUSH		(&Ibv_mr_flush)
#define MOCK_RAW_SIZE		8 /* memory region size */
#define MOCK_COMP_CHANNEL	(&Ibv_comp_channel)
#define MOCK_IBV_PD		(&Ibv_pd)
#define MOCK_CQ			(&Ibv_cq)
#define MOCK_VERBS		(&Verbs_context.context)
#define MOCK_EVCH		((struct rdma_event_channel *)0xE4C4)
#define MOCK_SRC_ADDR		((struct sockaddr *)0x0ADD)
#define MOCK_DST_ADDR		((struct sockaddr *)0x0ADE)
#define MOCK_QP			((struct ibv_qp *)0xD56A)
#define MOCK_OP_CONTEXT		((void *)0xC417)
#define MOCK_RKEY		((uint32_t)0x10111213)
#define MOCK_TIMEOUT		1000 /* RPMA_DEFAULT_TIMEOUT */
#define MOCK_DEFAULT_Q_SIZE	10 /* RPMA_DEFAULT_Q_SIZE */
#define MOCK_MAX_SGE		1 /* RPMA_MAX_SGE */
#define MOCK_MAX_INLINE_DATA	8 /* RPMA_MAX_INLINE_DATA */
#define MOCK_SIZE		1024
#define MOCK_OK			0

/*
 * Limited by the maximum length of the private data
 * for rdma_connect() in case of RDMA_PS_TCP (56 bytes).
 */
#define DESCRIPTORS_MAX_SIZE 24

struct common_data {
	uint16_t data_offset;	/* user data offset */
	uint8_t mr_desc_size;	/* size of mr_desc in descriptors[] */
	uint8_t pcfg_desc_size;	/* size of pcfg_desc in descriptors[] */
	/* buffer containing mr_desc and pcfg_desc */
	char descriptors[DESCRIPTORS_MAX_SIZE];
};

extern struct verbs_context Verbs_context;
/* mock IBV completion channel */
extern struct ibv_comp_channel Ibv_comp_channel;
extern struct ibv_context Ibv_context;	/* mock IBV context */
extern struct ibv_device Ibv_device;	/* mock IBV device */
extern struct ibv_cq Ibv_cq;		/* mock IBV CQ */
extern struct ibv_mr Ibv_mr;		/* mock IBV MR */
extern struct ibv_mr Ibv_mr_raw;	/* mock IBV MR RAW */
extern struct ibv_mr Ibv_mr_flush;	/* mock IBV MR FLUSH */
extern struct rdma_cm_id Cm_id;		/* mock CM ID */
extern struct ibv_qp Ibv_qp;		/* mock IBV QP */
extern struct ibv_pd Ibv_pd;		/* mock IBV PD */

/* predefined IBV On-demand Paging caps */
extern struct ibv_odp_caps Ibv_odp_capable_caps;

struct posix_memalign_args {
	void *ptr;
};

struct mmap_args {
	void *addr;
	size_t len;
};

#ifdef ON_DEMAND_PAGING_SUPPORTED
int ibv_query_device_ex_mock(struct ibv_context *ibv_ctx,
		const struct ibv_query_device_ex_input *input,
		struct ibv_device_attr_ex *attr,
		size_t attr_size);
#endif

int ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr);
int ibv_poll_cq_mock(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);

int ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only);

void *__real__test_malloc(size_t size);
void *__wrap__test_malloc(size_t size);
int __wrap_fprintf(FILE *__restrict __stream,
		const char *__restrict __format, ...);
int create_descriptor(void *desc,
		uint64_t raddr, uint64_t size, uint32_t rkey, uint8_t usage);
