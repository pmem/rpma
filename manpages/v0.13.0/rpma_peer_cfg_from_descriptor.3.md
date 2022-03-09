---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_cfg_from_descriptor.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.13.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_peer\_cfg\_from\_descriptor** - create a peer cfg from the
descriptor

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_from_descriptor(const void *desc, size_t desc_size,
                          struct rpma_peer_cfg **pcfg_ptr);

DESCRIPTION
===========

**rpma\_peer\_cfg\_from\_descriptor**() creates a peer configuration
object from the descriptor.

RETURN VALUE
============

The **rpma\_peer\_cfg\_from\_descriptor**() function returns 0 on
success or a negative error code on failure.
**rpma\_peer\_cfg\_from\_descriptor**() does not set \*pcfg\_ptr value
on failure.

ERRORS
======

**rpma\_peer\_cfg\_from\_descriptor**() can fail with the following
errors:

-   RPMA\_E\_INVAL - desc or pcfg\_ptr are NULL

-   RPMA\_E\_NOMEM - out of memory

SEE ALSO
========

**rpma\_conn\_apply\_remote\_peer\_cfg**(3),
**rpma\_peer\_cfg\_get\_descriptor**(3), **rpma\_peer\_cfg\_new**(3),
**librpma**(7) and https://pmem.io/rpma/
