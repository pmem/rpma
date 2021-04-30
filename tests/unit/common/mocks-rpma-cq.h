/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-cq.h -- librpma cq.c module mocks
 */

#ifndef MOCKS_RPMA_CQ_H
#define MOCKS_RPMA_CQ_H

extern struct rpma_cq Rpma_cq;

#define MOCK_RPMA_CQ		(struct rpma_cq *)&Rpma_cq

#endif /* MOCKS_RPMA_CQ_H */
