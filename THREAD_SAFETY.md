# THREAD SAFETY

This document presents the analysis of thread safety of the librpma library.

**The main assumptions** this analysis is based on are following:
1) many threads may use the same peer (`struct rpma_peer`) to create separate connections (`struct rpma_conn`) for each of threads, but
2) each of the endpoints (`struct rpma_ep`) can be used by only one thread at the same time and
3) each of the connections (`struct rpma_conn`) can be used by only one thread at the same time,

so **the most common scenarios** are following:
1) on the active side: each thread creates and uses a separate connection (`struct rpma_conn`),
2) on the passive side: the main thread establishes the connection but the rest of work (including connection shutdown) is done by a separate thread.

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_ep_get_fd;
- rpma_ep_listen;
- rpma_ep_next_conn_req;
- rpma_ep_shutdown;

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
