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
	echo "usage: $0 <read|write> <bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat> <server_ip>"
	echo
	echo "export JOB_NUMA=0"
	echo "export FIO_PATH=/custom/fio/path"
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_FIO_PATH=/custom/fio/path"
	echo "export REMOTE_JOB_PATH=/custom/jobs/path"
	echo "export REMOTE_JOB_MEM=mmap:/path/to/mem"
	exit 1
}

if [ "$#" -lt 3 ]; then
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

OP=$1
MODE=$2
SERVER_IP=$3

case $OP in
read|write)
	;;
*)
	usage "Wrong operation: $OP"
	;;
esac

case $MODE in
bw-bs)
	THREADS=1
	BLOCK_SIZE=(256 1024 4096 8192 65536)
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
*)
	usage "Wrong mode: $MODE"
	;;
esac

if [ -z "$REMOTE_JOB_MEM" ]; then
	MEM=dram
else
	MEM="$( echo "$REMOTE_JOB_MEM" | cut -c11- )"
fi

NAME=rpma_fio_${OP}_${MODE}_${MEM}-${TIMESTAMP}
DIR=/dev/shm
TEMP_JSON=${DIR}/${NAME}_temp.json
TEMP_CSV=${DIR}/${NAME}_temp.csv
OUTPUT=${NAME}.csv
LOG_ERR=${DIR}/${NAME}.log
SUFFIX=$(echo $MODE | cut -d'-' -f1)

echo "Performance results: $OUTPUT"
echo "Output and errors (both sides): $LOG_ERR"
echo "This tool is EXPERIMENTAL"
echo

rm -f $LOG_ERR $OUTPUT

if [ -z "$REMOTE_JOB_PATH" ]; then
	REMOTE_JOB_PATH=${DIR}/librpma-server-${TIMESTAMP}.fio
fi
if [ -z "$REMOTE_JOB_MEM" ]; then
	REMOTE_JOB_MEM=malloc
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
	sshpass -p "$REMOTE_PASS" scp ./fio_jobs/librpma-server.fio \
		$REMOTE_USER@$SERVER_IP:$REMOTE_JOB_PATH
	# run the server
	sshpass -p "$REMOTE_PASS" -v ssh $REMOTE_USER@$SERVER_IP \
		"bindname=$SERVER_IP mem=$REMOTE_JOB_MEM num_conns=${TH} \
		numactl -N $REMOTE_JOB_NUMA \
			${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH > $LOG_ERR" 2>>$LOG_ERR &
	sleep 1

	echo "[op: $OP, size: $BS, threads: $TH, iodepth: $DP]"
	# run FIO
	hostname=$SERVER_IP blocksize=$BS numjobs=$TH iodepth=${DP} readwrite=${OP} \
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
