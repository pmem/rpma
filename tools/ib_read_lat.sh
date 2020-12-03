#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# ib_read_lat.sh -- a single-sided ib_read_lat tool (EXPERIMENTAL)
#
# Spawns both server and the client, collects the results for multile data
# sizes (1KiB, 4KiB, 64KiB) and generates a single CSV file with all
# the resutls.
#

DATA_SIZE="1024 4096 65536"
ITERATIONS=10000000
TIMESTAMP=$(date +%y-%m-%d-%H%M%S)
OUTPUT=ib_read_lat-${TIMESTAMP}.csv
LOG_ERR=/dev/shm/ib_read_lat_err-${TIMESTAMP}.log
HEADER="#bytes #iterations    t_min[usec]    t_max[usec]  t_typical[usec]    t_avg[usec]    t_stdev[usec]   99% percentile[usec]   99.9% percentile[usec]"

function usage()
{
    echo "Error: $1"
    echo
    echo "usage: $0 <SERVER_IP>"
    echo
    echo "export JOB_NUMA=0"
    echo "export AUX_PARAMS='-d mlx5_0 -R'"
    echo "export REMOTE_USER=user"
    echo "export REMOTE_PASS=pass"
    echo "export REMOTE_JOB_NUMA=0"
    echo "export REMOTE_AUX_PARAMS='-d mlx5_0 -R'"
    exit 1
}

echo "Performance results: $OUTPUT"
echo "Output and errors (both sides): $LOG_ERR"
echo

if [ "$#" -lt 1 ]; then
    usage "Too few arguments"
elif [ -z "$JOB_NUMA" ]; then
    usage "JOB_NUMA not set"
elif [ -z "$REMOTE_USER" ]; then
    usage "REMOTE_USER not set"
elif [ -z "$REMOTE_PASS" ]; then
    usage "REMOTE_PASS not set"
elif [ -z "$REMOTE_JOB_NUMA" ]; then
    usage "REMOTE_JOB_NUMA not set"
fi

SERVER_IP=$1

rm -f $LOG_ERR
echo "$HEADER" | sed 's/% /%_/g' | sed -r 's/[[:blank:]]+/,/g' > $OUTPUT

for ds in $DATA_SIZE; do
    # run the server
    sshpass -p "$REMOTE_PASS" -v ssh $REMOTE_USER@$SERVER_IP \
        "numactl -N $REMOTE_JOB_NUMA ib_read_lat --size $ds \
        $REMOTE_AUX_PARAMS > $LOG_ERR" 2>>$LOG_ERR &
    sleep 1

    # XXX --duration hides detailed statistics
    echo "[size: $ds, iters: $ITERATIONS] (duration: ~60s)"
    numactl -N $JOB_NUMA ib_read_lat --size $ds --iters $ITERATIONS \
        --perform_warm_up $SERVER_IP $AUX_PARAMS 2>>$LOG_ERR | grep $ds | \
        grep -v '[B]' | sed 's/^[ ]*//' | sed 's/[ ]*$//' | \
        sed -r 's/[[:blank:]]+/,/g' >> $OUTPUT
done
