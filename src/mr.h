/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr.h -- internal definitions for librpma
 */

#ifndef LIBRPMA_MR_H
#define LIBRPMA_MR_H 1

struct rpma_mr_local {
	struct ibv_mr *ibv_mr;
};

#define RPMA_MR_USAGE_VALID (RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST)

#define RPMA_FLAG_ON(set, flag) (set) |= (flag)
#define RPMA_FLAG_OFF(set, flag) (set) &= ~(flag)

#endif /* LIBRPMA_MR_H */
