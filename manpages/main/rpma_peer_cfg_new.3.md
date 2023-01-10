---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_cfg_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.2.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_peer_cfg_new** - create a new peer configuration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg_ptr);

# DESCRIPTION

**rpma_peer_cfg_new**() creates a new peer configuration object.

# RETURN VALUE

The **rpma_peer_cfg_new**() function returns 0 on success or a negative
error code on failure. **rpm_peer_cfg_new**() does not set \*pcfg_ptr
value on failure.

# ERRORS

**rpma_peer_cfg_new**() can fail with the following errors:

-   RPMA_E\_INVAL - pcfg_ptr is NULL

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_conn_apply_remote_peer_cfg**(3), **rpma_peer_cfg_delete**(3),
**rpma_peer_cfg_from_descriptor**(3),
**rpma_peer_cfg_get_descriptor**(3),
**rpma_peer_cfg_get_descriptor_size**(3),
**rpma_peer_cfg_get_direct_write_to_pmem**(3),
**rpma_peer_cfg_set_direct_write_to_pmem**(3), **librpma**(7) and
https://pmem.io/rpma/
