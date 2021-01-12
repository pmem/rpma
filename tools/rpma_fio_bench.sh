#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
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
	echo "Usage: $0 <server_ip> all|apm|gpspm [all|read|write|rw|randrw] [all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat]"
	echo "Notes:"
	echo " - 'all' is the default value for missing arguments"
	echo " - the 'gpspm' mode does not support the 'read' operation for now."
	echo
	echo "export JOB_NUMA=0"
	echo "export FIO_PATH=/custom/fio/path/"
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_FIO_PATH=/custom/fio/path/"
	echo "export REMOTE_JOB_PATH=/custom/jobs/path"
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem (required in case of the GPSPM mode)"
	echo "export COMMENT=any_text_to_be_added_to_every_file_name"
	echo
	echo "Debug:"
	echo "export SHORT_RUNTIME=0 (adequate for functional verification only)"
	echo "export TRACER='gdbserver localhost:2345'"
	echo "export REMOTE_TRACER='gdbserver localhost:2345'"
	echo
	exit 1
}

if [ "$#" -lt 2 ] || [ "$#" -eq 2 -a "$2" != "all" ]; then
	usage "Too few arguments"
elif [ "$2" == "gpspm" -a "$3" == "read" ]; then
	usage "The 'gpspm' mode does not support the 'read' operation for now."
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

	SERVER_IP=$1
	P_MODE=$2 # persistency mode
	OP=$3
	MODE=$4
	if [ -n "$COMMENT" ]; then
		COMMENT="__$COMMENT""__"
	fi

	# the 'gpspm' mode does not support the 'read' operation for now
	if [ "$P_MODE" == "gpspm" -a "$OP" == "read" ]; then
		echo "Notice: SKIPPING the unsupported 'gpspm-read' combination."
		return
	fi

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
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
		ITERATIONS=${#BLOCK_SIZE[@]}
		DEPTH=2
		NAME_SUFFIX=th${THREADS}_dp${DEPTH}
		;;
	bw-dp-exp)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 4 8 16 32 64 128)
		ITERATIONS=${#DEPTH[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}
		;;
	bw-dp-lin)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 3 4 5 6 7 8 9 10)
		ITERATIONS=${#DEPTH[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}
		;;
	bw-th)
		THREADS=(1 2 4 8 12 16)
		BLOCK_SIZE=4096
		DEPTH=2
		ITERATIONS=${#THREADS[@]}
		NAME_SUFFIX=bs${BLOCK_SIZE}_dp${DEPTH}
		;;
	lat)
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
		DEPTH=1
		ITERATIONS=${#BLOCK_SIZE[@]}
		NAME_SUFFIX=th${THREADS}_dp${DEPTH}
		;;
	esac

	DEST="$(echo $REMOTE_JOB_DEST | cut -d'=' -f2- | cut -d'/' -f2- | sed 's/\//_/g')"
	[ "$DEST" == "malloc" ] && DEST="dram"

	NAME=rpma_fio_${P_MODE}_${OP}_${MODE}_${NAME_SUFFIX}_${DEST}${COMMENT}-${TIMESTAMP}
	DIR=/dev/shm
	TEMP_JSON=${DIR}/${NAME}_temp.json
	TEMP_CSV=${DIR}/${NAME}_temp.csv
	LOG_ERR=${DIR}/${NAME}.log
	SUFFIX=$(echo $MODE | cut -d'-' -f1)

	OPS=(read write)
	# set indexes (INDS) for arrays: OPS and OUTPUT
	case $OP in
	read)
		INDS=0 # read
		;;
	write)
		INDS=1 # write
		;;
	rw|randrw)
		INDS="0 1" # read write
		;;
	esac
	case $OP in
	read|write)
		OUTPUT=(${NAME}.csv ${NAME}.csv) # the same names
		;;
	rw|randrw)
		OUTPUT=(${NAME}_${OPS[0]}.csv ${NAME}_${OPS[1]}.csv)
		;;
	esac

	echo "STARTING benchmark for P_MODE=$P_MODE OP=$OP MODE=$MODE IP=$SERVER_IP ..."
	echo "Output and errors (both sides): $LOG_ERR"
	for i in $INDS; do
		echo "Performance results of ${OPS[i]}s: ${OUTPUT[i]}"
		rm -f ${OUTPUT[i]}
	done
	rm -f $LOG_ERR

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
		if [ "x$REMOTE_TRACER" == "x" ]; then
			REMOTE_TRACER="numactl -N $REMOTE_JOB_NUMA"
		fi
		sshpass -p "$REMOTE_PASS" -v ssh $REMOTE_USER@$SERVER_IP \
			"bindname=$SERVER_IP num_conns=${TH} iodepth=${DP} ${REMOTE_JOB_DEST} \
			$REMOTE_TRACER \
				${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH > $LOG_ERR 2>&1" 2>>$LOG_ERR &
		sleep 1

		echo "[mode: $P_MODE, op: $OP, size: $BS, threads: $TH, iodepth: $DP]"
		# run FIO
		if [ "x$TRACER" == "x" ]; then
			TRACER="numactl -N $JOB_NUMA"
		fi
		hostname=$SERVER_IP blocksize=$BS numjobs=$TH iodepth=${DP} readwrite=${OP} \
			ramp_time=$RAMP_TIME runtime=$RUNTIME ioengine=librpma${GPSPM}_client \
			$TRACER ${FIO_PATH}fio \
			./fio_jobs/librpma-client-${SUFFIX}.fio --output-format=json+ \
			> $TEMP_JSON
		if [ "$?" -ne 0 ]; then
			echo "Error: FIO job failed"
			exit 1
		fi

		for i in $INDS; do
			rm -f $TEMP_CSV
			# convert JSON to CSV
			./fio_json2csv.py $TEMP_JSON --output_file $TEMP_CSV --op ${OPS[i]}
			# append CSV to the output
			cat $TEMP_CSV >> ${OUTPUT[i]}
		done
	done

	for i in $INDS; do
		# remove redundant headers
		cat ${OUTPUT[i]} | head -1 > $TEMP_CSV
		cat ${OUTPUT[i]} | grep -v 'lat' >> $TEMP_CSV
		mv $TEMP_CSV ${OUTPUT[i]}
		# convert to standardized-CSV
		./csv2standardized.py --csv_type fio --output_file ${OUTPUT[i]} ${OUTPUT[i]}
	done

	echo "FINISHED benchmark for P_MODE=$P_MODE OP=$OP MODE=$MODE IP=$SERVER_IP"
	echo
}

echo "This tool is EXPERIMENTAL"

SERVER_IP=$1
P_MODES=$2 # persistency mode
[ -n "$3" ] && OPS=$3 || OPS="all"
[ -n "$4" ] && MODES=$4 || MODES="all"

if [ "$SHORT_RUNTIME" == "1" ]; then
	RAMP_TIME=0
	RUNTIME=10

	echo "Notice: The results may be inaccurate (SHORT_RUNTIME=1)"
else
	RAMP_TIME=15
	RUNTIME=60
fi

echo

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
read|write|rw|randrw)
	;;
all)
	OPS="read write rw randrw"
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
			benchmark_one $SERVER_IP $p $o $m $5
		done
	done
done
