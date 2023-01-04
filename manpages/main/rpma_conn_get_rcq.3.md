---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_rcq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_get_rcq \-- get the connection\'s receive CQ**

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_cq;
          int rpma_conn_get_rcq(const struct rpma_conn *conn, struct rpma_cq **rcq_ptr);

# DESCRIPTION

**rpma_conn_get_rcq**() gets the receive CQ from the connection. The
receive CQ allows handling all **rpma_recv**(3) completions within the
connection. It allows separating **rpma_recv**(3) completions processing
path from all other completions. The receive CQ is created only if the
receive CQ size in the provided connection configuration is greater than
0. When the receive CQ does not exist for the given connection the
\*rcq_ptr == NULL.

# RETURN VALUE

The **rpma_conn_get_rcq**() function returns 0 on success or a negative
error code on failure. **rpma_conn_get_rcq**() does not set \*rcq_ptr
value on failure.

# ERRORS

**rpma_conn_get_rcq**() can fail with the following error:

-   RPMA_E\_INVAL - conn or rcq_ptr is NULL

# SEE ALSO

**rpma_conn_cfg_set_rcq_size**(3), **rpma_conn_req_connect**(3),
**rpma_conn_get_cq**(3), **rpma_cq_wait**(3), **rpma_cq_get_wc**(3),
**rpma_cq_get_fd**(3), **rpma_recv**(3), **librpma**(7) and
https://pmem.io/rpma/
