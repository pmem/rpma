---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.9.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_cq\_get\_completion** - receive a completion of an operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_cq;
          enum rpma_op {
                  RPMA_OP_READ,
                  RPMA_OP_WRITE,
                  RPMA_OP_FLUSH,
                  RPMA_OP_SEND,
                  RPMA_OP_RECV,
                  RPMA_OP_RECV_RDMA_WITH_IMM,
          };
          struct rpma_completion {
                  void *op_context;
                  enum rpma_op op;
                  uint32_t byte_len;
                  enum ibv_wc_status op_status;
                  unsigned flags;
                  uint32_t imm;
          };

          int rpma_cq_get_completion(struct rpma_cq *cq,
                          struct rpma_completion *cmpl);

DESCRIPTION
===========

**rpma\_cq\_get\_completion**() receives the next available completion
of an already posted operation. All operations generate completion on
error. The operations posted with the RPMA\_F\_COMPLETION\_ALWAYS flag
also generate a completion on success.

The rpma\_completion structure provides the following fields:

-   op\_context - context of the operation provided by the user to
    either **rpma\_conn\_req\_recv**(3), **rpma\_flush**(3),
    **rpma\_read**(3), **rpma\_recv**(3), **rpma\_send**(3),
    **rpma\_send\_with\_imm**(3), **rpma\_write**(3),
    **rpma\_write\_atomic**(3), **rpma\_write\_with\_imm**(3)

-   op - type of the operation, for avaiable values please see the
    decription below

-   byte\_len - number of bytes transferred

-   op\_status - status of the operation

-   flags - flags of the operation, for avaiable values please see
    **ibv\_poll\_cq**(3)

-   imm - immediate data (in host byte order)

The available op values are:

-   RPMA\_OP\_READ - RMA read operation

-   RPMA\_OP\_WRITE - RMA write operation

-   RPMA\_OP\_FLUSH - RMA flush operation

-   RPMA\_OP\_SEND - messaging send operation

-   RPMA\_OP\_RECV - messaging receive operation

-   RPMA\_OP\_RECV\_RDMA\_WITH\_IMM - messaging receive operation for
    RMA write operation with immediate data

Note that if the provided cq is the main CQ and the receive CQ is
present on the same connection this function won\'t return
RPMA\_OP\_RECV and RPMA\_OP\_RECV\_RDMA\_WITH\_IMM at any time. The
receive CQ has to be used instead to collect these completions. Please
see the **rpma\_conn\_get\_rcq**(3) for details about the receive CQ.

RETURN VALUE
============

The **rpma\_cq\_get\_completion**() function returns 0 on success or a
negative error code on failure. On success, it writes the first
available completion to cmpl. If op\_status of the written cmpl is not
equal to IBV\_WC\_SUCCESS then only op\_context of the returned cmpl is
valid.

ERRORS
======

**rpma\_cq\_get\_completion**() can fail with the following errors:

-   RPMA\_E\_INVAL - cq or cmpl is NULL

-   RPMA\_E\_NO\_COMPLETION - no completions available

-   RPMA\_E\_PROVIDER - **ibv\_poll\_cq**(3) failed with a provider
    error

-   RPMA\_E\_UNKNOWN - **ibv\_poll\_cq**(3) failed but no provider error
    is available

-   RPMA\_E\_NOSUPP - not supported opcode

SEE ALSO
========

**rpma\_conn\_get\_cq**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_conn\_req\_recv**(3), **rpma\_cq\_wait**(3),
**rpma\_cq\_get\_fd**(3), **rpma\_flush**(3), **rpma\_read**(3),
**rpma\_recv**(3), **rpma\_send**(3), **rpma\_send\_with\_imm**(3),
**rpma\_write**(3), **rpma\_write\_atomic**(3),
**rpma\_write\_with\_imm**(3), **librpma**(7) and https://pmem.io/rpma/
