---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_get_rcq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.14.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_srq\_get\_rcq \-- get the receive CQ from the shared RQ object**

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_srq;
          struct rpma_cq;
          int rpma_srq_get_rcq(const struct rpma_srq *srq, struct rpma_cq **rcq_ptr);

DESCRIPTION
===========

**rpma\_srq\_get\_rcq() gets the receive CQ from the shared RQ object.**
The receive CQ created by **rpma\_srq\_new(3) allows handling all**
**rpma\_srq\_recv(3) completions within the shared RQ.
rpma\_srq\_cfg\_set\_rcq\_size(3)** can change the receive CQ size.

RETURN VALUE
============

The **rpma\_srq\_get\_rcq() function returns 0 on success or a negative
error** code on failure. **rpma\_srq\_get\_rcq() does not set \*rcq\_ptr
value on failure.**

ERRORS
======

**rpma\_srq\_get\_rcq() can fail with the following error:**

-   RPMA\_E\_INVAL - srq or rcq\_ptr is NULL

SEE ALSO
========

**rpma\_cq\_wait(3), rpma\_cq\_get\_wc(3), rpma\_cq\_get\_fd(3),**
**rpma\_srq\_cfg\_set\_rcq\_size(3), rpma\_srq\_new(3), librpma(7)** and
https://pmem.io/rpma/
