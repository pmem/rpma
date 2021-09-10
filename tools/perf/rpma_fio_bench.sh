#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
# Copyright 2021, Fujitsu
#

#
# rpma_fio_bench.sh -- a single-sided Fio RPMA benchmark (EXPERIMENTAL)
#
# Spawns both server and client, collects the results for multiple data
# sizes (1KiB, 4KiB, 64KiB) and generates a single CSV file with all results.
#

source $(dirname $0)/bench_common.sh

if [ "$1" == "--env" ]; then
	show_environment
fi

if [ "$#" -lt 2 ] || [ "$#" -eq 2 -a "$2" != "all" ]; then
	usage "Too few arguments"
elif [ "$2" == "gpspm" ]; then
	case "$3" in
	read|randread|rw|randrw)
		usage "The 'gpspm' mode does not support the '$3' operation for now."
		;;
	esac
elif [ "$2" == "aof_sw" -o "$2" == "aof_hw" ]; then
	case "$3" in
	read|randread|randwrite|rw|randrw)
		usage "The '$2' mode does not support the '$3' operation."
		;;
	esac
fi

check_env

if [ -z "$BUSY_WAIT_POLLING" ]; then
	BUSY_WAIT_POLLING=1
fi

if [ "$BUSY_WAIT_POLLING" == "1" ]; then
	POLLING="busy-wait"
else
	POLLING="no-busy-wait"
fi

CPU_LOAD_00_99="0 25 50 75 99"
CPU_LOAD_75_99="75 80 85 90 95 99"
CPU_LOAD_RANGE=${CPU_LOAD_RANGE-00_99} # the default value
CPU_LOAD_RANGE_VALUES="CPU_LOAD_$CPU_LOAD_RANGE"

