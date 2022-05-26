# THREAD SAFETY

This document presents the analysis of thread safety of the librpma library.

**The main assumptions** this analysis is based on are following:
1) many threads may use the same peer (`struct rpma_peer`) to create separate connections (`struct rpma_conn`) for each of threads, but
2) each of the connections (`struct rpma_conn`) can be used by only one thread at the same time,

so **the most common scenarios** are following:
1) on the active side: each thread creates and uses a separate connection (`struct rpma_conn`),
2) on the passive side: the main thread establishes the connection but the rest of work (including connection shutdown) is done by a separate thread.

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_conn_delete
- rpma_conn_disconnect
- rpma_conn_get_cq
- rpma_conn_get_compl_fd
- rpma_conn_get_event_fd
- rpma_conn_get_private_data
- rpma_conn_get_qp_num
- rpma_conn_get_rcq
- rpma_conn_next_event
- rpma_conn_wait

## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:
- rpma_conn_apply_remote_peer_cfg - calls rpma_peer_cfg_get_direct_write_to_pmem

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
