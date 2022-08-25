/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Fujitsu */

/*
 * srq.h -- librpma shared-RQ-related internal definitions
 */

#ifndef LIBRPMA_SRQ_H
#define LIBRPMA_SRQ_H

#include <infiniband/verbs.h>

#include "librpma.h"

/*
 * ERRORS
 * rpma_srq_get_ibv_srq() cannot fail.
 */
struct ibv_srq *rpma_srq_get_ibv_srq(const struct rpma_srq *srq);

#endif /* LIBRPMA_SRQ_H */
