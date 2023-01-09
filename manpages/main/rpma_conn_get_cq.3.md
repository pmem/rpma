---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_cq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_get_cq** - get the connection\'s main CQ

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_cq;
          int rpma_conn_get_cq(const struct rpma_conn *conn, struct rpma_cq **cq_ptr);

# DESCRIPTION

**rpma_conn_get_cq**() gets the main CQ from the connection. When the
receive CQ is not present the main CQ allows handling all completions
within the connection. When the receive CQ is present the main CQ allows
handling all completions except **rpma_recv**(3) completions within the
connection. Please see **rpma_conn_get_rcq**(3) for details about the
receive CQ.

# RETURN VALUE

The **rpma_conn_get_cq**() function returns 0 on success or a negative
error code on failure. **rpma_conn_get_cq**() does not set \*cq_ptr
value on failure.

# ERRORS

**rpma_conn_get_cq**() can fail with the following error:

-   RPMA_E\_INVAL - conn or cq_ptr is NULL

# SEE ALSO

**rpma_conn_req_connect**(3), **rpma_conn_get_rcq**(3),
**rpma_cq_wait**(3), **rpma_cq_get_wc**(3), **rpma_cq_get_fd**(3),
**rpma_recv**(3), **librpma**(7) and https://pmem.io/rpma/
