#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_bench.sh -- run all benchmarks required for the performance report
#                    (EXPERIMENTAL)
#

function usage()
{
	echo "Error: $1"
	echo
	echo "Usage: $0 <server_ip>"
	echo
	echo "Requirements:"
	echo "export REMOTE_SUDO_NOPASSWD=1"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>"
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem"
	echo
	exit 1
}

if [ $# -lt 1 ]; then
	usage "Too few arguments"
elif [ "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
	usage "REMOTE_SUDO_NOPASSWD!=1"
elif [ -z "$REMOTE_RNIC_PCIE_ROOT_PORT" ]; then
	usage "REMOTE_RNIC_PCIE_ROOT_PORT not set"
elif [ -z "$REMOTE_JOB_MEM_PATH" ]; then
	usage "REMOTE_JOB_MEM_PATH not set"
fi

SERVER_IP=$1
PMEM=$REMOTE_JOB_MEM_PATH
DRAM="malloc"

echo "READ LAT/BW"
# -x is used as cheap logging
set -x
for mode in lat bw-bs bw-th; do
	./ib_read.sh $SERVER_IP $mode
	for op in read randread; do
		REMOTE_JOB_MEM_PATH=$DRAM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
		REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
	done
done
set +x

echo "WRITE LAT/BW"
# -x is used as cheap logging
set -x
for mode in lat bw-bs bw-th; do
	for op in write randwrite; do
		# XXX the reference pipeline performance requires APM-style write but with
		# DDIO=on which is not the default for APM benchmarking
		REMOTE_JOB_MEM_PATH=$DRAM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
		REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
		REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP gpspm $op $mode
	done
done
set +x

echo "MIX LAT/BW"
# -x is used as cheap logging
# To be compared with "PMEM apm read/randread" and "PMEM apm write/randwrite"
set -x
for mode in lat bw-bs bw-th; do
	for op in rw randrw; do
		REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
	done
done
set +x
