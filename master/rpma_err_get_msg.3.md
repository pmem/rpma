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

**rpma\_err\_get\_msg** - return the last error message

SYNOPSIS
========

          #include <librpma.h>

          const char *rpma_err_get_msg(void);

DESCRIPTION
===========

If an error is detected during the call to a **librpma**(7) function,
the application may retrieve an error message describing the reason of
the failure from **rpma\_err\_get\_msg**(). The error message buffer is
thread-local; errors encountered in one thread do not affect its value
in other threads. The buffer is never cleared by any library function;
its content is significant only when the return value of the immediately
preceding call to a **librpma**(7) function indicated an error. The
application must not modify or free the error message string. Subsequent
calls to other library functions may modify the previous message.

RETURN VALUE
============

The **rpma\_err\_get\_msg**() function returns a pointer to a static
buffer containing the last error message logged for the current thread.

SEE ALSO
========

**librpma**(7) and **\<https://pmem.io\>**
