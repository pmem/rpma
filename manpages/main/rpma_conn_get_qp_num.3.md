---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_qp_num.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_get_qp_num** - get the connection\'s qp_num

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_get_qp_num(const struct rpma_conn *conn, uint32_t *qp_num);

# DESCRIPTION

**rpma_conn_get_qp_num**() obtains the unique identifier of the
connection.

# RETURN VALUE

The **rpma_conn_get_qp_num**() function returns 0 on success or a
negative error code on failure. **rpma_conn_get_qp_num**() does not set
\*qp_num value on failure.

# ERRORS

**rpma_conn_get_qp_num**() can fail with the following error:

-   RPMA_E\_INVAL - conn or qp_num is NULL

# SEE ALSO

**rpma_conn_req_new**(3), **rpma_ep_next_conn_req**(3),
**rpma_conn_req_connect**(3), **librpma**(7) and https://pmem.io/rpma/
