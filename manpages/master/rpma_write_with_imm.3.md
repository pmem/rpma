---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_write_with_imm.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.14.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_write\_with\_imm** - initiate the write operation with immediate
data

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_local;
          struct rpma_mr_remote;
          int rpma_write_with_imm(struct rpma_conn *conn,
                          struct rpma_mr_remote *dst, size_t dst_offset,
                          const struct rpma_mr_local *src,  size_t src_offset,
                          size_t len, int flags, uint32_t imm,
                          const void *op_context);

DESCRIPTION
===========

**rpma\_write\_with\_imm**() initiates the write operation with
immediate data (transferring data from the local memory to the remote
memory. To write a 0 bytes message, set src and dst to NULL and
src\_offset, dst\_offset and len to 0.

The attribute flags set the completion notification indicator:

-   RPMA\_F\_COMPLETION\_ON\_ERROR - generate the completion on error

-   RPMA\_F\_COMPLETION\_ALWAYS - generate the completion regardless of
    result of the operation.

op\_context is returned in the wr\_id field of the completion (struct
ibv\_wc).

RETURN VALUE
============

The **rpma\_write\_with\_imm**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_write\_with\_imm**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn == NULL \|\| flags == 0

-   RPMA\_E\_INVAL - dst == NULL && (src != NULL \|\| src\_offset != 0
    \|\| dst\_offset != 0 \|\| len != 0)

-   RPMA\_E\_INVAL - src == NULL && (dst != NULL \|\| src\_offset != 0
    \|\| dst\_offset != 0 \|\| len != 0)

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed

SEE ALSO
========

**rpma\_conn\_req\_connect**(3), **rpma\_mr\_reg**(3),
**rpma\_mr\_remote\_from\_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
