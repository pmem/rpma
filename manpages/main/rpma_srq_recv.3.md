---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_recv.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_srq_recv** - initiate the receive operation in shared RQ

# SYNOPSIS

          #include <librpma.h>

          struct rpma_srq;
          struct rpma_mr_local;
          int rpma_srq_recv(struct rpma_srq *srq, struct rpma_mr_local *dst, size_t offset,
                          size_t len, const void *op_context);

# DESCRIPTION

If multiple local connections use a shared RQ, **rpma_srq_recv**()
initiates the receive operation which prepares a buffer for a message
sent from other side of these connections. Please see **rpma_send**(3).

All buffers prepared via **rpma_srq_recv**(3) form an unordered set.
When a message arrives it is placed in one of the buffers awaiting and a
completion for the receive operation is generated.

A buffer for an incoming message has to be prepared beforehand.

The order of buffers in the set does not affect the order of completions
of the receive operations got via **rpma_cq_get_wc**(3).

op_context is returned in the wr_id field of the completion (struct
ibv_wc).

# NOTE

In the RDMA standard, receive requests form an ordered queue. The
librpma library does NOT inherit this guarantee.

# RETURN VALUE

The **rpma_srq_recv**() function returns 0 on success or a negative
error code on failure.

# ERRORS

**rpma_srq_recv**() can fail with the following errors:

-   RPMA_E\_INVAL - srq == NULL

-   RPMA_E\_INVAL - dst == NULL && (offset != 0 \|\| len != 0)

-   RPMA_E\_PROVIDER - **ibv_post_srq_recv**(3) failed

# SEE ALSO

**rpma_mr_reg**(3), **rpma_srq_new**(3), **librpma**(7) and
https://pmem.io/rpma/
