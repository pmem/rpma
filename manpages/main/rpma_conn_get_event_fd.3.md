---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_event_fd.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_get_event_fd** - get an event file descriptor of the
connection

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_get_event_fd(const struct rpma_conn *conn, int *fd);

# DESCRIPTION

**rpma_conn_get_event_fd**() gets an event file descriptor of the
connection.

# RETURN VALUE

The **rpma_conn_get_event_fd**() function returns 0 on success or a
negative error code on failure. **rpma_conn_get_event_fd**() does not
set \*fd value on failure.

# ERRORS

**rpma_conn_get_event_fd**() can fail with the following error:

-   RPMA_E\_INVAL - conn or fd is NULL

# SEE ALSO

**rpma_conn_next_event**(3), **rpma_conn_req_connect**(3),
**librpma**(7) and https://pmem.io/rpma/