function benchmark_one() {

	SERVER_IP=$1
	PERSIST_MODE=$2 # persistency mode
	OP=$3
	MODE=$4
	if [ -n "$COMMENT" ]; then
		COMMENT="__$COMMENT""__"
	fi

	if [ -n "$REMOTE_JOB_MEM_PATH" ]; then
		REMOTE_JOB_DEST="$REMOTE_JOB_MEM_PATH"
	else
		REMOTE_JOB_DEST="malloc"
	fi

	case $MODE in
	bw-bs)
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536 131072 262144)
		ITERATIONS=${#BLOCK_SIZE[@]}
		DEPTH=2
		NAME_SUFFIX=th${THREADS}_dp${DEPTH}
		SYNC=0
		;;
	bw-dp-exp)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 4 8 16 32 64 128)
		ITERATIONS=${#DEPTH[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}
		SYNC=0
		;;
	bw-dp-lin)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=(1 2 3 4 5 6 7 8 9 10)
		ITERATIONS=${#DEPTH[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}
		SYNC=0
		;;
	bw-th)
		THREADS=(1 2 4 8 12 16 32 64)
		BLOCK_SIZE=4096
		DEPTH=2
		ITERATIONS=${#THREADS[@]}
		NAME_SUFFIX=bs${BLOCK_SIZE}_dp${DEPTH}
		SYNC=0
		;;
	bw-cpu)
		THREADS=1
		BLOCK_SIZE=65536
		DEPTH=2
		CPU_LOAD=(${!CPU_LOAD_RANGE_VALUES})
		ITERATIONS=${#CPU_LOAD[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}_dp${DEPTH}_cpu${CPU_LOAD_RANGE}
		SYNC=0
		;;
	bw-cpu-mt)
		THREADS=$CORES_PER_SOCKET
		BLOCK_SIZE=4096
		DEPTH=2
		CPU_LOAD=(${!CPU_LOAD_RANGE_VALUES})
		ITERATIONS=${#CPU_LOAD[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}_dp${DEPTH}_cpu${CPU_LOAD_RANGE}
		SYNC=0
		;;
	lat)
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536 131072 262144)
		DEPTH=1
		ITERATIONS=${#BLOCK_SIZE[@]}
		NAME_SUFFIX=th${THREADS}_dp${DEPTH}
		SYNC=1
		;;
	lat-cpu)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=1
		CPU_LOAD=(${!CPU_LOAD_RANGE_VALUES})
		ITERATIONS=${#CPU_LOAD[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}_dp${DEPTH}_cpu${CPU_LOAD_RANGE}
		SYNC=1
		;;
	esac

	DEST="$(echo $REMOTE_JOB_DEST | cut -d'/' -f2- | sed 's/\//_/g')"
	[ "$DEST" == "malloc" ] && DEST="dram"

	NAME=rpma_fio_${PERSIST_MODE}_${POLLING}_${OP}_${MODE}_${NAME_SUFFIX}_${DEST}${COMMENT}-${TIMESTAMP}
	if [ "$DO_RUN" == 1 ]; then
		DIR=/dev/shm
		TEMP_JSON=${DIR}/${NAME}_temp.json
		TEMP_CSV=${DIR}/${NAME}_temp.csv
		LOG_ERR=${DIR}/${NAME}-errors.log
	elif [ "$DUMP_CMDS" == "1" ]; then
		SERVER_DUMP=${NAME}-server.log
		CLIENT_DUMP=${NAME}-client.log
		LOG_ERR=${NAME}-errors.log
		echo "Log commands [server]: $SERVER_DUMP"
		echo "Log commands [client]: $CLIENT_DUMP"
	fi
	SUFFIX=$(echo $MODE | cut -d'-' -f1)

	local RW_OPS=(read write)
	# set indexes (INDS) for arrays: RW_OPS and OUTPUT
	case $OP in
	read|randread)
		INDS=0 # read
		;;
	write|randwrite)
		INDS=1 # write
		;;
	rw|randrw)
		INDS="0 1" # read write
		if [ -n "$OUTPUT_FILE" ]; then
			# Mixed workloads have to be combined into a single output file.
			COMBINE=1
		fi
		;;
	esac

	if ! which ${FIO_PATH}fio > /dev/null; then
		echo "Error: wrong path to the 'fio' tool - \"${FIO_PATH}fio\" does not exist"
		exit 1
	fi

	if ! sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no $REMOTE_USER@$SERVER_IP \
			"which ${REMOTE_FIO_PATH}fio >/dev/null 2>$LOG_ERR" 2>>$LOG_ERR;
	then
		echo "Error: wrong remote path to the 'fio' tool - \"${REMOTE_FIO_PATH}fio\" does not exist"
		exit 1
	fi

	echo "STARTING benchmark for PERSIST_MODE=$PERSIST_MODE OP=$OP MODE=$MODE IP=$SERVER_IP ..."

	if [ "$DUMP_CMDS" != "1" ]; then
		case $OP in
		read|randread|write|randwrite)
			OUTPUT=(${NAME}.csv ${NAME}.csv) # the same names
			;;
		rw|randrw)
			OUTPUT=(${NAME}_${RW_OPS[0]}.csv ${NAME}_${RW_OPS[1]}.csv)
			;;
		esac

		echo "Output and errors (both sides): $LOG_ERR"
		for i in $INDS; do
			echo "Performance results of ${RW_OPS[i]}s: ${OUTPUT[i]}"
			rm -f ${OUTPUT[i]}
		done
		rm -f $LOG_ERR

		if [ -z "$REMOTE_JOB_PATH" ]; then
			REMOTE_JOB_PATH=${DIR}/librpma_${PERSIST_MODE}-server-${TIMESTAMP}.fio
		fi
	fi

	case $PERSIST_MODE in
	apm|aof_hw)
		REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=1
		;;
	gpspm|aof_sw)
		REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=0
		;;
	esac

	set_ddio /dev/shm

	for i in $(seq 0 $(expr $ITERATIONS - 1)); do
		case $MODE in
		bw-bs)
			BS="${BLOCK_SIZE[${i}]}"
			TH="${THREADS}"
			DP="${DEPTH}"
			CPU=0
			ITER=bs${BS}
			;;
		bw-dp-exp|bw-dp-lin)
			BS="${BLOCK_SIZE}"
			TH="${THREADS}"
			DP="${DEPTH[${i}]}"
			CPU=0
			ITER=dp${DP}
			;;
		bw-th)
			BS="${BLOCK_SIZE}"
			TH="${THREADS[${i}]}"
			DP="${DEPTH}"
			CPU=0
			ITER=th${TH}
			;;
		bw-cpu|bw-cpu-mt)
			BS="${BLOCK_SIZE}"
			TH="${THREADS}"
			DP="${DEPTH}"
			CPU="${CPU_LOAD[${i}]}"
			ITER=cpu${CPU}
			;;
		lat)
			BS="${BLOCK_SIZE[${i}]}"
			TH="${THREADS}"
			DP="${DEPTH}"
			CPU=0
			ITER=bs${BS}
			;;
		lat-cpu)
			BS="${BLOCK_SIZE}"
			TH="${THREADS}"
			DP="${DEPTH}"
			CPU="${CPU_LOAD[${i}]}"
			ITER=cpu${CPU}
			;;
		esac

		if [ "$CPU" == "0" ]; then
			FILTER="--section server" # no CPU load
		else
			FILTER="" # no section filtering so CPU load will be included
		fi

		prepare_RUN_NAME_and_CMP__SUBST

		ENV="serverip=$SERVER_IP numjobs=${TH} iodepth=${DP} \
			direct_write_to_pmem=${REMOTE_DIRECT_WRITE_TO_PMEM} \
			busy_wait_polling=${BUSY_WAIT_POLLING} cpuload=${CPU} \
			cores_per_socket=${CORES_PER_SOCKET}"

		# pick either a DRAM/DeviceDAX or a FileSystemDAX mode
		case "$REMOTE_JOB_DEST" in
			malloc)
				# create_on_open prevents FIO from creating files
				# where the engines won't make use of them anyways
				# since they are using DRAM instead
				FILE_NAME="--filename=${REMOTE_JOB_DEST} --create_on_open=1"
				;;
			/dev/dax*)
				FILE_NAME="--filename=${REMOTE_JOB_DEST}"
				;;
			*)
				FILE_NAME="--filename_format=${REMOTE_JOB_DEST}.\\\$jobnum"
				;;
		esac

		SERVER_COMMAND="$ENV $REMOTE_TRACER ${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH $FILE_NAME $FILTER >> $LOG_ERR 2>&1"

		if [ "$DO_RUN" == "1" ]; then
			remote_command --pre

			# copy config to the server
			sshpass -p "$REMOTE_PASS" scp -o StrictHostKeyChecking=no \
				./fio_jobs/librpma_${PERSIST_MODE}-server.fio \
				$REMOTE_USER@$SERVER_IP:$REMOTE_JOB_PATH 2>>$LOG_ERR
			# run the server
			if [ "x$REMOTE_TRACER" == "x" ]; then
				REMOTE_TRACER="numactl -N $REMOTE_JOB_NUMA"
			fi
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$SERVER_COMMAND" 2>>$LOG_ERR &
		elif [ "$DUMP_CMDS" == "1" ]; then
			echo "Remote command:" >> $SERVER_DUMP
			echo "$ $SERVER_COMMAND" >> $SERVER_DUMP
			echo >> $SERVER_DUMP
			echo "Fio job file (./fio_jobs/librpma_${PERSIST_MODE}-server.fio):" >> $SERVER_DUMP
			bash -c "cat ./fio_jobs/librpma_${PERSIST_MODE}-server.fio | \
				grep -v '^#' | $ENV envsubst >> $SERVER_DUMP"
			echo "---" >> $SERVER_DUMP
		fi

		echo "[mode: $PERSIST_MODE, op: $OP, size: $BS, threads: $TH, iodepth: $DP, sync: $SYNC, cpuload: $CPU]"
		ENV="serverip=$SERVER_IP blocksize=$BS sync=$SYNC numjobs=$TH iodepth=${DP} \
			readwrite=${OP} ramp_time=$RAMP_TIME runtime=$RUNTIME"
		if [ "x$TRACER" == "x" ]; then
			TRACER="numactl -N $JOB_NUMA"
		fi

		CLIENT_COMMAND="$ENV $TRACER ${FIO_PATH}fio ./fio_jobs/librpma_${PERSIST_MODE}-client.fio \
				--output-format=json+ > $TEMP_JSON"

		if [ "$DO_RUN" == "1" ]; then
			# run FIO
			bash -c "$CLIENT_COMMAND"
			if [ "$?" -ne 0 ]; then
				echo "Error: FIO job failed"
				exit 1
			fi

			remote_command --post

			for i in $INDS; do
				rm -f $TEMP_CSV
				# convert JSON to CSV
				./fio_json2csv.py $TEMP_JSON --output_file $TEMP_CSV \
					--op ${RW_OPS[i]} --extra "{cpuload: ${CPU-0}}"
				# append CSV to the output
				cat $TEMP_CSV >> ${OUTPUT[i]}
			done
		elif [ "$DO_NOTHING" == "1" ]; then
			for i in $INDS; do
				touch ${OUTPUT[i]}
			done
		elif [ "$DUMP_CMDS" == "1" ]; then
			echo "Command:" >> $CLIENT_DUMP
			echo "$ $CLIENT_COMMAND" >> $CLIENT_DUMP
			echo >> $CLIENT_DUMP
			echo "Fio job file (./fio_jobs/librpma_${PERSIST_MODE}-client.fio):" >> $CLIENT_DUMP
			bash -c "cat ./fio_jobs/librpma_${PERSIST_MODE}-client.fio | \
				grep -v '^#' | $ENV envsubst >> $CLIENT_DUMP"
			echo "---" >> $SERVER_DUMP
			echo "---" >> $CLIENT_DUMP
		fi
	done

	if [ "$DO_RUN" == "1" ]; then
		for i in $INDS; do
			# remove redundant headers
			cat ${OUTPUT[i]} | head -1 > $TEMP_CSV
			cat ${OUTPUT[i]} | grep -v 'lat' >> $TEMP_CSV
			mv $TEMP_CSV ${OUTPUT[i]}
			# Mixed workloads have to be processed together
			# in order to be combined into a single output file.
			[ "x$COMBINE" == "x1" ] && continue
			# prepare output file name
			output_file=${OUTPUT_FILE-${OUTPUT[i]}}
			if [ "$output_file" == "${OUTPUT[i]}" ]; then
				# save the input *.csv file in case of an error
				mv ${OUTPUT[i]} ${OUTPUT[i]}_error.csv
				OUTPUT[i]=${OUTPUT[i]}_error.csv
			fi
			# convert to standardized-CSV/JSON
			./csv2standardized.py --csv_type fio --output_file $output_file \
				${OUTPUT[i]}

			[ $? -eq 0 ] && rm -f ${OUTPUT[i]}
		done

		# Combine all outputs into a single file.
		if [ "x$COMBINE" == "x1" ]; then
			./csv2standardized.py --csv_type fio --output_file $OUTPUT_FILE \
				${OUTPUT[@]} --keys ${RW_OPS[@]}
			[ $? -eq 0 ] && rm -f ${OUTPUT[@]}
		fi
	fi

	echo "FINISHED benchmark for PERSIST_MODE=$PERSIST_MODE OP=$OP MODE=$MODE IP=$SERVER_IP"
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

