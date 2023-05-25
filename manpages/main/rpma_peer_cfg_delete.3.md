---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_cfg_delete.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_peer_cfg_delete** - delete the peer configuration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg_ptr);

# DESCRIPTION

**rpma_peer_cfg_delete**() deletes the peer configuration object.

# RETURN VALUE

The **rpma_peer_cfg_delete**() function returns 0 on success or a
negative error code on failure. **rpm_peer_cfg_delete**() does not set
\*pcfg_ptr value to NULL on failure.

# ERRORS

**rpma_peer_cfg_delete**() can fail with the following error:

-   RPMA_E\_INVAL - pcfg_ptr is NULL

# SEE ALSO

**rpma_peer_cfg_new**(3), **librpma**(7) and https://pmem.io/rpma/
