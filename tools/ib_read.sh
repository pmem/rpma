#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# ib_read.sh -- a single-sided ib_read_lat/bw tool (EXPERIMENTAL)
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
HEADER=" #bytes #iterations    t_min[usec]    t_max[usec]  t_typical[usec]    t_avg[usec]    t_stdev[usec]   99% percentile[usec]   99.9% percentile[usec]"

function usage()
{
    echo "Error: $1"
    echo
    echo "usage: $0 <bw|lat> <server_ip>"
    echo
    echo "export JOB_NUMA=0"
    echo "export AUX_PARAMS='-d mlx5_0 -R'"
    echo "export REMOTE_USER=user"
    echo "export REMOTE_PASS=pass"
    echo "export REMOTE_JOB_NUMA=0"
    echo "export REMOTE_AUX_PARAMS='-d mlx5_0 -R'"
    exit 1
}

if [ "$#" -lt 2 ]; then
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

MODE=$1
SERVER_IP=$2

case $MODE in
bw)
	IB_TOOL=ib_read_bw
	;;
lat)
	IB_TOOL=ib_read_lat
	AUX_PARAMS="$AUX_PARAMS --perform_warm_up"
	;;
*)
	usage "Wrong mode: $MODE"
	;;
esac

OUTPUT=${IB_TOOL}-${TIMESTAMP}.csv
LOG_ERR=/dev/shm/${IB_TOOL}_err-${TIMESTAMP}.log

echo "Performance results: $OUTPUT"
echo "Output and errors (both sides): $LOG_ERR"
echo

rm -f $LOG_ERR
echo "$HEADER" | sed 's/% /%_/g' | sed -r 's/[[:blank:]]+/,/g' > $OUTPUT

for ds in $DATA_SIZE; do
	# run the server
	sshpass -p "$REMOTE_PASS" -v ssh $REMOTE_USER@$SERVER_IP \
		"numactl -N $REMOTE_JOB_NUMA ${IB_TOOL} --size $ds \
		$REMOTE_AUX_PARAMS > $LOG_ERR" 2>>$LOG_ERR &
	sleep 1

	# XXX --duration hides detailed statistics
	echo "[size: $ds, iters: $ITERATIONS] (duration: ~60s)"
	numactl -N $JOB_NUMA ${IB_TOOL} --size $ds --iters $ITERATIONS \
		$SERVER_IP $AUX_PARAMS 2>>$LOG_ERR | grep $ds | \
		grep -v '[B]' | sed 's/^[ ]*//' | sed 's/[ ]*$//' | \
		sed -r 's/[[:blank:]]+/,/g' >> $OUTPUT
done

# convert to standardized-CSV
./csv2standardized.py --csv_type ib --output_file $OUTPUT $OUTPUT
