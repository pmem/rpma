---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_set_srq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

# NAME

**rpma_conn_cfg_set_srq** - set a shared RQ object for the connection

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          struct rpma_srq;
          int rpma_conn_cfg_set_srq(struct rpma_conn_cfg *cfg, struct rpma_srq *srq);

# DESCRIPTION

**rpma_conn_cfg_set_srq**() sets a shared RQ object for the connection.
If this function is not called, the srq has the default value (NULL) set
by **rpma_conn_cfg_new**(3).

# RETURN VALUE

The **rpma_conn_cfg_set_srq**() function returns 0 on success or a
negative error code on failure.

# ERRORS

**rpma_conn_cfg_set_srq**() can fail with the following error:

-   RPMA_E\_INVAL - cfg is NULL

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_cfg_get_srq**(3), **librpma**(7)
and https://pmem.io/rpma/
