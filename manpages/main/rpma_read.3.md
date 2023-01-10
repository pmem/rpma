---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_read.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.2.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_read** - initiate the read operation

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_local;
          struct rpma_mr_remote;
          int rpma_read(struct rpma_conn *conn,
                          struct rpma_mr_local *dst, size_t dst_offset,
                          const struct rpma_mr_remote *src, size_t src_offset,
                          size_t len, int flags, const void *op_context);

# DESCRIPTION

**rpma_read**() initiates transferring data from the remote memory to
the local memory. To read a 0 bytes message, set src and dst to NULL and
src_offset, dst_offset and len to 0.

The attribute flags set the completion notification indicator:

-   RPMA_F\_COMPLETION_ON_ERROR - generate the completion on error

-   RPMA_F\_COMPLETION_ALWAYS - generate the completion regardless of
    result of the operation.

op_context is returned in the wr_id field of the completion (struct
ibv_wc).

# RETURN VALUE

The **rpma_read**() function returns 0 on success or a negative error
code on failure.

# ERRORS

**rpma_read**() can fail with the following errors:

-   RPMA_E\_INVAL - conn == NULL \|\| flags == 0

-   RPMA_E\_INVAL - dst == NULL && (src != NULL \|\| src_offset != 0
    \|\| dst_offset != 0 \|\| len != 0)

-   RPMA_E\_INVAL - src == NULL && (dst != NULL \|\| src_offset != 0
    \|\| dst_offset != 0 \|\| len != 0)

-   RPMA_E\_PROVIDER - **ibv_post_send**(3) failed

# SEE ALSO

**rpma_conn_req_connect**(3), **rpma_mr_reg**(3),
**rpma_mr_remote_from_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
