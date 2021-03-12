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
	echo "Usage: $0 <server_ip> all|apm|gpspm|aof_sw|aof_hw [all|read|randread|write|randwrite|rw|randrw] [all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat]"
	echo "       $0 --env - show environment variables used by the script"
	echo
	echo "Notes:"
	echo " - 'all' is the default value for missing arguments"
	echo " - the 'gpspm' mode does not support the 'read' operation for now."
	echo " - the 'aof_*' modes do not support the 'read', 'randread', 'randwrite', 'rw' and 'randrw' operations."
	echo
	echo "export JOB_NUMA=0"
	echo "export FIO_PATH=/custom/fio/path/"
	echo "export COMMENT=any_text_to_be_added_to_every_file_name"
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_SUDO_NOPASSWD=0/1"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html)"
	echo "export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (forces setting REMOTE_DIRECT_WRITE_TO_PMEM to this value)"
	echo "export REMOTE_FIO_PATH=/custom/fio/path/"
	echo "export REMOTE_JOB_PATH=/custom/jobs/path"
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem"
	echo
	echo "export REMOTE_ANOTHER_NUMA=1"
	echo "export REMOTE_CMD_PRE='rm -f sar.dat; numactl -N \${REMOTE_ANOTHER_NUMA} sar -u -P \${REMOTE_JOB_NUMA_CPULIST} -o sar.dat 5 > /dev/null'"
	echo "export REMOTE_CMD_POST='sleep 10; killall -9 sar; sadf -d -- -u -P \${REMOTE_JOB_NUMA_CPULIST} sar.dat > sar.csv'"
	echo
	echo "Debug:"
	echo "export SHORT_RUNTIME=0 (adequate for functional verification only)"
	echo "export TRACER='gdbserver localhost:2345'"
	echo "export REMOTE_TRACER='gdbserver localhost:2345'"
	echo "export DUMP_CMDS=1 (dump all commands that would be executed; do not run the actual execution)"
	echo
	exit 1
}

function show_environment() {
	echo
	echo "Environment variables used by the script:"
	echo
	echo "export JOB_NUMA=$JOB_NUMA"
	echo "export FIO_PATH=$FIO_PATH"
	echo "export COMMENT=$COMMENT"
	echo
	echo "export REMOTE_USER=$REMOTE_USER"
	echo "export REMOTE_PASS=$REMOTE_PASS"
	echo "export REMOTE_JOB_NUMA=$REMOTE_JOB_NUMA"
	echo "export REMOTE_SUDO_NOPASSWD=$REMOTE_SUDO_NOPASSWD"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=$REMOTE_RNIC_PCIE_ROOT_PORT"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=$REMOTE_DIRECT_WRITE_TO_PMEM"
	echo "export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM"
	echo "export REMOTE_FIO_PATH=$REMOTE_FIO_PATH"
	echo "export REMOTE_JOB_PATH=$REMOTE_JOB_PATH"
	echo "export REMOTE_JOB_MEM_PATH=$REMOTE_JOB_MEM_PATH"
	echo
	echo "export REMOTE_ANOTHER_NUMA=$REMOTE_ANOTHER_NUMA"
	echo "export REMOTE_CMD_PRE='$REMOTE_CMD_PRE'"
	echo "export REMOTE_CMD_POST='$REMOTE_CMD_POST'"
	echo
	echo "Debug:"
	echo "export SHORT_RUNTIME=$SHORT_RUNTIME"
	echo "export TRACER=$TRACER"
	echo "export REMOTE_TRACER=$REMOTE_TRACER"
	echo "export DUMP_CMDS=$DUMP_CMDS"
	exit 0
}

if [ "$1" == "--env" ]; then
	show_environment
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

