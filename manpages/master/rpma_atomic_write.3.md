---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_atomic_write.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_atomic\_write \-- initiate the atomic 8 bytes write operation**

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_remote;
          int rpma_atomic_write(struct rpma_conn *conn,
                          struct rpma_mr_remote *dst, size_t dst_offset,
                          const char src[8], int flags, const void *op_context);

DESCRIPTION
===========

**rpma\_atomic\_write() initiates the atomic 8 bytes write operation
(transferring data from** the local memory to the remote memory). The
atomic write operation allows transferring exactly 8 bytes of data and
storing them atomically in the remote memory.

The attribute flags set the completion notification indicator:

-   RPMA\_F\_COMPLETION\_ON\_ERROR - generate the completion on error

-   RPMA\_F\_COMPLETION\_ALWAYS - generate the completion regardless of
    result of the operation.

op\_context is returned in the wr\_id field of the completion (struct
ibv\_wc).

RETURN VALUE
============

The **rpma\_atomic\_write() function returns 0 on success or a negative
error code on failure.**

ERRORS
======

**rpma\_atomic\_write() can fail with the following errors:**

-   RPMA\_E\_INVAL - conn, dst or src is NULL

-   RPMA\_E\_INVAL - dst\_offset is not aligned to 8 bytes

-   RPMA\_E\_INVAL - flags are not set (flags == 0)

-   RPMA\_E\_PROVIDER - **ibv\_post\_send(3) failed**

SEE ALSO
========

**rpma\_conn\_req\_connect(3), rpma\_mr\_reg(3),
rpma\_mr\_remote\_from\_descriptor(3), librpma(7) and**
https://pmem.io/rpma/