if [ "$DUMP_CMDS" != "1" -a "$DO_NOTHING" != "1" ]; then
	DO_RUN="1"
else
	DO_RUN="0"
fi

echo

case $P_MODES in
apm|gpspm|aof_sw|aof_hw)
	;;
all)
	P_MODES="apm gpspm aof_sw aof_hw"
	;;
*)
	usage "Wrong persistency mode: $P_MODES"
	;;
esac

case $OPS in
read|randread|write|randwrite|rw|randrw)
	;;
all)
	OPS="read randread write randwrite rw randrw"
	;;
*)
	usage "Wrong operation: $OPS"
	;;
esac

case $MODES in
bw-bs|bw-dp-exp|bw-dp-lin|bw-th|bw-cpu|bw-cpu-mt|lat|lat-cpu)
	;;
all)
	MODES="lat lat-cpu bw-bs bw-cpu bw-cpu-mt bw-th bw-dp-lin bw-dp-exp"
	;;
*)
	usage "Wrong mode: $MODES"
	;;
esac

# get cpulist of the remote NUMA node (REMOTE_JOB_NUMA)
export REMOTE_JOB_NUMA_CPULIST=$( \
	sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
	$REMOTE_USER@$SERVER_IP \
	"cat /sys/devices/system/node/node${REMOTE_JOB_NUMA}/cpulist")
