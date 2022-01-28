// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * server.c -- a server of the flush-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR \
	"usage: %s <server_address> <port> [<pmem-path>] [direct-pmem-write]\n"\
	PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM */

#ifdef USE_LIBPMEM
#define ON_STR "on"
#endif /* USE_LIBPMEM */

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
	struct rpma_mr_local *mr = NULL;

	int is_pmem = 0;

#ifdef USE_LIBPMEM
	char *pmem_path = NULL;
	if (argc >= 4) {
		pmem_path = argv[3];

		/* map the file */
		mr_ptr = pmem_map_file(pmem_path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &mr_size, &is_pmem);
		if (mr_ptr == NULL) {
			(void) fprintf(stderr, "pmem_map_file() for %s "
					"failed\n", pmem_path);
			return -1;
		}

		/* pmem is expected */
		if (!is_pmem) {
			(void) fprintf(stderr, "%s is not an actual PMEM\n",
				pmem_path);
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
					pmem_path, mr_size, SIGNATURE_LEN);
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
				pmem_path, mr_size, KILOBYTE + data_offset);
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
	}
#endif /* USE_LIBPMEM */

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mr_ptr = malloc_aligned(KILOBYTE);
		if (mr_ptr == NULL)
			return -1;

		mr_size = KILOBYTE;
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
			RPMA_MR_USAGE_WRITE_DST |
			(is_pmem ? (RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT |
				RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY) :
				RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY),
			&mr);
	if (ret)
		goto err_ep_shutdown;

#ifdef USE_LIBPMEM
	/* rpma_mr_advise() should be called only in case of FsDAX */
	if (is_pmem && strstr(pmem_path, "/dev/dax") == NULL) {
		ret = rpma_mr_advise(mr, 0, mr_size,
			IBV_ADVISE_MR_ADVICE_PREFETCH_WRITE,
			IBV_ADVISE_MR_FLAG_FLUSH);
		if (ret)
			goto err_mr_dereg;
	}
#endif /* USE_LIBPMEM */

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
	struct common_data data = {0};
	data.data_offset = data_offset;
	data.mr_desc_size = mr_desc_size;
	data.pcfg_desc_size = pcfg_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.descriptors[0]);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Get the peer's configuration descriptor.
	 * The pcfg_desc descriptor is saved in the `descriptors[]` array
	 * just after the mr_desc descriptor.
	 */
	ret = rpma_peer_cfg_get_descriptor(pcfg,
			&data.descriptors[mr_desc_size]);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);
	ret = server_accept_connection(ep, NULL, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	ret = common_wait_for_conn_close_and_disconnect(&conn);
	if (ret)
		goto err_mr_dereg;

	(void) printf("New value: %s\n", (char *)mr_ptr + data_offset);

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
	if (is_pmem) {
		pmem_unmap(mr_ptr, mr_size);
		mr_ptr = NULL;
	}
#endif /* USE_LIBPMEM */

	if (mr_ptr != NULL)
		free(mr_ptr);

	return ret;
}
