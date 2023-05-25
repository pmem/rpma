---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_reg.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_reg** - create a local memory registration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_mr_local;

          int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
                  struct rpma_mr_local **mr_ptr);

# DESCRIPTION

**rpma_mr_reg**() registers a memory region and creates a local memory
registration object. The usage parameter specifies the operations that
can be performed on the given memory region which should be expressed as
bitwise-inclusive OR of the following:

-   RPMA_MR_USAGE_READ_SRC - memory used as a source of the read
    operation

-   RPMA_MR_USAGE_READ_DST - memory used as a destination of the read
    operation

-   RPMA_MR_USAGE_WRITE_SRC - memory used as a source of the write
    operation

-   RPMA_MR_USAGE_WRITE_DST - memory used as a destination of the write
    operation

-   RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY - memory with available flush
    operation

-   RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT - memory with available
    persistent flush operation

-   RPMA_MR_USAGE_SEND - memory used for send operation

-   RPMA_MR_USAGE_RECV - memory used for receive operation

# RETURN VALUE

The **rpma_mr_reg**() function returns 0 on success or a negative error
code on failure. **rpma_mr_reg**() does not set \*mr_ptr value on
failure.

# ERRORS

**rpma_mr_reg**() can fail with the following errors:

-   RPMA_E\_INVAL - peer or ptr or mr_ptr is NULL

-   RPMA_E\_INVAL - size equals 0

-   RPMA_E\_NOMEM - out of memory

-   RPMA_E\_PROVIDER - memory registration failed

# SEE ALSO

**rpma_conn_req_recv**(3), **rpma_mr_dereg**(3),
**rpma_mr_get_descriptor**(3), **rpma_mr_get_descriptor_size**(3),
**rpma_peer_new**(3), **rpma_read**(3), **rpma_recv**(3),
**rpma_send**(3), **rpma_write**(3), **rpma_atomic_write**(3),
**librpma**(7) and https://pmem.io/rpma/
