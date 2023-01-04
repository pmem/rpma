---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_cfg_new** - create a new connection configuration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr);

# DESCRIPTION

**rpma_conn_cfg_new**() creates a new connection configuration object
and fills it with the default values:

            .timeout_ms = 1000
            .cq_size = 10
            .rcq_size = 0
            .sq_size = 10
            .rq_size = 10
            .shared_comp_channel = false

# RETURN VALUE

The **rpma_conn_cfg_new**() function returns 0 on success or a negative
error code on failure. **rpma_conn_cfg_new**() does not set \*cfg_ptr
value on failure.

# ERRORS

**rpma_conn_cfg_new**() can fail with the following error:

-   RPMA_E\_INVAL - cfg_ptr is NULL

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_conn_cfg_delete**(3), **rpma_conn_cfg_get_compl_channel**(3),
**rpma_conn_cfg_get_cq_size**(3), **rpma_conn_cfg_get_rq_size**(3),
**rpma_conn_cfg_get_sq_size**(3), **rpma_conn_cfg_get_timeout**(3),
**rpma_conn_cfg_set_compl_channel**(3),
**rpma_conn_cfg_set_cq_size**(3), **rpma_conn_cfg_set_rq_size**(3),
**rpma_conn_cfg_set_sq_size**(3), **rpma_conn_cfg_set_timeout**(3),
**rpma_conn_req_new**(3), **rpma_ep_next_conn_req**(3), **librpma**(7)
and https://pmem.io/rpma/
