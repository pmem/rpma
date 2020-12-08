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

HEADER_LAT="#bytes #iterations    t_min[usec]    t_max[usec]  t_typical[usec]    t_avg[usec]    t_stdev[usec]   99% percentile[usec]   99.9% percentile[usec]"
HEADER_BW="#threads #bytes     #iterations    BW_peak[Gb/sec]    BW_average[Gb/sec]   MsgRate[Mpps]"

TIMESTAMP=$(date +%y-%m-%d-%H%M%S)

function usage()
{
    echo "Error: $1"
    echo
    echo "usage: $0 <bw-ds|bw-th|lat> <server_ip>"
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

function verify_data_size()
{
	if [ ${#DATA_SIZE[@]} -ne ${#ITERATIONS[@]} ]; then
		echo "Error: sizes of the arrays: DATA_SIZE(${#DATA_SIZE[@]}) and ITERATIONS(${#ITERATIONS[@]}) are different!"
		exit 1
	fi
}

function verify_threads()
{
	if [ ${#THREADS[@]} -ne ${#ITERATIONS[@]} ]; then
		echo "Error: sizes of the arrays: THREADS(${#THREADS[@]}) and ITERATIONS(${#ITERATIONS[@]}) are different!"
		exit 1
	fi
}

case $MODE in
bw-ds)
	IB_TOOL=ib_read_bw
	HEADER=$HEADER_BW
	THREADS=1
	DATA_SIZE=(256 1024 4096 8192 65536)
	# values measured empirically, so that duration was ~60s
	# 100000000 is the maximum value of iterations
	ITERATIONS=(100000000 100000000 100000000 85753202 11138529)
	AUX_PARAMS="$AUX_PARAMS --report_gbits"
	NAME="${MODE}-${THREADS}th"
	verify_data_size
	;;
bw-th)
	IB_TOOL=ib_read_bw
	HEADER=$HEADER_BW
	# XXX TH=16 hangs the ib_read_bw at the moment
	# XXX TH=16 takes 11143637 iterations to run for ~60s
	THREADS=(1 2 4 8 12)
	DATA_SIZE=4096
	# values measured empirically, so that duration was ~60s
	# 100000000 is the maximum value of iterations
	ITERATIONS=(100000000 89126559 44581990 22290994 14859379)
	AUX_PARAMS="$AUX_PARAMS --report_gbits"
	NAME="${MODE}-${DATA_SIZE}ds"
	verify_threads
	;;
lat)
	IB_TOOL=ib_read_lat
	HEADER=$HEADER_LAT
	THREADS=1
	DATA_SIZE=(1024 4096 65536)
	# values measured empirically, so that duration was ~60s
	ITERATIONS=(27678723 20255739 6002473)
	AUX_PARAMS="$AUX_PARAMS --perform_warm_up"
	NAME="${MODE}"
	verify_data_size
	;;
*)
	usage "Wrong mode: $MODE"
	;;
esac

OUTPUT=ib_read_${NAME}-${TIMESTAMP}.csv
LOG_ERR=/dev/shm/ib_read_${NAME}-err-${TIMESTAMP}.log

echo "Performance results: $OUTPUT"
echo "Output and errors (both sides): $LOG_ERR"
echo

rm -f $LOG_ERR
echo "$HEADER" | sed 's/% /%_/g' | sed -r 's/[[:blank:]]+/,/g' > $OUTPUT

for i in $(seq 0 $(expr ${#ITERATIONS[@]} - 1)); do
	case $MODE in
	bw-ds)
		IT=${ITERATIONS[${i}]}
		DS="${DATA_SIZE[${i}]}"
		TH="${THREADS}"
		IT_OPT="--iters $IT"
		DS_OPT="--size $DS"
		QP_OPT="--qp $TH"
		echo -n "1," >> $OUTPUT
		;;
	bw-th)
		IT=${ITERATIONS[${i}]}
		DS="${DATA_SIZE}"
		TH="${THREADS[${i}]}"
		IT_OPT="--iters $IT"
		DS_OPT="--size $DS"
		QP_OPT="--qp $TH"
		echo -n "${TH}," >> $OUTPUT
		;;
	lat)
		IT=${ITERATIONS[${i}]}
		DS="${DATA_SIZE[${i}]}"
		TH="1"
		IT_OPT="--iters $IT"
		DS_OPT="--size $DS"
		QP_OPT=""
		;;
	esac

	# run the server
	sshpass -p "$REMOTE_PASS" -v ssh $REMOTE_USER@$SERVER_IP \
		"numactl -N $REMOTE_JOB_NUMA ${IB_TOOL} $DS_OPT $QP_OPT \
		$REMOTE_AUX_PARAMS >> $LOG_ERR" 2>>$LOG_ERR &
	sleep 1

	# XXX --duration hides detailed statistics
	echo "[size: ${DS}, threads: ${TH}, iters: ${IT}] (duration: ~60s)"
	numactl -N $JOB_NUMA ${IB_TOOL} $IT_OPT $DS_OPT $QP_OPT \
		$AUX_PARAMS $SERVER_IP 2>>$LOG_ERR | grep ${DS} | \
		grep -v '[B]' | sed 's/^[ ]*//' | sed 's/[ ]*$//' | \
		sed -r 's/[[:blank:]]+/,/g' >> $OUTPUT
done

CSV_MODE=$(echo ${IB_TOOL} | sed 's/_read//')

# convert to standardized-CSV
./csv2standardized.py --csv_type ${CSV_MODE} --output_file $OUTPUT $OUTPUT
