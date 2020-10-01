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

**rpma\_utils\_conn\_event\_2str** - convert RPMA\_CONN\_\* enum to a
string

SYNOPSIS
========

          #include <librpma.h>

          const char *rpma_utils_conn_event_2str(enum rpma_conn_event conn_event);

          enum rpma_conn_event{
                  RPMA_CONN_UNDEFINED = -1,
                  RPMA_CONN_ESTABLISHED,
                  RPMA_CONN_CLOSED,
                  RPMA_CONN_LOST
          };

DESCRIPTION
===========

**rpma\_utils\_conn\_event\_2str**() converts RPMA\_CONN\_\* enum to the
const string representation.

RETURN VALUE
============

The **rpma\_utils\_conn\_event\_2str**() function returns the const
string representation of RPMA\_CONN\_\* enums.

ERRORS
======

**rpma\_utils\_conn\_event\_2str**() can not fail.

SEE ALSO
========

**rpma\_conn\_next\_event**(3), **librpma**(7) and https://pmem.io/rpma/
