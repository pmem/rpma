# THREAD SAFETY

This document describes the analysis of thread safety of the librpma library ... XXX

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_conn_cfg_new
- rpma_conn_cfg_delete

## Conditionally thread-safe API calls

The following API calls of the librpma library:
- rpma_conn_cfg_get_compl_channel
- rpma_conn_cfg_get_cq_size
- rpma_conn_cfg_get_rcq_size
- rpma_conn_cfg_get_rq_size
- rpma_conn_cfg_get_sq_size
- rpma_conn_cfg_get_timeout
- rpma_conn_cfg_set_compl_channel
- rpma_conn_cfg_set_cq_size
- rpma_conn_cfg_set_rcq_size
- rpma_conn_cfg_set_rq_size
- rpma_conn_cfg_set_sq_size
- rpma_conn_cfg_set_timeout
- rpma_conn_req_new (calls rpma_conn_cfg_get_*() APIs)

are thread-safe only if each thread operates on a **separate configuration structure** (`struct rpma_conn_cfg`) used only by this one thread. They are not thread-safe if threads operate on one configuration structure common for more than one thread.

The following API calls of the librpma library:
- rpma_conn_req_connect
- rpma_conn_req_delete
- rpma_conn_req_get_private_data
- rpma_conn_req_recv

are thread-safe only if each thread operates on a **separate connection request** (`struct rpma_conn_req`) used only by this one thread. They are not thread-safe if threads operate on one connection request common for more than one thread.

## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:
- XXX

## Relationship of libibverbs and librdmacm

XXX

## Not thread-safe scenarios

XXX

## Analysis of Valgrind suppressions

### Suppressions for libibverbs

The suppressions for libibverbs are described in the [tests/memcheck-libibverbs.supp](tests/memcheck-libibverbs.supp) file.

### Suppressions for the drd tool

The suppressions for the drd tool are described in the [tests/drd.supp](tests/drd.supp) file.

### Suppressions for the helgrind tool

The suppressions for the helgrind tool are described in the [tests/helgrind.supp](tests/helgrind.supp) file.
