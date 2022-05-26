# THREAD SAFETY

This document describes the analysis of thread safety of the librpma library ... XXX

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_mr_reg()
- rpma_mr_dereg()
- rpma_mr_get_descriptor()
- rpma_mr_remote_from_descriptor()
- rpma_mr_get_descriptor_size()
- rpma_mr_get_ptr()
- rpma_mr_get_size()
- rpma_mr_remote_get_size()
- rpma_mr_remote_delete()
- rpma_mr_remote_get_flush_type()
- rpma_mr_advise()

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
