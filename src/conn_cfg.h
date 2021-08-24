/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_cfg.h -- librpma connection-configuration-related internal definitions
 */

#ifndef LIBRPMA_CONN_CFG_H
#define LIBRPMA_CONN_CFG_H

#include "librpma.h"

/*
 * ERRORS
 * rpma_conn_cfg_default() cannot fail.
 */
struct rpma_conn_cfg *rpma_conn_cfg_default();

/*
 * ERRORS
 * rpma_conn_cfg_get_cqe() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or cqe is NULL
 */
int rpma_conn_cfg_get_cqe(const struct rpma_conn_cfg *cfg, int *cqe);

/*
 * ERRORS
 * rpma_conn_cfg_get_rcqe() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or rcqe is NULL
 */
int rpma_conn_cfg_get_rcqe(const struct rpma_conn_cfg *cfg, int *rcqe);

#endif /* LIBRPMA_CONN_CFG_H */
