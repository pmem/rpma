---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_req_get_private_data.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_req_get_private_data** - get a pointer to the request\'s
private data

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_req;
          struct rpma_conn_private_data;
          int rpma_conn_req_get_private_data(const struct rpma_conn_req *req,
                          struct rpma_conn_private_data *pdata);

# DESCRIPTION

**rpma_conn_req_get_private_data**() obtains the pointer to the
connection\'s private data given by the other side of the connection
before the connection is established.

# SECURITY WARNING

See **rpma_conn_get_private_data**(3).

# RETURN VALUE

The **rpma_conn_req_get_private_data**() function returns 0 on success
or a negative error code on failure.
**rpma_conn_req_get_private_data**() does not set \*pdata value on
failure.

# ERRORS

**rpma_conn_req_get_private_data**() can fail with the following error:

-   RPMA_E\_INVAL - req or pdata is NULL

# SEE ALSO

**rpma_conn_get_private_data**(3), **rpma_ep_next_conn_req**(3),
**librpma**(7) and https://pmem.io/rpma/
