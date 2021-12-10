/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2021, Intel Corporation */

/*
 * mocks-unistd.h -- the unistd mocks' header
 */

#ifndef MOCKS_UNISTD_H
#define MOCKS_UNISTD_H

#define PAGESIZE 4096

void enable_unistd_mocks(void);
void disable_unistd_mocks(void);

long __wrap_sysconf(int name);

#endif /* MOCKS_UNISTD_H */