# validate the output
[[ "$REMOTE_JOB_NUMA_CPULIST" =~ ^[,0-9\\-]+$ ]]
if [ -z "${BASH_REMATCH[0]}" ]; then
	echo "Obtained remote cpulist for the provided \
		REMOTE_JOB_NUMA=$REMOTE_JOB_NUMA is invalid: $REMOTE_JOB_NUMA_CPULIST"
	exit 1
fi

export CORES_PER_SOCKET=$( \
	sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
	$REMOTE_USER@$SERVER_IP \
	"lscpu | egrep 'Core\(s\) per socket:' | sed 's/[^0-9]*//g'")
# validate the output
[[ "$CORES_PER_SOCKET" =~ ^[0-9]+$ ]]
if [ -z "${BASH_REMATCH[0]}" ]; then
	echo "Invalid value: CORES_PER_SOCKET=$CORES_PER_SOCKET"
	exit 1
fi

for p in $P_MODES; do
	for o in $OPS; do
		if [ "$p" == "gpspm" ]; then
			case "$o" in
			read|randread|rw|randrw)
				continue
				;;
			esac
		elif [ "$p" == "aof_sw" -o "$p" == "aof_hw" ]; then
			case "$o" in
			read|randread|randwrite|rw|randrw)
				continue
				;;
			esac
		fi

		for m in $MODES; do
			benchmark_one $SERVER_IP $p $o $m
		done
	done
done
