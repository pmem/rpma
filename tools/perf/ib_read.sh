#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
# Copyright 2021, Fujitsu
#

#
# ib_read.sh -- a single-sided ib_read_lat/bw tool (EXPERIMENTAL)
#
# Spawns both server and the client, collects the results for multiple data
# sizes (1KiB, 4KiB, 64KiB) and generates a single CSV file with all
# the resutls.
#

source $(dirname $0)/bench_common.sh

HEADER_LAT="#bytes #iterations    t_min[usec]    t_max[usec]  t_typical[usec]    t_avg[usec]    t_stdev[usec]   99% percentile[usec]   99.9% percentile[usec]"
HEADER_BW="#threads #bytes     #iterations    BW_peak[Gb/sec]    BW_average[Gb/sec]   MsgRate[Mpps]"

echo "This tool is EXPERIMENTAL"

if [ "$1" == "--env" ]; then
	show_environment
fi

if [ "$#" -lt 2 ]; then
	usage "Too few arguments"
fi

check_env

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
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536 131072 262144)
		DEPTH=2
		# values measured empirically, so that duration was ~60s
		# 100000000 is the maximum value of iterations
		ITERATIONS=(48336720 48336720 34951167 24475088 23630690 8299603 5001135 4800000 2600000)
		local AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}_${THREADS}th"
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
		local AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}_${BLOCK_SIZE}bs"
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
		local AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}_${BLOCK_SIZE}bs"
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
		local AUX_PARAMS="$AUX_PARAMS --report_gbits"
		NAME="${MODE}_${BLOCK_SIZE}bs"
		verify_threads
		;;
	lat)
		IB_TOOL=ib_read_lat
		HEADER=$HEADER_LAT
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536 131072 262144)
		DEPTH=1
		# values measured empirically, so that duration was ~60s
		ITERATIONS=(27678723 27678723 20255739 16778088 11423082 8138946 6002473 3600000 2100000)
		local AUX_PARAMS="$AUX_PARAMS --perform_warm_up"
		NAME="${MODE}"
		verify_block_size
		;;
	*)
		usage "Wrong mode: $MODE"
		;;
	esac

	NAME=ib_read_${NAME}_${TIMESTAMP}
	echo "STARTING benchmark for MODE=$MODE IP=$SERVER_IP ..."
	if [ "$DUMP_CMDS" != "1" ]; then
		OUTPUT=${NAME}.csv
		LOG_ERR=/dev/shm/${NAME}-errors.log
		echo "Performance results: $OUTPUT"
		echo "Output and errors (both sides): $LOG_ERR"
	elif [ "$DUMP_CMDS" == "1" ]; then
		SERVER_DUMP=${NAME}-server.log
		CLIENT_DUMP=${NAME}-client.log
		LOG_ERR=${NAME}-errors.log
		echo "Log commands [server]: $SERVER_DUMP"
		echo "Log commands [client]: $CLIENT_DUMP"
	fi
	echo

	# By default, Direct Write to PMem is impossible due to DDIO=on
	# which forces writing to cache instead of PMem directly.
	# This test requires restoring the default settings.
	REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=0

	set_ddio /dev/shm

	if [ "$DO_RUN" == "1" ]; then
		rm -f $LOG_ERR
		echo "$HEADER" | sed 's/% /%_/g' | sed -r 's/[[:blank:]]+/,/g' > $OUTPUT
	elif [ "$DO_NOTHING" == "1" ]; then
		touch $OUTPUT
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
			ITER=bs${BS}
			[ "$DO_RUN" == "1" ] && echo -n "${TH},${DP}," >> $OUTPUT
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
			ITER=dp${DP}
			[ "$DO_RUN" == "1" ] && echo -n "${TH},${DP}," >> $OUTPUT
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
			ITER=th${TH}
			[ "$DO_RUN" == "1" ] && echo -n "${TH},${DP}," >> $OUTPUT
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
			ITER=bs${BS}
			;;
		esac

		if [ "$SHORT_RUNTIME" == "1" ]; then
			IT=100
			IT_OPT="--iters $IT"
		fi

		prepare_RUN_NAME_and_CMP__SUBST

		if ! which ${IB_PATH}${IB_TOOL} > /dev/null; then
			echo "Error: wrong path (IB_PATH) to the \"${IB_TOOL}\" tool - " \
					"\"${IB_PATH}${IB_TOOL}\" does not exist"
			exit 1
		fi

		if ! sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no $REMOTE_USER@$SERVER_IP \
				"which ${REMOTE_IB_PATH}${IB_TOOL} >/dev/null 2>$LOG_ERR" 2>>$LOG_ERR;
		then
			echo "Error: wrong remote path (REMOTE_IB_PATH) to the \"${IB_TOOL}\" tool - " \
					"\"${REMOTE_IB_PATH}${IB_TOOL}\" does not exist"
			exit 1
		fi

		# run the server
		CMD="numactl -N $REMOTE_JOB_NUMA ${REMOTE_IB_PATH}${IB_TOOL} $BS_OPT $QP_OPT \
			$DP_OPT $REMOTE_AUX_PARAMS"

		if [ "$DO_RUN" == "1" ]; then
			remote_command --pre

			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$CMD >> $LOG_ERR" 2>>$LOG_ERR &
			sleep 1
		elif [ "$DUMP_CMDS" == "1" ]; then
			echo "$CMD" >> $SERVER_DUMP
		fi

		# XXX --duration hides detailed statistics
		echo "[size: ${BS}, threads: ${TH}, tx_depth: ${DP}, iters: ${IT}] (duration: ~60s)"
		CMD="numactl -N $JOB_NUMA ${IB_PATH}${IB_TOOL} $IT_OPT $BS_OPT \
			$QP_OPT $DP_OPT $AUX_PARAMS $SERVER_IP"
		if [ "$DO_RUN" == "1" ]; then
			$CMD 2>>$LOG_ERR | grep ${BS} | grep -v '[B]' | sed 's/^[ ]*//' \
				| sed 's/[ ]*$//' | sed -r 's/[[:blank:]]+/,/g' >> $OUTPUT

			remote_command --post

		elif [ "$DUMP_CMDS" == "1" ]; then
			echo "$CMD" >> $CLIENT_DUMP
		fi
	done

	if [ "$DO_RUN" == "1" ]; then
		CSV_MODE=$(echo ${IB_TOOL} | sed 's/_read//')

		# prepare output file name
		output_file=${OUTPUT_FILE-$OUTPUT}
		if [ "$output_file" == "$OUTPUT" ]; then
			# save the input *.csv file in case of an error
			mv ${OUTPUT} ${OUTPUT}_error.csv
			OUTPUT=${OUTPUT}_error.csv
		fi

		# convert to standardized-CSV/JSON
		./csv2standardized.py --csv_type ${CSV_MODE} \
			--output_file $output_file $OUTPUT

		[ $? -eq 0 ] && rm -f $OUTPUT
	fi

	echo "FINISHED benchmark for MODE=$MODE IP=$SERVER_IP"
	echo
}

SERVER_IP=$1
MODES=$2

if [ "$DUMP_CMDS" != "1" -a "$DO_NOTHING" != "1" ]; then
	DO_RUN="1"
else
	DO_RUN="0"
fi

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
