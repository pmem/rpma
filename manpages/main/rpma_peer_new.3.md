---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

# NAME

**rpma_peer_new** - create a peer object

# SYNOPSIS

          #include <librpma.h>

          struct ibv_context;
          struct rpma_peer;
          int rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer_ptr);

# DESCRIPTION

**rpma_peer_new**() creates a new peer object.

# RETURN VALUE

The **rpma_peer_new**() function returns 0 on success or a negative
error code on failure. **rpma_peer_new**() does not set \*peer_ptr value
on failure.

# ERRORS

**rpma_peer_new**() can fail with the following errors:

-   RPMA_E\_INVAL - ibv_ctx or peer_ptr is NULL

-   RPMA_E\_NOMEM - creating a verbs protection domain failed with
    ENOMEM.

-   RPMA_E\_PROVIDER - creating a verbs protection domain failed with
    error other than ENOMEM.

-   RPMA_E\_UNKNOWN - creating a verbs protection domain failed without
    error value.

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_conn_req_new**(3), **rpma_ep_listen**(3), **rpma_mr_reg**(3),
**rpma_peer_delete**(3), **rpma_utils_get_ibv_context**(3),
**librpma**(7) and https://pmem.io/rpma/
