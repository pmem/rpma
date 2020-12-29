#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# rpma_fio_bench.sh -- a single-sided Fio RPMA benchmark (EXPERIMENTAL)
#
# Spawns both server and client, collects the results for multiple data
# sizes (1KiB, 4KiB, 64KiB) and generates a single CSV file with all results.
#

TIMESTAMP=$(date +%y-%m-%d-%H%M%S)

function usage()
{
	echo "Error: $1"
	echo
	echo "usage: $0 <all|apm|gpspm> <all|read|write> <all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat> <server_ip>"
	echo
	echo "export JOB_NUMA=0"
	echo "export FIO_PATH=/custom/fio/path"
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_FIO_PATH=/custom/fio/path"
	echo "export REMOTE_JOB_PATH=/custom/jobs/path"
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem (required in case of the GPSPM mode)"
	exit 1
}

if [ "$#" -lt 4 ]; then
	usage "Too few arguments"
elif [ -z "$JOB_NUMA" ]; then
	usage "JOB_NUMA not set"
elif [ -z "$REMOTE_USER" ]; then
	usage "REMOTE_USER not set"
elif [ -z "$REMOTE_PASS" ]; then
	usage "REMOTE_PASS not set"
elif [ -z "$REMOTE_JOB_NUMA" ]; then
	usage "REMOTE_JOB_NUMA not set"
elif [ -z "$REMOTE_JOB_MEM_PATH" -a "$1" == "gpspm" ]; then
	usage "REMOTE_JOB_MEM_PATH not set"
fi

function benchmark_one() {

	P_MODE=$1 # persistency mode
	OP=$2
	MODE=$3
	SERVER_IP=$4

	case $P_MODE in
	apm)
		GPSPM=""
		if [ -n "$REMOTE_JOB_MEM_PATH" ]; then
			REMOTE_JOB_DEST="mem=mmap:$REMOTE_JOB_MEM_PATH"
		else
			REMOTE_JOB_DEST="mem=malloc"
		fi
		;;
	gpspm)
		GPSPM="_gpspm"
		REMOTE_JOB_DEST="filename=$REMOTE_JOB_MEM_PATH"
		;;
	esac

	case $MODE in
	bw-bs)
		THREADS=1
		BLOCK_SIZE=(256 512 1024 2048 4096 8192 16384 24576 32768 65536)
		ITERATIONS=${#BLOCK_SIZE[@]}
		DEPTH=2
		;;
	bw-dp-exp)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 4 8 16 32 64 128)
		ITERATIONS=${#DEPTH[@]}
		;;
	bw-dp-lin)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 3 4 5 6 7 8 9 10)
		ITERATIONS=${#DEPTH[@]}
		;;
	bw-th)
		THREADS=(1 2 4 8 12 16)
		BLOCK_SIZE=4096
		DEPTH=2
		ITERATIONS=${#THREADS[@]}
		;;
	lat)
		THREADS=1
		BLOCK_SIZE=(1024 4096 65536)
		DEPTH=1
		ITERATIONS=${#BLOCK_SIZE[@]}
		;;
	esac

	DEST="$(echo $REMOTE_JOB_DEST | cut -d'=' -f2- | cut -d'/' -f2- | sed 's/\//_/g')"
	[ "$DEST" == "malloc" ] && DEST="dram"

	NAME=rpma_fio_${P_MODE}_${OP}_${MODE}_${DEST}-${TIMESTAMP}
	DIR=/dev/shm
	TEMP_JSON=${DIR}/${NAME}_temp.json
	TEMP_CSV=${DIR}/${NAME}_temp.csv
	OUTPUT=${NAME}.csv
	LOG_ERR=${DIR}/${NAME}.log
	SUFFIX=$(echo $MODE | cut -d'-' -f1)

	echo "STARTING benchmark for P_MODE=$P_MODE OP=$OP MODE=$MODE IP=$SERVER_IP ..."
	echo "Performance results: $OUTPUT"
	echo "Output and errors (both sides): $LOG_ERR"
	echo "This tool is EXPERIMENTAL"
	echo

	rm -f $LOG_ERR $OUTPUT

	if [ -z "$REMOTE_JOB_PATH" ]; then
		REMOTE_JOB_PATH=${DIR}/librpma${GPSPM}-server-${TIMESTAMP}.fio
	fi

	for i in $(seq 0 $(expr $ITERATIONS - 1)); do
		case $MODE in
		bw-bs)
			BS="${BLOCK_SIZE[${i}]}"
			TH="${THREADS}"
			DP="${DEPTH}"
			;;
		bw-dp-exp|bw-dp-lin)
			BS="${BLOCK_SIZE}"
			TH="${THREADS}"
			DP="${DEPTH[${i}]}"
			;;
		bw-th)
			BS="${BLOCK_SIZE}"
			TH="${THREADS[${i}]}"
			DP="${DEPTH}"
			;;
		lat)
			BS="${BLOCK_SIZE[${i}]}"
			TH="${THREADS}"
			DP="${DEPTH}"
			;;
		esac

		# copy config to the server
		sshpass -p "$REMOTE_PASS" scp -o StrictHostKeyChecking=no \
			./fio_jobs/librpma${GPSPM}-server.fio \
			$REMOTE_USER@$SERVER_IP:$REMOTE_JOB_PATH
		# run the server
		sshpass -p "$REMOTE_PASS" -v ssh $REMOTE_USER@$SERVER_IP \
			"bindname=$SERVER_IP num_conns=${TH} ${REMOTE_JOB_DEST} \
			numactl -N $REMOTE_JOB_NUMA \
				${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH > $LOG_ERR" 2>>$LOG_ERR &
		sleep 1

		echo "[mode: $P_MODE, op: $OP, size: $BS, threads: $TH, iodepth: $DP]"
		# run FIO
		hostname=$SERVER_IP blocksize=$BS numjobs=$TH iodepth=${DP} readwrite=${OP} \
			ioengine=librpma${GPSPM}_client \
			numactl -N $JOB_NUMA ${FIO_PATH}fio \
			./fio_jobs/librpma-client-${SUFFIX}.fio --output-format=json+ \
			> $TEMP_JSON
		if [ "$?" -ne 0 ]; then
			echo "Error: FIO job failed"
			exit 1
		fi
		# convert JSON to CSV
		./fio_json2csv.py $TEMP_JSON --output_file $TEMP_CSV \
			--op ${OP}
		# append CSV to the output
		cat $TEMP_CSV >> $OUTPUT
	done

	# remove redundant headers
	cat $OUTPUT | head -1 > $TEMP_CSV
	cat $OUTPUT | grep -v 'lat' >> $TEMP_CSV
	cp $TEMP_CSV $OUTPUT

	# convert to standardized-CSV
	./csv2standardized.py --csv_type fio --output_file $OUTPUT $OUTPUT

	echo "FINISHED benchmark for P_MODE=$P_MODE OP=$OP MODE=$MODE IP=$SERVER_IP"
	echo
}

P_MODES=$1 # persistency mode
OPS=$2
MODES=$3
SERVER_IP=$4

case $P_MODES in
apm|gpspm)
	;;
all)
	P_MODES="apm gpspm"
	;;
*)
	usage "Wrong persistency mode: $P_MODES"
	;;
esac

case $OPS in
read|write)
	;;
all)
	OPS="read write"
	;;
*)
	usage "Wrong operation: $OPS"
	;;
esac

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

for p in $P_MODES; do
	for o in $OPS; do
		for m in $MODES; do
			benchmark_one $p $o $m $SERVER_IP
		done
	done
done
