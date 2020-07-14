// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the read-to-volatile example
 *
 * The client in this example, if provided (and capable of), prepares a local
 * persistent memory and registers it as a reading source and exposes its
 * description along with other parameters required to perform an RDMA read.
 * After the connection is established the client just waits for the server to
 * disconnect.
 *
 * If the client does not have a pmem path (or it is not capable to use pmem
 * at all) it uses DRAM instead.
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_PMEM2
#include <libpmem2.h>

#define USAGE_PMEM " [<pmem-path>]"
#else
#define USAGE_PMEM
#endif

#include "common.h"

enum lang_t {en, es};

static const char *hello_str[] = {
	[en] = "Hello world!",
	[es] = "¡Hola Mundo!"
};

#define LANG_NUM	(sizeof(hello_str) / sizeof(hello_str[0]))
#define MAX_STR_SIZE	100

struct hello_t {
	enum lang_t lang;
	char str[MAX_STR_SIZE];
};

static inline void
write_hello_str(struct hello_t *hello, enum lang_t lang)
{
	hello->lang = lang;
	strncpy(hello->str, hello_str[hello->lang], MAX_STR_SIZE);
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
		fprintf(stderr,
		    "usage: %s <server_address> <service>" USAGE_PMEM "\n",
		    argv[0]);
		exit(-1);
	}

	/* read common parameters */
	char *addr = argv[1];
	char *service = argv[2];
	int ret;

	/* resources - memory region */
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	size_t data_offset = 0;
	enum rpma_mr_plt mr_plt = RPMA_MR_PLT_VOLATILE;
	int initialized = 0;
	struct rpma_mr_local *mr = NULL;
	struct hello_t *hello = NULL;


#ifdef USE_PMEM2
	/* resources - pmem2 pmem access */
	pmem2_persist_fn persist_fn = NULL;
	struct pmem2_map *map;
	int fd;

	if (argc >= 4) {
		/* map pmem */
		char *path = argv[3];
		ret = common_pmem2_map(path, &fd, &map, &data_offset,
				&initialized);
		if (ret)
			return -1;

		/* set pmem parameters */
		mr_plt = RPMA_MR_PLT_PERSISTENT;
		persist_fn = pmem2_get_persist_fn(map);
		mr_ptr = pmem2_map_get_address(map);
		mr_size = pmem2_map_get_size(map);

		/* validate pmem is big enough */
		if (mr_size - data_offset < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mr_size, KILOBYTE + data_offset);
			common_pmem2_unmap(fd, &map);
			return -1;
		}

		hello = (struct hello_t *)((uintptr_t)mr_ptr + data_offset);
	}
#endif

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		mr_ptr = malloc_aligned(KILOBYTE);
		if (mr_ptr == NULL)
			return -1;

		mr_size = KILOBYTE;
		hello = mr_ptr;
	}

	/* initialize / translate contents */
	if (initialized) {
		(void) printf("Old value: %s\n", hello->str);
		translate(hello);
	} else {
		write_hello_str(hello, en);
	}

#ifdef USE_PMEM2
	if (mr_plt == RPMA_MR_PLT_PERSISTENT) {
		persist_fn(hello, sizeof(struct hello_t));

		if (!initialized)
			common_write_signature(map);
	}
#endif

	/* calculate data for the server read */
	struct common_data data;
	data.data_len = MAX_STR_SIZE;
	data.data_offset = data_offset + offsetof(struct hello_t, str);

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
	ret = rpma_mr_reg(peer, mr_ptr, mr_size, RPMA_MR_USAGE_READ_SRC, mr_plt,
			&mr);
	if (ret) {
		print_error("rpma_mr_reg", ret);
		goto err_peer_delete;
	}

	/* receive the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.desc);
	if (ret)
		print_error("rpma_mr_get_descriptor", ret);

	/* establish a new connection to a server listening at addr:service */
	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);
	ret = client_connect(peer, addr, service, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Between the connection being established and the connection being
	 * closed the server will perform the RDMA read.
	 */

	(void) common_wait_for_conn_close_and_disconnect(&conn);

err_mr_dereg:
	/* deregister the memory region */
	ret = rpma_mr_dereg(&mr);
	if (ret)
		print_error("rpma_mr_dereg", ret);

err_peer_delete:
	/* delete the peer */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error("rpma_peer_delete", ret);


err_free:
#ifdef USE_PMEM2
	if (mr_plt == RPMA_MR_PLT_PERSISTENT) {
		common_pmem2_unmap(fd, &map);
		mr_ptr = NULL;
	}
#endif

	if (mr_ptr != NULL)
		free(mr_ptr);

	return ret;
}
