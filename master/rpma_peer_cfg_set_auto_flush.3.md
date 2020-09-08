---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_peer\_cfg\_set\_auto\_flush** - declare the auto flush state

SYNOPSIS
========

          #include <librpma.h>

          int rpma_peer_cfg_set_auto_flush(struct rpma_peer_cfg *pcfg,
                          enum rpma_on_off_type state);

DESCRIPTION
===========

ERRORS
======

**rpma\_peer\_cfg\_set\_auto\_flush**() can fail with the following
error:

-   XXX
