---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_req_recv.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_req_recv** - initiate the receive operation

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_req;
          struct rpma_mr_local;
          int rpma_conn_req_recv(struct rpma_conn_req *req, struct rpma_mr_local *dst, size_t offset,
                          size_t len, const void *op_context);

# DESCRIPTION

**rpma_conn_req_recv**() initiates the receive operation. It prepares a
buffer for a message sent from other side of the connection. Please see
**rpma_send**(3). This is a variant of **rpma_recv**(3) which may be
used before the connection is established. op_context is returned in the
wr_id field of the completion (struct ibv_wc).

# RETURN VALUE

The **rpma_conn_req_recv**() function returns 0 on success or a negative
error code on failure.

# ERRORS

**rpma_conn_req_recv**() can fail with the following errors:

-   RPMA_E\_INVAL - req or src or op_context is NULL

-   RPMA_E\_PROVIDER - **ibv_post_recv**(3) failed

# SEE ALSO

**rpma_conn_req_new**(3), **rpma_mr_reg**(3), **librpma**(7) and
https://pmem.io/rpma/
