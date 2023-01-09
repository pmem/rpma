---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_cfg_get_descriptor.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_peer_cfg_get_descriptor** - get the descriptor of the peer
configuration

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg, void *desc);

# DESCRIPTION

**rpma_peer_cfg_get_descriptor**() gets the descriptor of the peer
configuration.

# SECURITY WARNING

See **rpma_peer_cfg_from_descriptor**(3).

# RETURN VALUE

The **rpma_peer_cfg_get_descriptor**() function returns 0 on success or
a negative error code on failure.

# ERRORS

**rpma_peer_cfg_get_descriptor**() can fail with the following error:

-   RPMA_E\_INVAL - pcfg or desc are NULL

# SEE ALSO

**rpma_peer_cfg_from_descriptor**(3), **rpma_peer_cfg_new**(3),
**librpma**(7) and https://pmem.io/rpma/
