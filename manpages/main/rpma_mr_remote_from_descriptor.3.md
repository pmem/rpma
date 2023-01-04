---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_remote_from_descriptor.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_remote_from_descriptor** - create a memory region from a
descriptor

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_from_descriptor(const void *desc, size_t desc_size,
                          struct rpma_mr_remote **mr_ptr);

# DESCRIPTION

Create a remote memory region\'s structure based on the provided
descriptor with a network-transferable description of the memory region
local to the remote peer.

# SECURITY WARNING

An attacker might modify the serialized remote memory registration
configuration while it is transferred via an unsecured connection (e.g.
rdma_cm private data), which might cause data corruption when writing to
a different location. Users should avoid using
**rpma_conn_get_private_data**(3) and
**rpma_conn_req_get_private_data**(3) API calls and they should utilize
TLS/SSL connections to transfer all configuration data between peers
instead.

# RETURN VALUE

The **rpma_mr_remote_from_descriptor**() function returns 0 on success
or a negative error code on failure.
**rpma_mr_remote_from_descriptor**() does not set \*mr_ptr value on
failure.

# ERRORS

**rpma_mr_remote_from_descriptor**() can fail with the following errors:

-   RPMA_E\_INVAL - desc or mr_ptr is NULL

-   RPMA_E\_INVAL - incompatible descriptor size

-   RPMA_E\_NOSUPP - deserialized information does not represent a valid
    memory region

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_mr_remote_delete**(3), **rpma_mr_remote_get_flush_type**(3),
**rpma_mr_remote_get_size**(3), **rpma_flush**(3), **rpma_read**(3),
**rpma_write**(3), **rpma_atomic_write**(3), **librpma**(7) and
https://pmem.io/rpma/
