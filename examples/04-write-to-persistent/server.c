// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the write-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR "usage: %s <server_address> <service> [<pmem-path>]\n"
#else
#define USAGE_STR "usage: %s <server_address> <service>\n"
#endif /* USE_LIBPMEM */

#include "common.h"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* read common parameters */
	char *addr = argv[1];
	char *service = argv[2];
	int ret;

	/* resources - memory region */
	void *dst_ptr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	enum rpma_mr_plt dst_plt = RPMA_MR_PLT_VOLATILE;
	struct rpma_mr_local *mr = NULL;

#ifdef USE_LIBPMEM
	if (argc >= 4) {
		char *path = argv[3];
		int is_pmem;

		/* map the file */
		dst_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &dst_size, &is_pmem);
		if (dst_ptr == NULL)
			return -1;

		/* pmem is expected */
		if (!is_pmem) {
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (dst_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, dst_size, SIGNATURE_LEN);
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}
		dst_offset = SIGNATURE_LEN;

		/*
		 * All of the space under the offset is intended for
		 * the string contents. Space is assumed to be at least 1 KiB.
		 */
		if (dst_size - dst_offset < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, dst_size, KILOBYTE + dst_offset);
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(dst_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial empty string and persist it */
			((char *)dst_ptr + dst_offset)[0] = '\0';
			pmem_persist(dst_ptr, 1);
			/* write the signature to mark the content as valid */
			memcpy(dst_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			pmem_persist(dst_ptr, SIGNATURE_LEN);
		}

		dst_plt = RPMA_MR_PLT_PERSISTENT;
	}
#endif

	/* if no pmem support or it is not provided */
	if (dst_ptr == NULL) {
		dst_ptr = malloc_aligned(KILOBYTE);
		if (dst_ptr == NULL)
			return -1;

		dst_size = KILOBYTE;
		dst_plt = RPMA_MR_PLT_VOLATILE;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_free;

	/* start a listening endpoint at addr:service */
	ret = server_listen(peer, addr, service, &ep);
	if (ret)
		goto err_peer_delete;

	/* register the memory */
	ret = rpma_mr_reg(peer, dst_ptr, dst_size,
			RPMA_MR_USAGE_WRITE_DST | RPMA_MR_USAGE_READ_SRC,
			dst_plt, &mr);
	if (ret) {
		print_error_ex("rpma_mr_reg", ret);
		goto err_ep_shutdown;
	}

	struct rpma_conn_private_data pdata;
	rpma_mr_descriptor desc;
	pdata.ptr = &desc;
	pdata.len = sizeof(rpma_mr_descriptor);

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &desc);
	if (ret)
		print_error_ex("rpma_mr_get_descriptor", ret);

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = server_accept_connection(ep, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/* if the string content is not empty */
	if (((char *)dst_ptr + dst_offset)[0] != '\0') {
		(void) printf("Old value: %s\n", (char *)dst_ptr + dst_offset);
	}

#ifdef USE_LIBPMEM
	if (dst_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_persist((char *)dst_ptr + dst_offset, KILOBYTE);
	}
#endif

	(void) printf("New value: %s\n", (char *)dst_ptr + dst_offset);

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	ret = common_wait_for_conn_close_and_disconnect(&conn);
	if (ret)
		goto err_mr_dereg;

err_mr_dereg:
	/* deregister the memory region */
	ret = rpma_mr_dereg(&mr);
	if (ret)
		print_error_ex("rpma_mr_dereg", ret);

err_ep_shutdown:
	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret)
		print_error_ex("rpma_ep_shutdown", ret);

err_peer_delete:
	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error_ex("rpma_peer_delete", ret);

err_free:
#ifdef USE_LIBPMEM
	if (dst_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_unmap(dst_ptr, dst_size);
		dst_ptr = NULL;
	}
#endif

	if (dst_ptr != NULL)
		free(dst_ptr);

	return ret;
}
