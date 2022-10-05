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
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_mr\_remote\_from\_descriptor** - create a memory region from a
descriptor

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_from_descriptor(const void *desc, size_t desc_size,
                          struct rpma_mr_remote **mr_ptr);

DESCRIPTION
===========

Create a remote memory region\'s structure based on the provided
descriptor with a network-transferable description of the memory region
local to the remote peer.

SECURITY WARNING
================

An attacker might modify the serialized remote memory registration
configuration while it is transferred via an unsecured connection (e.g.
rdma\_cm private data), which might cause data corruption when writing
to a different location. Users should avoid using
**rpma\_conn\_get\_private\_data**(3) and
**rpma\_conn\_req\_get\_private\_data**(3) API calls and they should
utilize TLS/SSL connections to transfer all configuration data between
peers instead.

RETURN VALUE
============

The **rpma\_mr\_remote\_from\_descriptor**() function returns 0 on
success or a negative error code on failure.
**rpma\_mr\_remote\_from\_descriptor**() does not set \*mr\_ptr value on
failure.

ERRORS
======

**rpma\_mr\_remote\_from\_descriptor**() can fail with the following
errors:

-   RPMA\_E\_INVAL - desc or mr\_ptr is NULL

-   RPMA\_E\_INVAL - incompatible descriptor size

-   RPMA\_E\_NOSUPP - deserialized information does not represent a
    valid memory region

-   RPMA\_E\_NOMEM - out of memory

SEE ALSO
========

**rpma\_mr\_remote\_delete**(3),
**rpma\_mr\_remote\_get\_flush\_type**(3),
**rpma\_mr\_remote\_get\_size**(3), **rpma\_flush**(3),
**rpma\_read**(3), **rpma\_write**(3), **rpma\_atomic\_write**(3),
**librpma**(7) and https://pmem.io/rpma/
