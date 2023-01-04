---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_flush.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_flush** - initiate the flush operation

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_remote;
          enum rpma_flush_type {
                  RPMA_FLUSH_TYPE_PERSISTENT,
                  RPMA_FLUSH_TYPE_VISIBILITY,
          };

          int rpma_flush(struct rpma_conn *conn,
                          struct rpma_mr_remote *dst, size_t dst_offset,
                          size_t len, enum rpma_flush_type type, int flags,
                          const void *op_context);

# DESCRIPTION

**rpma_flush**() initiates the flush operation (finalizing a transfer of
data to the remote memory). Possible types of **rpma_flush**()
operation:

-   RPMA_FLUSH_TYPE_PERSISTENT - flush data down to the persistent
    domain

-   RPMA_FLUSH_TYPE_VISIBILITY - flush data deep enough to make it
    visible on the remote node

The attribute flags set the completion notification indicator:

-   RPMA_F\_COMPLETION_ON_ERROR - generate the completion on error

-   RPMA_F\_COMPLETION_ALWAYS - generate the completion regardless of
    result of the operation.

op_context is returned in the wr_id field of the completion (struct
ibv_wc).

# RETURN VALUE

The **rpma_flush**() function returns 0 on success or a negative error
code on failure.

# ERRORS

**rpma_flush**() can fail with the following errors:

-   RPMA_E\_INVAL - conn or dst is NULL

-   RPMA_E\_INVAL - unknown type value

-   RPMA_E\_INVAL - flags are not set

-   RPMA_E\_PROVIDER - **ibv_post_send**(3) failed

-   RPMA_E\_NOSUPP - type is RPMA_FLUSH_TYPE_PERSISTENT and the direct
    write to pmem is not supported

# SEE ALSO

**rpma_conn_req_connect**(3), **rpma_mr_remote_from_descriptor**(3),
**librpma**(7) and https://pmem.io/rpma/
