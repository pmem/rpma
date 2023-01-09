---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_get_rcq_size.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_cfg_get_rcq_size** - get receive CQ size for the connection

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_get_rcq_size(const struct rpma_conn_cfg *cfg, uint32_t *rcq_size);

# DESCRIPTION

**rpma_conn_cfg_get_rcq_size**() gets the receive CQ size for the
connection. Please see the **rpma_conn_get_rcq**() for details about the
receive CQ.

# RETURN VALUE

The **rpma_conn_cfg_get_rcq_size**() function returns 0 on success or a
negative error code on failure. **rpma_conn_cfg_get_rcq_size**() does
not set \*rcq_size value on failure.

# ERRORS

**rpma_conn_cfg_get_rcq_size**() can fail with the following error:

-   RPMA_E\_INVAL - cfg or rcq_size is NULL

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_cfg_set_rcq_size**(3),
**rpma_conn_get_rcq**(3), **librpma**(7) and https://pmem.io/rpma/
