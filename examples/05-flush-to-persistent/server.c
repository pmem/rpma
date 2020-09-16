// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the flush-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <librpma_log.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>] " \
	"[direct-pmem-write]\n"
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM */

#ifdef USE_LIBPMEM
#define ON_STR "on"
#endif /* USE_LIBPMEM */

#include "common-conn.h"

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
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	size_t data_offset = 0;
	enum rpma_mr_plt mr_plt = RPMA_MR_PLT_VOLATILE;
	struct rpma_mr_local *mr = NULL;

#ifdef USE_LIBPMEM
	if (argc >= 4) {
		char *path = argv[3];
		int is_pmem;

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
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
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
		 * All of the space under the offset is intended for
		 * the string contents. Space is assumed to be at least 1 KiB.
		 */
		if (mr_size - data_offset < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mr_size, KILOBYTE + data_offset);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial empty string and persist it */
			char *ch = (char *)mr_ptr + data_offset;
			ch[0] = '\0';
			pmem_persist(ch, 1);
			/* write the signature to mark the content as valid */
			memcpy(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			pmem_persist(mr_ptr, SIGNATURE_LEN);
		}

		mr_plt = RPMA_MR_PLT_PERSISTENT;
	}
#endif /* USE_LIBPMEM */

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		mr_ptr = malloc_aligned(KILOBYTE);
		if (mr_ptr == NULL)
			return -1;

		mr_size = KILOBYTE;
		mr_plt = RPMA_MR_PLT_VOLATILE;
	}

	/* RPMA resources */
	struct rpma_peer_cfg *pcfg = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;

	/* if the string content is not empty */
	if (((char *)mr_ptr + data_offset)[0] != '\0') {
		(void) printf("Old value: %s\n", (char *)mr_ptr + data_offset);
	}

	/* create a peer configuration structure */
	ret = rpma_peer_cfg_new(&pcfg);
	if (ret)
		goto err_free;

#ifdef USE_LIBPMEM
	/* configure peer's direct write to pmem support */
	if (argc == 5) {
		ret = rpma_peer_cfg_set_direct_write_to_pmem(pcfg,
				(strcmp(argv[4], ON_STR) == 0));
		if (ret) {
			(void) rpma_peer_cfg_delete(&pcfg);
			goto err_free;
		}
	}
#endif /* USE_LIBPMEM */

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_pcfg_delete;

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(peer, addr, port, &ep);
	if (ret)
		goto err_peer_delete;

	/* register the memory */
	ret = rpma_mr_reg(peer, mr_ptr, mr_size,
			RPMA_MR_USAGE_WRITE_DST | RPMA_MR_USAGE_FLUSHABLE,
			mr_plt, &mr);
	if (ret)
		goto err_ep_shutdown;

	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(mr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	/* get size of the peer config descriptor */
	size_t pcfg_desc_size;
	ret = rpma_peer_cfg_get_descriptor_size(pcfg, &pcfg_desc_size);
	if (ret)
		goto err_mr_dereg;

	/* calculate data for the client write */
	size_t data_size = sizeof(struct common_data)
				+ mr_desc_size + pcfg_desc_size;
	struct common_data *data = malloc(data_size);
	if (data == NULL)
		goto err_mr_dereg;

	data->data_offset = data_offset;
	data->mr_desc_offset = 0;		/* the first is mr_desc */
	data->mr_desc_size = mr_desc_size;
	data->pcfg_desc_offset = mr_desc_size;	/* the second is pcfg_desc */
	data->pcfg_desc_size = pcfg_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr,
			&data->descriptors[data->mr_desc_offset]);
	if (ret)
		goto err_free_data;

	/* get the peer's configuration descriptor */
	ret = rpma_peer_cfg_get_descriptor(pcfg,
			&data->descriptors[data->pcfg_desc_offset]);
	if (ret)
		goto err_free_data;

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = data;
	pdata.len = data_size;
	ret = server_accept_connection(ep, &pdata, &conn);
	if (ret)
		goto err_free_data;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	ret = common_wait_for_conn_close_and_disconnect(&conn);
	if (ret)
		goto err_free_data;

	(void) printf("New value: %s\n", (char *)mr_ptr + data_offset);

err_free_data:
	free(data);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_ep_shutdown:
	/* shutdown the endpoint */
	(void) rpma_ep_shutdown(&ep);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

err_pcfg_delete:
	(void) rpma_peer_cfg_delete(&pcfg);

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
