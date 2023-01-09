---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_get_sq_size.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_cfg_get_sq_size** - get SQ size for the connection

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_get_sq_size(const struct rpma_conn_cfg *cfg, uint32_t *sq_size);

# DESCRIPTION

**rpma_conn_cfg_get_sq_size**() gets the SQ size for the connection.

# RETURN VALUE

The **rpma_conn_cfg_get_sq_size**() function returns 0 on success or a
negative error code on failure. **rpma_conn_cfg_get_sq_size**() does not
set \*sq_size value on failure.

# ERRORS

**rpma_conn_cfg_get_sq_size**() can fail with the following error:

-   RPMA_E\_INVAL - cfg or sq_size is NULL

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_cfg_set_sq_size**(3),
**librpma**(7) and https://pmem.io/rpma/
