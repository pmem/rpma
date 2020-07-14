// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the read-to-persistent example
 *
 * The client in this example, if provided (and capable of), prepares a local
 * persistent memory (including its contents) and registers it as a reading
 * source and exposes the memory description along with other parameters
 * required to perform an RDMA read. After the connection is established
 * the client just waits for the server to disconnect.
 *
 * If the client does not have a pmem path (or it is not capable to use pmem
 * at all) it uses DRAM instead.
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_LIBPMEM
#include <libpmem.h>

#define USAGE_PMEM " [<pmem-path>]"
#else
#define USAGE_PMEM
#endif /* USE_LIBPMEM */

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
	int has_content = 0;
	struct rpma_mr_local *mr = NULL;
	struct hello_t *hello = NULL;


#ifdef USE_LIBPMEM
	if (argc >= 4) {
		char *path = argv[3];
		int is_pmem = 0;

		/* map the file */
		mr_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &mr_size, &is_pmem);
		if (mr_ptr == NULL)
			return -1;

		/* pmem is expected */
		if (!is_pmem) {
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		/*
		 * At the beginning of the persistent memory is stored
		 * a signature which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (mr_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mr_size, SIGNATURE_LEN);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}
		data_offset = SIGNATURE_LEN;

		/*
		 * Check if the signature is in place. If it is this means
		 * the content is already persistent.
		 */
		has_content = (strncmp(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN)
				== 0);

		/*
		 * The space under the offset is intended for storing the hello
		 * structure. So the total space is assumed to be at least
		 * 1 KiB + offset of the string contents.
		 */
		if (mr_size - data_offset <
				KILOBYTE + offsetof(struct hello_t, str)) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mr_size,
					KILOBYTE + data_offset +
						offsetof(struct hello_t, str));
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		hello = (struct hello_t *)((uintptr_t)mr_ptr + data_offset);
		mr_plt = RPMA_MR_PLT_PERSISTENT;
	}
#endif

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		mr_ptr = malloc_aligned(KILOBYTE);
		if (mr_ptr == NULL)
			return -1;

		mr_size = KILOBYTE;
		hello = mr_ptr;
		mr_plt = RPMA_MR_PLT_VOLATILE;
	}

	/* initialize / translate content */
	if (has_content) {
		(void) printf("Old value: %s\n", hello->str);
		translate(hello);
	} else {
		write_hello_str(hello, en);
	}

#ifdef USE_LIBPMEM
	if (mr_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_persist(hello, sizeof(struct hello_t));

		/*
		 * If the memory region was empty (has no content) before
		 * reading content the signature has to be written to mark
		 * the new content is valid.
		 */
		if (!has_content) {
			memcpy(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			pmem_persist(mr_ptr, SIGNATURE_LEN);
		}
	}
#endif

	/* calculate data for the server read */
	struct common_data data;
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
		print_error_ex("rpma_mr_reg", ret);
		goto err_peer_delete;
	}

	/* receive the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.desc);
	if (ret)
		print_error_ex("rpma_mr_get_descriptor", ret);

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
		print_error_ex("rpma_mr_dereg", ret);

err_peer_delete:
	/* delete the peer */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error_ex("rpma_peer_delete", ret);


err_free:
#ifdef USE_LIBPMEM
	if (mr_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_unmap(mr_ptr, mr_size);
		mr_ptr = NULL;
	}
#endif

	if (mr_ptr != NULL)
		free(mr_ptr);

	return ret;
}
