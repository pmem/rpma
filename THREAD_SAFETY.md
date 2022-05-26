# THREAD SAFETY

This document describes the analysis of thread safety of the librpma library ... XXX

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_utils_ibv_context_is_odp_capable
- rpma_utils_conn_event_2str

## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:
- rpma_utils_get_ibv_context

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
