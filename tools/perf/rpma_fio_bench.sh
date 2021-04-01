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
	echo "Usage: $0 <server_ip> all|apm|gpspm|aof_sw|aof_hw [all|read|randread|write|randwrite|rw|randrw] [all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|bw-cpu|bw-cpu-mt|lat|lat-cpu]"
	echo "       $0 --env - show environment variables used by the script"
	echo
	echo "Notes:"
	echo " - 'all' is the default value for missing arguments"
	echo " - the 'gpspm' mode does not support the 'read' operation for now."
	echo " - the 'aof_*' modes do not support the 'read', 'randread', 'randwrite', 'rw' and 'randrw' operations."
	echo
	./common.sh --usage
	exit 1
}

if [ "$1" == "--env" ]; then
	./common.sh --env
	exit 0
fi

if [ "$#" -lt 2 ] || [ "$#" -eq 2 -a "$2" != "all" ]; then
	usage "Too few arguments"
elif [ -z "$JOB_NUMA" ]; then
	usage "JOB_NUMA not set"
elif [ -z "$REMOTE_USER" ]; then
	usage "REMOTE_USER not set"
elif [ -z "$REMOTE_PASS" ]; then
	usage "REMOTE_PASS not set"
elif [ -z "$REMOTE_JOB_NUMA" ]; then
	usage "REMOTE_JOB_NUMA not set"
elif [ -z "$REMOTE_RNIC_PCIE_ROOT_PORT" -a "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
	usage "REMOTE_RNIC_PCIE_ROOT_PORT not set"
elif [ -z "$REMOTE_DIRECT_WRITE_TO_PMEM" -a "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
	usage "REMOTE_DIRECT_WRITE_TO_PMEM not set"
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

./common.sh --check_parameters

if [ -z "$BUSY_WAIT_POLLING" ]; then
	BUSY_WAIT_POLLING=1
fi

if [ "$BUSY_WAIT_POLLING" == "1" ]; then
	POLLING="busy-wait"
else
	POLLING="no-busy-wait"
fi

function benchmark_one() {

	SERVER_IP=$1
	PERSIST_MODE=$2 # persistency mode
	OP=$3
	MODE=$4
	if [ -n "$COMMENT" ]; then
		COMMENT="__$COMMENT""__"
	fi

	case $PERSIST_MODE in
	apm|aof_hw)
		./common.sh --ddio_off
		;;
	gpspm|aof_sw)
		./common.sh --ddio_on
		;;
	esac

	if [ -n "$REMOTE_JOB_MEM_PATH" ]; then
		REMOTE_JOB_DEST="$REMOTE_JOB_MEM_PATH"
	else
		REMOTE_JOB_DEST="malloc"
	fi

	case $MODE in
	bw-bs)
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
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
		CPU_LOAD=(0 25 50 75 100)
		ITERATIONS=${#CPU_LOAD[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}_dp${DEPTH}
		SYNC=0
		;;
	bw-cpu-mt)
		THREADS=$CORES_PER_SOCKET
		BLOCK_SIZE=4096
		DEPTH=2
		CPU_LOAD=(0 25 50 75 100)
		ITERATIONS=${#CPU_LOAD[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}_dp${DEPTH}
		SYNC=0
		;;
	lat)
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
		DEPTH=1
		ITERATIONS=${#BLOCK_SIZE[@]}
		NAME_SUFFIX=th${THREADS}_dp${DEPTH}
		SYNC=1
		;;
	lat-cpu)
		THREADS=1
		BLOCK_SIZE=4096
		DEPTH=1
		CPU_LOAD=(0 25 50 75 100)
		ITERATIONS=${#CPU_LOAD[@]}
		NAME_SUFFIX=th${THREADS}_bs${BLOCK_SIZE}_dp${DEPTH}
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
		LOG_ERR=${DIR}/${NAME}.log
	elif [ "$DUMP_CMDS" == "1" ]; then
		SERVER_DUMP=${NAME}-server.log
		CLIENT_DUMP=${NAME}-client.log
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
		;;
	esac

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

	./common.sh --set_ddio

	if [ "$DUMP_CMDS" == "1" ]; then
		echo "REMOTE_DIRECT_WRITE_TO_PMEM=$REMOTE_DIRECT_WRITE_TO_PMEM" >> $SERVER_DUMP
		echo >> $SERVER_DUMP
	fi

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

		export RUN_NAME=${NAME}_${ITER}
		echo "Name of this run: ${RUN_NAME}"

		REMOTE_CMD_PRE_SUBST=$(echo "$REMOTE_CMD_PRE" | envsubst)
		REMOTE_CMD_POST_SUBST=$(echo "$REMOTE_CMD_POST" | envsubst)

		ENV="serverip=$SERVER_IP numjobs=${TH} iodepth=${DP} \
			filename=${REMOTE_JOB_DEST} \
			direct_write_to_pmem=${REMOTE_DIRECT_WRITE_TO_PMEM} \
			busy_wait_polling=${BUSY_WAIT_POLLING} cpuload=${CPU} \
			cores_per_socket=${CORES_PER_SOCKET}"
		if [ "$DO_RUN" == "1" ]; then
			if [ "x$REMOTE_CMD_PRE_SUBST" != "x" ]; then
				echo "$REMOTE_CMD_PRE_SUBST"
				sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
					$REMOTE_USER@$SERVER_IP "$REMOTE_CMD_PRE_SUBST" 2>>$LOG_ERR &
			fi

			# copy config to the server
			sshpass -p "$REMOTE_PASS" scp -o StrictHostKeyChecking=no \
				./fio_jobs/librpma_${PERSIST_MODE}-server.fio \
				$REMOTE_USER@$SERVER_IP:$REMOTE_JOB_PATH 2>>$LOG_ERR
			# run the server
			if [ "x$REMOTE_TRACER" == "x" ]; then
				REMOTE_TRACER="numactl -N $REMOTE_JOB_NUMA"
			fi
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$ENV $REMOTE_TRACER \
				${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH $FILTER >> $LOG_ERR 2>&1" 2>>$LOG_ERR &
		elif [ "$DUMP_CMDS" == "1" ]; then
			bash -c "cat ./fio_jobs/librpma_${PERSIST_MODE}-server.fio | \
				grep -v '^#' | $ENV envsubst >> $SERVER_DUMP"
		fi

		echo "[mode: $PERSIST_MODE, op: $OP, size: $BS, threads: $TH, iodepth: $DP, sync: $SYNC, cpuload: $CPU]"
		ENV="serverip=$SERVER_IP blocksize=$BS sync=$SYNC numjobs=$TH iodepth=${DP} \
			readwrite=${OP} ramp_time=$RAMP_TIME runtime=$RUNTIME"
		if [ "$DO_RUN" == "1" ]; then
			# run FIO
			if [ "x$TRACER" == "x" ]; then
				TRACER="numactl -N $JOB_NUMA"
			fi
			bash -c "$ENV $TRACER ${FIO_PATH}fio ./fio_jobs/librpma_${PERSIST_MODE}-client.fio \
				--output-format=json+ > $TEMP_JSON"
			if [ "$?" -ne 0 ]; then
				echo "Error: FIO job failed"
				exit 1
			fi

			if [ "x$REMOTE_CMD_POST_SUBST" != "x" ]; then
				echo "$REMOTE_CMD_POST_SUBST"
				sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
					$REMOTE_USER@$SERVER_IP "$REMOTE_CMD_POST_SUBST" 2>>$LOG_ERR
			fi

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
			# convert to standardized-CSV
			./csv2standardized.py --csv_type fio --output_file ${OUTPUT[i]} ${OUTPUT[i]}
		done
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
