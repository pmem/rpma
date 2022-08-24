---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_rcq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_conn\_get\_rcq \-- get the connection\'s receive CQ**

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_cq;
          int rpma_conn_get_rcq(const struct rpma_conn *conn, struct rpma_cq **rcq_ptr);

DESCRIPTION
===========

**rpma\_conn\_get\_rcq() gets the receive CQ from the connection. The
receive CQ allows handling all** **rpma\_recv(3) completions within the
connection. It allows separating rpma\_recv(3) completions** processing
path from all other completions. The receive CQ is created only if the
receive CQ size in the provided connection configuration is greater than
0. When the receive CQ does not exist for the given connection the
\*rcq\_ptr == NULL.

RETURN VALUE
============

The **rpma\_conn\_get\_rcq() function returns 0 on success or a negative
error code on failure.** **rpma\_conn\_get\_rcq() does not set
\*rcq\_ptr value on failure.**

ERRORS
======

**rpma\_conn\_get\_rcq() can fail with the following error:**

-   RPMA\_E\_INVAL - conn or rcq\_ptr is NULL

SEE ALSO
========

**rpma\_conn\_cfg\_set\_rcq\_size(3), rpma\_conn\_req\_connect(3),
rpma\_conn\_get\_cq(3), rpma\_cq\_wait(3),** **rpma\_cq\_get\_wc(3),
rpma\_cq\_get\_fd(3), rpma\_recv(3), librpma(7) and
https://pmem.io/rpma/**
