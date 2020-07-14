// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * common.c -- a common functions used by examples
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef USE_LIBPMEM
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "common.h"

/*
 * print_error_ex -- print RPMA error to stderr
 *
 * The name is "print_error_ex" to distinguish
 * from the cmocka's "print_error" function.
 */
void
print_error_ex(const char *fname, int ret)
{
	if (ret == RPMA_E_PROVIDER) {
		int errnum = rpma_err_get_provider_error();
		const char *errstr = strerror(errnum);
		(void) fprintf(stderr, "%s failed: %s (%s)\n", fname,
				rpma_err_2str(ret), errstr);
	} else {
		(void) fprintf(stderr, "%s failed: %s\n", fname,
				rpma_err_2str(ret));
	}
}

/*
 * malloc_aligned -- allocate an aligned chunk of memory
 */
void *
malloc_aligned(size_t size)
{
	long pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		perror("sysconf");
		return NULL;
	}

	/* allocate a page size aligned local memory pool */
	void *mem;
	int ret = posix_memalign(&mem, (size_t)pagesize, size);
	if (ret) {
		(void) fprintf(stderr, "posix_memalign: %s\n", strerror(ret));
		return NULL;
	}

	return mem;
}
/*
 * common_peer_via_address -- create a new RPMA peer based on ibv_context
 * received by the provided address
 */
int
common_peer_via_address(const char *addr, enum rpma_util_ibv_context_type type,
		struct rpma_peer **peer_ptr)
{
	struct ibv_context *dev = NULL;

	int ret = rpma_utils_get_ibv_context(addr, type, &dev);
	if (ret) {
		print_error_ex("rpma_utils_get_ibv_context", ret);
		return -1;
	}

	/* create a new peer object */
	ret = rpma_peer_new(dev, peer_ptr);
	if (ret) {
		print_error_ex("rpma_peer_new", ret);
		return -1;
	}

	return 0;
}

/*
 * client_connect -- establish a new connection to a server listening at
 * addr:service
 */
int
client_connect(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* create a connection request */
	int ret = rpma_conn_req_new(peer, addr, service, &req);
	if (ret) {
		print_error_ex("rpma_conn_req_new", ret);
		return -1;
	}

	/* connect the connection request and obtain the connection object */
	ret = rpma_conn_req_connect(&req, pdata, conn_ptr);
	if (ret) {
		print_error_ex("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return -1;
	}

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret) {
		print_error_ex("rpma_conn_next_event", ret);
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
				"rpma_conn_next_event returned an unexptected event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * server_listen -- start a listening endpoint at addr:service
 */
int
server_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep_ptr)
{
	/* create a new endpoint object */
	int ret = rpma_ep_listen(peer, addr, service, ep_ptr);
	if (ret) {
		print_error_ex("rpma_ep_listen", ret);
		return ret;
	}
	fprintf(stdout, "Waiting for incoming connections...\n");

	return 0;
}

/*
 * server_accept_connection -- wait for an incoming connection request,
 * accept it and wait for its establishment
 */
int
server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* receive an incoming connection request */
	int ret = rpma_ep_next_conn_req(ep, &req);
	if (ret) {
		print_error_ex("rpma_ep_next_conn_req", ret);
		return ret;
	}

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	ret = rpma_conn_req_connect(&req, pdata, conn_ptr);
	if (ret) {
		print_error_ex("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return ret;
	}

	/* wait for the connection to be established */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret) {
		print_error_ex("rpma_conn_next_event", ret);
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
				"rpma_conn_next_event returned an unexptected event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		ret = -1;
	} else {
		fprintf(stdout, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	if (ret)
		(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * common_disconnect_verbose -- call rpma_conn_disconnect() and print an error
 * message on error
 */
static inline int
common_disconnect_verbose(struct rpma_conn *conn)
{
	/* disconnect the connection */
	int ret = rpma_conn_disconnect(conn);
	if (ret)
		print_error_ex("rpma_conn_disconnect", ret);

	return ret;
}

/*
 * common_wait_for_conn_close_verbose -- wait for RPMA_CONN_CLOSED and print
 * an error message on error
 */
static inline int
common_wait_for_conn_close_verbose(struct rpma_conn *conn)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* wait for the connection to be closed */
	int ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		print_error_ex("rpma_conn_next_event", ret);
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr,
				"rpma_conn_next_event returned an unexptected event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	} else {
		fprintf(stderr,
				"rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	return ret;
}

/*
 * common_conn_delete_verbose -- call rpma_conn_delete() and print an error
 * message on error
 */
static inline int
common_conn_delete_verbose(struct rpma_conn **conn_ptr)
{
	/* delete the connection object */
	int ret = rpma_conn_delete(conn_ptr);
	if (ret)
		print_error_ex("rpma_conn_delete", ret);

	return ret;
}

/*
 * common_wait_for_conn_close_and_disconnect -- wait for RPMA_CONN_CLOSED,
 * disconnect and delete the connection structure
 */
int
common_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr)
{
	int ret = 0;
	ret |= common_wait_for_conn_close_verbose(*conn_ptr);
	ret |= common_disconnect_verbose(*conn_ptr);
	ret |= common_conn_delete_verbose(conn_ptr);

	return ret;
}

/*
 * common_disconnect_and_wait_for_conn_close -- disconnect, wait for
 * RPMA_CONN_CLOSED and delete the connection structure
 */
int
common_disconnect_and_wait_for_conn_close(struct rpma_conn **conn_ptr)
{
	int ret = 0;

	ret |= common_disconnect_verbose(*conn_ptr);
	if (ret == 0)
		ret |= common_wait_for_conn_close_verbose(*conn_ptr);

	ret |= common_conn_delete_verbose(conn_ptr);

	return ret;
}

#ifdef USE_LIBPMEM

/* signature marking the persistent contents as valid */
#define SIGNATURE_STR "RPMA_EXAMPLE_SIG"
#define SIGNATURE_LEN (strlen(SIGNATURE_STR) + 1)

/*
 * common_pmem_map -- prepare pmem using libpmem API
 */
int
common_pmem_map(const char *path, void **ptr, size_t *mapped_len,
		size_t *data_offset, int *has_contents)
{
	int is_pmem = 0;

	/* get the root address */
	*ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
			0 /* mode */, mapped_len, &is_pmem);
	if (ptr == NULL)
		return -1;

	/* pmem is expected */
	if (!is_pmem) {
		(void) pmem_unmap(*ptr, *mapped_len);
		return -1;
	}

	/*
	 * At the beginning of the persistent memory is stored a signature which
	 * marks its contents as valid. So the length of the mapped memory has
	 * to be at least of the length of the signature to convey any
	 * meaningful contents and be usable as a persistent store.
	 */
	if (*mapped_len < SIGNATURE_LEN) {
		(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
				path, *mapped_len, SIGNATURE_LEN);
		(void) pmem_unmap(*ptr, *mapped_len);
		return -1;
	}

	*has_contents = (strncmp(*ptr, SIGNATURE_STR, SIGNATURE_LEN) == 0);
	*data_offset = SIGNATURE_LEN;

	return 0;
}

/*
 * common_write_signature -- write SIGNATURE_STR at the beginning of the pmem
 * mapping aka marking the pmem contents as valid
 */
void
common_write_signature(char *ptr)
{
	memcpy(ptr, SIGNATURE_STR, SIGNATURE_LEN);
	pmem_persist(ptr, SIGNATURE_LEN);
}

#endif
