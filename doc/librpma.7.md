---
layout: manual
Content-Style: 'text/css'
title: _MP(LIBRPMA, 7)
collection: librpma
header: PMDK
date: rpma API version 1.3
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2019-2020, Intel Corporation)

[comment]: <> (librpma.7 -- man page for librpma)


[NAME](#name)<br />
[SYNOPSIS](#synopsis)<br />
[DESCRIPTION](#description)<br />
[DEBUGGING AND ERROR HANDLING](#debugging-and-error-handling)<br />
[EXAMPLE](#example)<br />
[NOTE](#note)<br />
[ACKNOWLEDGEMENTS](#acknowledgements)<br />
[SEE ALSO](#see-also)


# NAME #

**librpma** - remote persistent memory access library (EXPERIMENTAL)

# SYNOPSIS #

```c
#include <librpma.h>
cc ... -lrpma
```

# DESCRIPTION #

**librpma** provides ...

# DEBUGGING AND ERROR HANDLING #

```c
const char *rpma_errormsg(void);
```

# EXAMPLE #

The following example uses **librpma** to ...

```c
#include <librpma.h>

/* XXX */
```

# NOTE #

The **librpma** API is experimental and may be subject to change in the future.

# ACKNOWLEDGEMENTS #

**librpma** builds on the persistent memory programming model
recommended by the SNIA NVM Programming Technical Work Group:
<http://snia.org/nvmp>

# SEE ALSO #

**XXX**(3)
and **<https://pmem.io>**
