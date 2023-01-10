---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_delete.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.2.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_delete** - delete already closed connection

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_delete(struct rpma_conn **conn_ptr);

# DESCRIPTION

**rpma_conn_delete**() deletes already closed connection.

# RETURN VALUE

The **rpma_conn_delete**() function returns 0 on success or a negative
error code on failure. **rpma_conn_delete**() sets \*conn_ptr value to
NULL on success and on failure.

# ERRORS

**rpma_conn_delete**() can fail with the following errors:

-   RPMA_E\_INVAL - conn_ptr is NULL or **munmap**() failed

-   RPMA_E\_PROVIDER - **ibv_destroy_cq**() or **rdma_destroy_id**()
    failed

# SEE ALSO

**rpma_conn_disconnect**(3), **rpma_conn_req_connect**(3),
**librpma**(7) and https://pmem.io/rpma/
