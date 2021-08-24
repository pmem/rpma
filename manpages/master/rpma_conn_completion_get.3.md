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

**rpma\_conn\_completion\_get** - receive a completion of an operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_completion;
          enum rpma_op {
                  RPMA_OP_READ,
                  RPMA_OP_WRITE,
                  RPMA_OP_FLUSH,
                  RPMA_OP_SEND,
                  RPMA_OP_RECV,
                  RPMA_OP_RECV_RDMA_WITH_IMM,
          };

          int rpma_conn_completion_get(struct rpma_conn *conn,
                          struct rpma_completion *cmpl);

DESCRIPTION
===========

**rpma\_conn\_completion\_get**() receives the next available completion
of an already posted operation. All operations generate completion on
error. The operations posted with the RPMA\_F\_COMPLETION\_ALWAYS flag
also generate a completion on success. The following operations are
available:

-   RPMA\_OP\_READ - RMA read operation

-   RPMA\_OP\_WRITE - RMA write operation

-   RPMA\_OP\_FLUSH - RMA flush operation

-   RPMA\_OP\_SEND - messaging send operation

-   RPMA\_OP\_RECV - messaging receive operation

-   RPMA\_OP\_RECV\_RDMA\_WITH\_IMM - messaging receive operation for
    RMA write operation with immediate data

RETURN VALUE
============

The **rpma\_conn\_completion\_get**() function returns 0 on success or a
negative error code on failure. On success, it writes the first
available completion to cmpl. If op\_status of the written cmpl is not
equal to IBV\_WC\_SUCCESS then only op\_context of the returned cmpl is
valid.

ERRORS
======

**rpma\_conn\_completion\_get**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or cmpl is NULL

-   RPMA\_E\_NO\_COMPLETION - no completions available

-   RPMA\_E\_PROVIDER - **ibv\_poll\_cq**(3) failed with a provider
    error

-   RPMA\_E\_UNKNOWN - **ibv\_poll\_cq**(3) failed but no provider error
    is available

-   RPMA\_E\_NOSUPP - not supported opcode

SEE ALSO
========

**rpma\_conn\_get\_completion\_fd**(3),
**rpma\_conn\_completion\_wait**(3), **rpma\_conn\_req\_connect**(3),
**rpma\_flush**(3), **rpma\_read**(3), **rpma\_recv**(3),
**rpma\_send**(3), **rpma\_write**(3), **rpma\_write\_atomic**(3),
**librpma**(7) and https://pmem.io/rpma/
