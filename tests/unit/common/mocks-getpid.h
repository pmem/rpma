/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * mocks-getpid.h -- the getpid mock's header
 */

#ifndef MOCKS_GETPID_H
#define MOCKS_GETPID_H

#include <unistd.h>

/*
 * __wrap_getpid -- getpid() mock
 */
__pid_t
__wrap_getpid();

#endif /* MOCKS_GETPID_H */
