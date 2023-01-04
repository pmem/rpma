---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_send.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_send** - initiate the send operation

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_local;
          int rpma_send(struct rpma_conn *conn, const struct rpma_mr_local *src, size_t offset,
                          size_t len, int flags, const void *op_context);

# DESCRIPTION

**rpma_send**() initiates the send operation which transfers a message
from the local memory to other side of the connection. To send a 0 byte
message, set src to NULL and both offset and len to 0.

The attribute flags set the completion notification indicator:

-   RPMA_F\_COMPLETION_ON_ERROR - generate the completion on error

-   RPMA_F\_COMPLETION_ALWAYS - generate the completion regardless of
    result of the operation.

op_context is returned in the wr_id field of the completion (struct
ibv_wc).

# RETURN VALUE

The **rpma_send**() function returns 0 on success or a negative error
code on failure.

# ERRORS

**rpma_send**() can fail with the following errors:

-   RPMA_E\_INVAL - conn == NULL \|\| flags == 0

-   RPMA_E\_INVAL - src == NULL && (offset != 0 \|\| len != 0)

-   RPMA_E\_PROVIDER - **ibv_post_send**(3) failed

# SEE ALSO

**rpma_conn_req_connect**(3), **rpma_mr_reg**(3), **librpma**(7) and
https://pmem.io/rpma/
