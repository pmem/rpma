---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_compl_fd.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_conn\_get\_compl\_fd** - get a file descriptor of the shared
completion channel of the connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          int fd;
          int rpma_conn_get_compl_fd(const struct rpma_conn *conn, int *fd);

DESCRIPTION
===========

**rpma\_conn\_get\_compl\_fd**() gets a file descriptor of the shared
completion channel from the connection.

RETURN VALUE
============

The **rpma\_conn\_get\_compl\_fd**() function returns 0 on success or a
negative error code on failure. **rpma\_conn\_get\_compl\_fd**() does
not set \*fd value on failure.

ERRORS
======

**rpma\_conn\_get\_compl\_fd**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or fd is NULL

-   RPMA\_E\_NOT\_SHARED\_CHNL - the completion event channel is not
    shared

SEE ALSO
========

**librpma**(7), **rpma\_conn\_req\_connect**(3) and
https://pmem.io/rpma/
