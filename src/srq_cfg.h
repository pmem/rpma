/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg.h -- librpma shared-RQ-configuration-related internal definitions
 */

#ifndef LIBRPMA_SRQ_CFG_H
#define LIBRPMA_SRQ_CFG_H

#include "librpma.h"

/*
 * ERRORS
 * rpma_srq_cfg_default() cannot fail.
 */
struct rpma_srq_cfg *rpma_srq_cfg_default();

/*
 * ERRORS
 * rpma_srq_cfg_get_rcqe() cannot fail.
 *
 * ASSUMPTIONS
 * cfg != NULL && rcqe != NULL
 */
void rpma_srq_cfg_get_rcqe(const struct rpma_srq_cfg *cfg, int *rcqe);

#endif /* LIBRPMA_SRQ_CFG_H */
