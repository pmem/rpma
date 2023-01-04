---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_cfg_set_direct_write_to_pmem.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_peer_cfg_set_direct_write_to_pmem** - declare direct write to
PMEM support

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg, bool supported);

# DESCRIPTION

**rpma_peer_cfg_set_direct_write_to_pmem**() declares the support of the
direct write to PMEM.

# RETURN VALUE

The **rpma_peer_cfg_set_direct_write_to_pmem**() function returns 0 on
success or a negative error code on failure.

# ERRORS

**rpma_peer_cfg_set_direct_write_to_pmem**() can fail with the following
error:

-   RPMA_E\_INVAL - pcfg is NULL

# SEE ALSO

**rpma_conn_apply_remote_peer_cfg**(3),
**rpma_peer_cfg_get_descriptor**(3),
**rpma_peer_cfg_get_direct_write_to_pmem**(3), **rpma_peer_cfg_new**(3),
**librpma**(7) and https://pmem.io/rpma/
