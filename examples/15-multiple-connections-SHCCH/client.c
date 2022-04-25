// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * client.c -- a client of the multiple-connections example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "common-conn.h"
#include "multiple-connections-common.h"

static char *Names[] = {
	"Andy",
	"Chet",
	"Derek",
	"Janek",
	"Lukasz",
	"Oksana",
	"Pawel",
	"Piotr",
	"Tomasz",
	"Xiang",
	"Xiaoran",
	"Xiaoyan"
};

#define NAMES_NUM (sizeof(Names) / sizeof(Names[0]))

#define USAGE_STR "usage: %s <server_address> <port> <seed>\n"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 4) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	long unsigned seed = strtoul(argv[3], NULL, 10);
	if (seed == ULONG_MAX && errno == ERANGE) {
		(void) fprintf(stderr, "strtoul(seed) overflowed\n");
		return -1;
	}

	int ret;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/* resources - memory region */
	void *mr_ptr = NULL;
	struct rpma_mr_local *mr = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		return ret;

	/* allocate a memory */
	mr_ptr = malloc_aligned(MAX_NAME_SIZE);
	if (mr_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}

	/* pick a name */
	srand(seed % UINT_MAX);
	const char *name = Names[(long unsigned int)rand() % NAMES_NUM];
	(void) strncpy((char *)mr_ptr, name, (MAX_NAME_SIZE - 1));
	((char *)mr_ptr)[MAX_NAME_SIZE - 1] = '\0';
	printf("My names is: %s\n", (char *)mr_ptr);

	/* register the memory */
	ret = rpma_mr_reg(peer, mr_ptr, MAX_NAME_SIZE, RPMA_MR_USAGE_READ_SRC,
				&mr);
	if (ret)
		goto err_mr_free;

	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(mr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	struct common_data data = {0};
	data.mr_desc_size = mr_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.descriptors[0]);
	if (ret)
		goto err_mr_dereg;

	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(peer, addr, port, NULL, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	ret = common_wait_for_conn_close_and_disconnect(&conn);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_mr_free:
	/* free the memory */
	free(mr_ptr);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

	return ret;
}