if [ "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
	echo "WARNING: sudo (called on the remote side) will prompt for password!"
	echo "         Toggling DDIO will be skipped!"
	echo
	echo "         In order to change it:"
	echo "           1) set permissions of sudo to NOPASSWD in '/etc/sudoers' and"
	echo "           2) set REMOTE_SUDO_NOPASSWD=1"
	echo
	if [ -n "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" ]; then
		echo "Error: FORCE_REMOTE_DIRECT_WRITE_TO_PMEM is set, but REMOTE_SUDO_NOPASSWD does not equal 1."
		echo "       Change sudo permissions in order to force setting REMOTE_DIRECT_WRITE_TO_PMEM."
		echo "Exiting..."
		exit 1
	fi
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
		REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=1
		if [ -z "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" ] || \
		   [ $FORCE_REMOTE_DIRECT_WRITE_TO_PMEM -eq $REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM ]; then
			DDIO_MODE="disable"
			DDIO_QUERY=0
		else
			DDIO_MODE="enable"
			DDIO_QUERY=1
			REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM
		fi
		;;
	gpspm|aof_sw)
		REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=0
		if [ -z "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" ] || \
		   [ $FORCE_REMOTE_DIRECT_WRITE_TO_PMEM -eq $REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM ]; then
			DDIO_MODE="enable"
			DDIO_QUERY=1
		else
			DDIO_MODE="disable"
			DDIO_QUERY=0
			REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM
		fi
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
	lat)
		THREADS=1
		BLOCK_SIZE=(256 1024 4096 8192 16384 32768 65536)
		DEPTH=1
		ITERATIONS=${#BLOCK_SIZE[@]}
		NAME_SUFFIX=th${THREADS}_dp${DEPTH}
		SYNC=1
		;;
	esac

	DEST="$(echo $REMOTE_JOB_DEST | cut -d'/' -f2- | sed 's/\//_/g')"
	[ "$DEST" == "malloc" ] && DEST="dram"

	NAME=rpma_fio_${PERSIST_MODE}_${OP}_${MODE}_${NAME_SUFFIX}_${DEST}${COMMENT}-${TIMESTAMP}
	if [ "$DUMP_CMDS" != "1" ]; then
		DIR=/dev/shm
		TEMP_JSON=${DIR}/${NAME}_temp.json
		TEMP_CSV=${DIR}/${NAME}_temp.csv
		LOG_ERR=${DIR}/${NAME}.log
	else
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

	if [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		if [ "$DUMP_CMDS" != "1" ]; then
			# copy the ddio.sh script to the server
			sshpass -p "$REMOTE_PASS" scp -o StrictHostKeyChecking=no \
				../ddio.sh $REMOTE_USER@$SERVER_IP:$DIR
			# set DDIO on the server
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP \
				"sudo $DIR/ddio.sh -d $REMOTE_RNIC_PCIE_ROOT_PORT -s $DDIO_MODE \
				> $LOG_ERR 2>&1" 2>>$LOG_ERR
			# query DDIO on the server
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP \
				"sudo $DIR/ddio.sh -d $REMOTE_RNIC_PCIE_ROOT_PORT -q \
				> $LOG_ERR 2>&1" 2>>$LOG_ERR
			if [ $? -ne $DDIO_QUERY ]; then
				echo "Error: setting DDIO to '$DDIO_MODE' failed"
				exit 1
			fi
		fi
		REMOTE_DIRECT_WRITE_TO_PMEM=$REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM
	elif [ $REMOTE_DIRECT_WRITE_TO_PMEM -ne $REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM ]; then
		echo "Error: REMOTE_DIRECT_WRITE_TO_PMEM does not have the required value ($REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM)"
		echo "Skipping..."
		return
	fi

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

		ENV="serverip=$SERVER_IP numjobs=${TH} iodepth=${DP} \
			filename=${REMOTE_JOB_DEST} \
			direct_write_to_pmem=${REMOTE_DIRECT_WRITE_TO_PMEM}"
		if [ "$DUMP_CMDS" != "1" ]; then
			if [ "x$REMOTE_CMD_PRE" != "x" ]; then
				echo "$REMOTE_CMD_PRE"
				sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
					$REMOTE_USER@$SERVER_IP "$REMOTE_CMD_PRE" 2>>$LOG_ERR &
			fi

			# copy config to the server
			sshpass -p "$REMOTE_PASS" scp -o StrictHostKeyChecking=no \
				./fio_jobs/librpma_${PERSIST_MODE}-server.fio \
				$REMOTE_USER@$SERVER_IP:$REMOTE_JOB_PATH
			# run the server
			if [ "x$REMOTE_TRACER" == "x" ]; then
				REMOTE_TRACER="numactl -N $REMOTE_JOB_NUMA"
			fi
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$ENV $REMOTE_TRACER \
				${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH >> $LOG_ERR 2>&1" 2>>$LOG_ERR &
		else
			bash -c "cat ./fio_jobs/librpma_${PERSIST_MODE}-server.fio | \
				grep -v '^#' | $ENV envsubst >> $SERVER_DUMP"
		fi

		echo "[mode: $PERSIST_MODE, op: $OP, size: $BS, threads: $TH, iodepth: $DP sync: $SYNC]"
		ENV="serverip=$SERVER_IP blocksize=$BS sync=$SYNC numjobs=$TH iodepth=${DP} \
			readwrite=${OP} ramp_time=$RAMP_TIME runtime=$RUNTIME"
		if [ "$DUMP_CMDS" != "1" ]; then
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

			if [ "x$REMOTE_CMD_POST" != "x" ]; then
				echo "$REMOTE_CMD_POST"
				sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
					$REMOTE_USER@$SERVER_IP "$REMOTE_CMD_POST" 2>>$LOG_ERR
			fi

			for i in $INDS; do
				rm -f $TEMP_CSV
				# convert JSON to CSV
				./fio_json2csv.py $TEMP_JSON --output_file $TEMP_CSV --op ${RW_OPS[i]}
				# append CSV to the output
				cat $TEMP_CSV >> ${OUTPUT[i]}
			done
		else
			bash -c "cat ./fio_jobs/librpma_${PERSIST_MODE}-client.fio | \
				grep -v '^#' | $ENV envsubst >> $CLIENT_DUMP"
			echo "---" >> $SERVER_DUMP
			echo "---" >> $CLIENT_DUMP
		fi
	done

	if [ "$DUMP_CMDS" != "1" ]; then
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
bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat)
	;;
all)
	MODES="lat bw-bs bw-th bw-dp-lin bw-dp-exp"
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

REMOTE_CMD_PRE=$(echo "$REMOTE_CMD_PRE" | envsubst)
REMOTE_CMD_POST=$(echo "$REMOTE_CMD_POST" | envsubst)

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
