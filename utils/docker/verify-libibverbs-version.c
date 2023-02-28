// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023, Intel Corporation */

/*
 * verify-libibverbs-version.c - check if libibverbs supports native atomic write and native flush
 */

#include <infiniband/verbs.h>

int main() {
	/*
	 * Check if IB_UVERBS_DEVICE_ATOMIC_WRITE, IBV_QP_EX_WITH_ATOMIC_WRITE
	 * and ibv_wr_atomic_write() are defined.
	 */
	uint64_t device_cap_flag_1 = IB_UVERBS_DEVICE_ATOMIC_WRITE;
	uint64_t send_ops_flag_1 = IBV_QP_EX_WITH_ATOMIC_WRITE;
	int native_atomic_write = (ibv_wr_atomic_write != NULL);

	/*
	 * Check if IBV_ACCESS_FLUSH_GLOBAL, IBV_ACCESS_FLUSH_PERSISTENT,
	 * IB_UVERBS_DEVICE_FLUSH_GLOBAL, IB_UVERBS_DEVICE_FLUSH_PERSISTENT,
	 * IBV_QP_EX_WITH_FLUSH and ibv_wr_flush() are defined.
	 */
	int access = IBV_ACCESS_FLUSH_GLOBAL | IBV_ACCESS_FLUSH_PERSISTENT;
	uint64_t device_cap_flag_2 = IB_UVERBS_DEVICE_FLUSH_GLOBAL |
					IB_UVERBS_DEVICE_FLUSH_PERSISTENT;
	uint64_t send_ops_flag_2 = IBV_QP_EX_WITH_FLUSH;
	int native_flush = (ibv_wr_flush != NULL);

	return !(native_atomic_write && native_flush);
}
