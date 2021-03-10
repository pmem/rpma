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
	echo "export REMOTE_SUDO_NOPASSWD=0/1"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=0/1"
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem"
	echo
	exit 1
}

if [ $# -lt 1 ]; then
	usage "Too few arguments"
elif [ -z "$REMOTE_JOB_MEM_PATH" ]; then
	usage "REMOTE_JOB_MEM_PATH not set"
elif [ -z "$REMOTE_RNIC_PCIE_ROOT_PORT" -a "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
	usage "REMOTE_RNIC_PCIE_ROOT_PORT not set"
elif [ -z "$REMOTE_DIRECT_WRITE_TO_PMEM" -a "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
	usage "REMOTE_DIRECT_WRITE_TO_PMEM not set"
fi

SERVER_IP=$1
PMEM=$REMOTE_JOB_MEM_PATH
DRAM="malloc"

set -e # stop on the first error

if [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
	echo "Run all workloads."
else
	if [ "$REMOTE_DIRECT_WRITE_TO_PMEM" == "1" ]; then
		echo "Run APM workloads."
	else
	        echo "RUN GPSPM workloads."
	fi
fi

echo "READ LAT/BW"
# -x is used as cheap logging
set -x
for mode in lat bw-bs bw-th; do
	./ib_read.sh $SERVER_IP $mode
	if [ "$REMOTE_DIRECT_WRITE_TO_PMEM" == "1" ] || [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		for op in read randread; do
			REMOTE_JOB_MEM_PATH=$DRAM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
			REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
		done
	fi
done
set +x

echo "WRITE LAT/BW"
# -x is used as cheap logging
set -x
for mode in lat bw-bs bw-th; do
	if [ "$REMOTE_DIRECT_WRITE_TO_PMEM" == "1" ] || [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		for op in write randwrite; do
			REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
		done
	fi
	if [ "$REMOTE_DIRECT_WRITE_TO_PMEM" == "0" ] || [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		for op in write randwrite; do
			REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP gpspm $op $mode
		done
	fi
done
set +x

echo "MIX LAT/BW"
# -x is used as cheap logging
# To be compared with "PMEM apm read/randread" and "PMEM apm write/randwrite"
set -x
for mode in lat bw-bs bw-th; do
	if [ "$REMOTE_DIRECT_WRITE_TO_PMEM" == "1" ] || [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		for op in rw randrw; do
			REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm $op $mode
		done
	fi
done
set +x
