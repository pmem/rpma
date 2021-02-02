#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
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

echo "This tool is EXPERIMENTAL"

function usage()
{
	echo "Error: $1"
	echo
	echo "usage: $0 <server_ip> <all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat>"
	echo
	echo "export JOB_NUMA=0"
	echo "export AUX_PARAMS='-d mlx5_0 -R'"
	echo "export IB_PATH=/custom/ib tool/path/"
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_AUX_PARAMS='-d mlx5_0 -R'"
	echo "export REMOTE_IB_PATH=/custom/ib tool/path/"
	echo "Debug:"
	echo "export SHORT_RUNTIME=1 (adequate for functional verification only)"
	echo "export LOG_CMDS=1 (it does not run the actual execution)"
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

function verify_block_size()
{
	if [ ${#BLOCK_SIZE[@]} -ne ${#ITERATIONS[@]} ]; then
		echo "Error: sizes of the arrays: BLOCK_SIZE(${#BLOCK_SIZE[@]}) and ITERATIONS(${#ITERATIONS[@]}) are different!"
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

function verify_depth()
{
	if [ ${#DEPTH[@]} -ne ${#ITERATIONS[@]} ]; then
		echo "Error: sizes of the arrays: DEPTH(${#DEPTH[@]}) and ITERATIONS(${#ITERATIONS[@]}) are different!"
		exit 1
	fi
}

function benchmark_one() {

	SERVER_IP=$1
	MODE=$2

	case $MODE in
	bw-bs)
		IB_TOOL=ib_read_bw
		HEADER=$HEADER_BW
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
		DEPTH=2
		# values measured empirically, so that duration was ~60s
		# 100000000 is the maximum value of iterations
		ITERATIONS=(48336720 48336720 34951167 24475088 23630690 8299603 5001135)
		AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}-${THREADS}th"
		verify_block_size
		;;
	bw-dp-exp)
		IB_TOOL=ib_read_bw
		HEADER=$HEADER_BW
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 4 8 16 32 64 128)
		# values measured empirically, so that duration was ~60s
		# 100000000 is the maximum value of iterations
		ITERATIONS=(20769620 30431214 45416656 65543498 85589536 100000000 100000000 100000000)
		AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}-${BLOCK_SIZE}bs"
		verify_depth
		;;
	bw-dp-lin)
		IB_TOOL=ib_read_bw
		HEADER=$HEADER_BW
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 3 4 5 6 7 8 9 10)
		# values measured empirically, so that duration was ~60s
		# 100000000 is the maximum value of iterations
		ITERATIONS=(20609419 30493585 40723132 43536049 50576557 55879517 60512919 65088286 67321386 68566797)
		AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}-${BLOCK_SIZE}bs"
		verify_depth
		;;
	bw-th)
		IB_TOOL=ib_read_bw
		HEADER=$HEADER_BW
		# XXX TH=16 hangs the ib_read_bw at the moment
		# XXX TH=16 takes 11143637 iterations to run for ~60s
		THREADS=(1 2 4 8 12)
		BLOCK_SIZE=4096
		DEPTH=2
		# values measured empirically, so that duration was ~60s
		# 100000000 is the maximum value of iterations
		ITERATIONS=(16527218 32344690 61246542 89456698 89591370)
		AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}-${BLOCK_SIZE}bs"
		verify_threads
		;;
	lat)
		IB_TOOL=ib_read_lat
		HEADER=$HEADER_LAT
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
		DEPTH=1
		# values measured empirically, so that duration was ~60s
		ITERATIONS=(27678723 27678723 20255739 16778088 11423082 8138946 6002473)
		AUX_PARAMS="$AUX_PARAMS --perform_warm_up"
		NAME="${MODE}"
		verify_block_size
		;;
	*)
		usage "Wrong mode: $MODE"
		;;
	esac

	NAME=ib_read_${NAME}-${TIMESTAMP}
	echo "STARTING benchmark for MODE=$MODE IP=$SERVER_IP ..."
	if [ "$LOG_CMDS" != "1" ]; then
		OUTPUT=${NAME}.csv
		LOG_ERR=/dev/shm/${NAME}-err.log
		echo "Performance results: $OUTPUT"
		echo "Output and errors (both sides): $LOG_ERR"
	else
		SERVER=${NAME}-server.log
		CLIENT=${NAME}-client.log
		echo "Log commands [server]: $SERVER"
		echo "Log commands [client]: $CLIENT"
	fi
	echo

	if [ "$LOG_CMDS" != "1" ]; then
		rm -f $LOG_ERR
		echo "$HEADER" | sed 's/% /%_/g' | sed -r 's/[[:blank:]]+/,/g' > $OUTPUT
	fi

	for i in $(seq 0 $(expr ${#ITERATIONS[@]} - 1)); do
		case $MODE in
		bw-bs)
			IT=${ITERATIONS[${i}]}
			BS="${BLOCK_SIZE[${i}]}"
			TH="${THREADS}"
			DP="${DEPTH}"
			IT_OPT="--iters $IT"
			BS_OPT="--size $BS"
			QP_OPT="--qp $TH"
			DP_OPT="--tx-depth=${DP}"
			[ "$LOG_CMDS" != "1" ] && echo -n "${TH},${DP}," >> $OUTPUT
			;;
		bw-dp-exp|bw-dp-lin)
			IT=${ITERATIONS[${i}]}
			BS="${BLOCK_SIZE}"
			TH="${THREADS}"
			DP="${DEPTH[${i}]}"
			IT_OPT="--iters $IT"
			BS_OPT="--size $BS"
			QP_OPT="--qp $TH"
			DP_OPT="--tx-depth=${DP}"
			[ "$LOG_CMDS" != "1" ] && echo -n "${TH},${DP}," >> $OUTPUT
			;;
		bw-th)
			IT=${ITERATIONS[${i}]}
			BS="${BLOCK_SIZE}"
			TH="${THREADS[${i}]}"
			DP="${DEPTH}"
			IT_OPT="--iters $IT"
			BS_OPT="--size $BS"
			QP_OPT="--qp $TH"
			DP_OPT="--tx-depth=${DP}"
			[ "$LOG_CMDS" != "1" ] && echo -n "${TH},${DP}," >> $OUTPUT
			;;
		lat)
			IT=${ITERATIONS[${i}]}
			BS="${BLOCK_SIZE[${i}]}"
			TH="$THREADS"
			DP="${DEPTH}"
			IT_OPT="--iters $IT"
			BS_OPT="--size $BS"
			QP_OPT=""
			DP_OPT=""
			;;
		esac

		if [ "$SHORT_RUNTIME" == "1" ]; then
			IT=100
			IT_OPT="--iters $IT"
		fi

		# run the server
		CMD="numactl -N $REMOTE_JOB_NUMA ${IB_PATH}${IB_TOOL} $BS_OPT $QP_OPT \
			$DP_OPT $REMOTE_AUX_PARAMS"
		if [ "$LOG_CMDS" != "1" ]; then
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$CMD >> $LOG_ERR" 2>>$LOG_ERR &
			sleep 1
		else
			echo "$CMD" >> $SERVER
		fi

		# XXX --duration hides detailed statistics
		echo "[size: ${BS}, threads: ${TH}, tx_depth: ${DP}, iters: ${IT}] (duration: ~60s)"
		CMD="numactl -N $JOB_NUMA ${REMOTE_IB_PATH}${IB_TOOL} $IT_OPT $BS_OPT \
			$QP_OPT $DP_OPT $AUX_PARAMS $SERVER_IP"
		if [ "$LOG_CMDS" != "1" ]; then
			$CMD 2>>$LOG_ERR | grep ${BS} | grep -v '[B]' | sed 's/^[ ]*//' \
				| sed 's/[ ]*$//' | sed -r 's/[[:blank:]]+/,/g' >> $OUTPUT
		else
			echo "$CMD" >> $CLIENT
		fi
	done

	if [ "$LOG_CMDS" != "1" ]; then
		CSV_MODE=$(echo ${IB_TOOL} | sed 's/_read//')

		# convert to standardized-CSV
		./csv2standardized.py --csv_type ${CSV_MODE} --output_file $OUTPUT $OUTPUT
	fi

	echo "FINISHED benchmark for MODE=$MODE IP=$SERVER_IP"
	echo
}

SERVER_IP=$1
MODES=$2

case $MODES in
bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat)
	;;
all)
	MODES="bw-bs bw-dp-exp bw-dp-lin bw-th lat"
	;;
*)
	usage "Wrong mode: $MODES"
	;;
esac

for m in $MODES; do
	benchmark_one $SERVER_IP $m
done
