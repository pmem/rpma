---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_utils_get_ibv_context.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_utils_get_ibv_context** - obtain an RDMA device context by IP
address

# SYNOPSIS

          #include <librpma.h>

          struct ibv_context;
          enum rpma_util_ibv_context_type {
                  RPMA_UTIL_IBV_CONTEXT_LOCAL,
                  RPMA_UTIL_IBV_CONTEXT_REMOTE
          };

          int rpma_utils_get_ibv_context(const char *addr, enum rpma_util_ibv_context_type type,
                  struct ibv_context **ibv_ctx_ptr);

# DESCRIPTION

**rpma_utils_get_ibv_context**() obtains an RDMA device context by the
given IPv4/IPv6 address (either local or remote) using the TCP RDMA port
space (RDMA_PS_TCP) - reliable, connection-oriented and message-based QP
communication. Possible values of the \'type\' argument:

-   RPMA_UTIL_IBV_CONTEXT_LOCAL - lookup for a device based on the given
    local address

-   RPMA_UTIL_IBV_CONTEXT_REMOTE - lookup for a device based on the
    given remote address

# RETURN VALUE

The **rpma_utils_get_ibv_context**() function returns 0 on success or a
negative error code on failure. **rpma_utils_get_ibv_context**() does
not set \*ibv_ctx_ptr value on failure.

# ERRORS

**rpma_utils_get_ibv_context**() can fail with the following errors:

-   RPMA_E\_INVAL - addr or ibv_ctx_ptr is NULL or type is unknown

-   RPMA_E\_NOMEM - out of memory

-   RPMA_E\_PROVIDER - **rdma_getaddrinfo**(), **rdma_create_id**(),
    **rdma_bind_addr**() or **rdma_resolve_addr**() failed, the exact
    cause of the error can be read from the log

# SEE ALSO

**rpma_peer_new**(3), **rpma_utils_ibv_context_is_odp_capable**(3),
**librpma**(7) and https://pmem.io/rpma/
