// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * client.c -- a client of the read-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"
#include "client_pmem_map_file.h"
#include "hello.h"

#ifdef USE_PMEM
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_PMEM */

static inline void
write_hello_str(struct hello_t *hello, enum lang_t lang)
{
	hello->lang = lang;
	strncpy(hello->str, hello_str[hello->lang], KILOBYTE - 1);
	hello->str[KILOBYTE - 1] = '\0';
}

static void
translate(struct hello_t *hello)
{
	printf("translating...\n");
	enum lang_t lang = (enum lang_t)((hello->lang + 1) % LANG_NUM);
	write_hello_str(hello, lang);
}

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	int ret;

	/* resources - memory region */
	struct example_mem mem;
	mem.mr_ptr = NULL;
	mem.mr_size = 0;
	mem.data_offset = 0;
	struct rpma_mr_local *mr = NULL;
	struct hello_t *hello = NULL;

#ifdef USE_PMEM

	mem.is_pmem = 0;

	if (argc >= 4) {
		char *path = argv[3];

		ret = client_pmem_map_file(path, argc, &mem);
		if (ret)
			goto err_free;

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (mem.mr_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mem.mr_size, SIGNATURE_LEN);
			ret = -1;
			goto err_free;
		}
		mem.data_offset = SIGNATURE_LEN;

		/*
		 * The space under the offset is intended for storing the hello
		 * structure. So the total space is assumed to be at least
		 * 1 KiB + offset of the string contents.
		 */
		if (mem.mr_size - mem.data_offset < sizeof(struct hello_t)) {
			fprintf(stderr, "%s too small (%zu < %zu)\n", path,
				mem.mr_size, sizeof(struct hello_t));
			ret = -1;
			goto err_free;
		}

		hello = (struct hello_t *)
				((uintptr_t)mem.mr_ptr + mem.data_offset);

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mem.mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial value and persist it */
			write_hello_str(hello, en);
			mem.persist(hello, sizeof(struct hello_t));
			/* write the signature to mark the content as valid */
			memcpy(mem.mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			mem.persist(mem.mr_ptr, SIGNATURE_LEN);
		}
	}
#endif
	/* if no pmem support or it is not provided */
	if (mem.mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mem.mr_ptr = malloc_aligned(sizeof(struct hello_t));
		if (mem.mr_ptr == NULL)
			return -1;

		mem.mr_size = sizeof(struct hello_t);
		hello = (struct hello_t *)mem.mr_ptr;

		/* write an initial value */
		write_hello_str(hello, en);
	}

	(void) printf("Next value: %s\n", hello->str);

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_free;

	/* register the memory */
	ret = rpma_mr_reg(peer, mem.mr_ptr, mem.mr_size,
			RPMA_MR_USAGE_READ_SRC, &mr);
	if (ret)
		goto err_peer_delete;

	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(mr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	/* calculate data for the server read */
	struct common_data data = {0};
	data.data_offset = mem.data_offset + offsetof(struct hello_t, str);
	data.mr_desc_size = mr_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.descriptors[0]);
	if (ret)
		goto err_mr_dereg;

	/* establish a new connection to a server listening at addr:port */
	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);
	ret = client_connect(peer, addr, port, NULL, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Between the connection being established and the connection being
	 * closed the server will perform the RDMA read.
	 */

	(void) common_wait_for_conn_close_and_disconnect(&conn);

	/*
	 * Translate the message so the next time the greeting will be
	 * surprising.
	 */
	translate(hello);
#ifdef USE_PMEM
	if (mem.is_pmem) {
		mem.persist(hello, sizeof(struct hello_t));
	}
#endif

	(void) printf("Translation: %s\n", hello->str);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

err_free:
#ifdef USE_PMEM
	if (mem.is_pmem) {
		client_pmem_unmap_file(&mem);
	}
#endif
	if (mem.mr_ptr != NULL) {
		free(mem.mr_ptr);
	}

	return ret;
}
