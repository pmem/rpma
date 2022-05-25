# THREAD SAFETY

This document describes the analysis of thread safety of the librpma library ... XXX

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- struct rpma_peer;
- rpma_peer_new()
- rpma_peer_delete()

- rpma_peer_cfg_new()
- rpma_peer_cfg_delete()
- rpma_peer_cfg_from_descriptor()


## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:
- struct rpma_peer_cfg;
- rpma_peer_cfg_set_direct_write_to_pmem()
- rpma_peer_cfg_get_direct_write_to_pmem()
- rpma_peer_cfg_get_descriptor()
- rpma_peer_cfg_get_descriptor_size()

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
