---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_ep_listen.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.12.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_ep\_listen** - create a listening endpoint

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_ep;
          int rpma_ep_listen(struct rpma_peer *peer, const char *addr,
                          const char *port, struct rpma_ep **ep_ptr);

DESCRIPTION
===========

**rpma\_ep\_listen**() creates an endpoint and initiates listening for
incoming connections using reliable, connection-oriented and
message-based (RDMA\_PS\_TCP) QP communication.

RETURN VALUE
============

The **rpma\_ep\_listen**() function returns 0 on success or a negative
error code on failure. **rpma\_ep\_listen**() does not set \*ep\_ptr
value on failure.

ERRORS
======

**rpma\_ep\_listen**() can fail with the following errors:

-   RPMA\_E\_INVAL - peer, addr, port or ep\_ptr is NULL

-   RPMA\_E\_PROVIDER - **rdma\_create\_event\_channel**(3),
    **rdma\_create\_id**(3), **rdma\_getaddrinfo**(3),
    **rdma\_listen**(3) failed

-   RPMA\_E\_NOMEM - out of memory

SEE ALSO
========

**rpma\_ep\_get\_fd**(3), **rpma\_ep\_next\_conn\_req**(3),
**rpma\_ep\_shutdown**(3), **rpma\_peer\_new**(3), **librpma**(7) and
https://pmem.io/rpma/
