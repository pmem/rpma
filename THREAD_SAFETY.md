# THREAD SAFETY

This document presents the analysis of thread safety of the librpma library.

**The main assumptions** this analysis is based on are following:
1) many threads may use the same peer (`struct rpma_peer`) to create separate connections (`struct rpma_conn`) for each of threads, but
2) each of the connections (`struct rpma_conn`) can be used by only one thread at the same time,

so **the most common scenarios** are following:
1) on the active side: each thread creates and uses a separate connection (`struct rpma_conn`),
2) on the passive side: the main thread establishes the connection but the rest of work (including connection shutdown) is done by a separate thread.

Most of the core librpma API calls are thread-safe but there are also very important exceptions (described below) mainly related to connection's configuration, establishment and tear-down.

Creating resources of RPMA library usually involves dynamic memory allocation and destroying resources usually involves a dynamic memory release. The same resource cannot be destroyed more than once at any thread and a resource cannot be used after it was destroyed. It is the user's responsibility to follow those rules and not doing so may result in a segmentation fault or an undefined behaviour.

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_peer_new
- rpma_peer_delete
- rpma_peer_cfg_new
- rpma_peer_cfg_delete
- rpma_peer_cfg_from_descriptor
- rpma_peer_cfg_get_descriptor_size

## Conditionally thread-safe API calls

The following API calls of the librpma library:
- rpma_peer_cfg_set_direct_write_to_pmem
- rpma_peer_cfg_get_direct_write_to_pmem
- rpma_peer_cfg_get_descriptor

are thread-safe only if each thread operates on a **separate configuration structure** (`struct rpma_peer_cfg`) used only by this one thread. They are not thread-safe if threads operate on one configuration structure common for more than one thread.

## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:

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
