#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_bench.sh -- run all benchmarks required for the performance report
#					 (EXPERIMENTAL)
#

function usage()
{
	echo "Error: $1"
	echo
	echo "Usage: $0 <server_ip>"
    echo
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem (required)"
	echo
	exit 1
}

if [ $# -lt 1 ]; then
	usage "Too few arguments"
elif [ -z "$REMOTE_JOB_MEM_PATH" ]; then
	usage "REMOTE_JOB_MEM_PATH not set"
fi

# export is required to access these variables from a subshell
export SERVER_IP=$1
export PMEM=$REMOTE_JOB_MEM_PATH
export DRAM="malloc"

echo "READ LAT"
# The subshell and -x is used as cheap logging
(set -x; \
    ./ib_read.sh $SERVER_IP lat &&
    REMOTE_JOB_MEM_PATH=$DRAM ./rpma_fio_bench.sh $SERVER_IP apm read lat && \
    REMOTE_JOB_MEM_PATH=$PMEM ./rpma_fio_bench.sh $SERVER_IP apm randread lat
)

# XXX To be continued...
