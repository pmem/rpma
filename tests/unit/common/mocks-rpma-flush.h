/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-flush.h -- a librpma_flush mocks header
 */

#ifndef MOCKS_RPMA_FLUSH_H
#define MOCKS_RPMA_FLUSH_H

extern struct rpma_flush Rpma_flush;
#define MOCK_FLUSH (struct rpma_flush *)&Rpma_flush

#endif /* #define MOCKS_RPMA_FLUSH_H */
