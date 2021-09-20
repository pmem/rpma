#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
# Copyright 2021, Fujitsu
#

#
# bench_common.sh -- a single-sided ib_read_lat/bw
#                    and Fio tools common (EXPERIMENTAL)

TIMESTAMP=$(date +%y-%m-%d-%H%M%S)
FILENAME=$(basename -- $0)

if [ "$FILENAME" != "rpma_fio_bench.sh" -a "$FILENAME" != "ib_read.sh" ]; then
	echo "The script $FILENAME is not supported"
	exit 1
fi

function show_environment() {
	echo
	echo "Environment variables used by the script:"
	echo
	echo "export JOB_NUMA=$JOB_NUMA"
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "export FIO_PATH=$FIO_PATH"
		echo "export COMMENT=$COMMENT"
	else
		echo "export AUX_PARAMS=$AUX_PARAMS"
		echo "export IB_PATH=$IB_PATH"
	fi
	echo
	echo "export REMOTE_USER=$REMOTE_USER"
	echo "export REMOTE_PASS=$REMOTE_PASS"
	echo "export REMOTE_JOB_NUMA=$REMOTE_JOB_NUMA"
	echo "export REMOTE_SUDO_NOPASSWD=$REMOTE_SUDO_NOPASSWD"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=$REMOTE_RNIC_PCIE_ROOT_PORT"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=$REMOTE_DIRECT_WRITE_TO_PMEM"
	echo "export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM"
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "export REMOTE_FIO_PATH=$REMOTE_FIO_PATH"
		echo "export REMOTE_JOB_PATH=$REMOTE_JOB_PATH"
		echo "export REMOTE_JOB_MEM_PATH=$REMOTE_JOB_MEM_PATH"
		echo "export BUSY_WAIT_POLLING=$BUSY_WAIT_POLLING"
		echo "export CPU_LOAD_RANGE=$CPU_LOAD_RANGE"
	else
		echo "export REMOTE_AUX_PARAMS=$REMOTE_AUX_PARAMS"
		echo "export REMOTE_IB_PATH=$REMOTE_IB_PATH"
	fi
	echo
	echo "export REMOTE_ANOTHER_NUMA=$REMOTE_ANOTHER_NUMA"
	echo "export REMOTE_CMD_PRE='$REMOTE_CMD_PRE'"
	echo "export REMOTE_CMD_POST='$REMOTE_CMD_POST'"
	echo
	echo "Debug:"
	echo "export SHORT_RUNTIME=$SHORT_RUNTIME"
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "export TRACER=$TRACER"
		echo "export REMOTE_TRACER=$REMOTE_TRACER"
	fi
	echo "export DO_NOTHING=$DO_NOTHING"
	echo "export DUMP_CMDS=$DUMP_CMDS"
	echo
	echo "export OUTPUT_FILE=$OUTPUT_FILE"
	exit 0
}

function usage()
{
	echo "Error: $1"
	echo
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "Usage: $0 <server_ip> all|apm|gpspm|aof_sw|aof_hw [all|read|randread|write|randwrite|rw|randrw] [all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|bw-cpu|bw-cpu-mt|lat|lat-cpu]"
		echo "       $0 --env - show environment variables used by the script"
		echo
		echo "Notes:"
		echo " - 'all' is the default value for missing arguments"
		echo " - the 'gpspm' mode does not support the 'read' operation for now."
		echo " - the 'aof_*' modes do not support the 'read', 'randread', 'randwrite', 'rw' and 'randrw' operations."
	else
		echo "usage: $0 <server_ip> <all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat>"
		echo "       $0 --env - show environment variables used by the script"
	fi
	echo
	echo "export JOB_NUMA=0"
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "export FIO_PATH=/custom/fio/path/"
		echo "export COMMENT=any_text_to_be_added_to_every_file_name"
	else
		echo "export AUX_PARAMS='-d mlx5_0 -R'"
		echo "export IB_PATH=/custom/ib tool/path/"
	fi
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_SUDO_NOPASSWD=0/1"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html)"
	echo "export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (forces setting REMOTE_DIRECT_WRITE_TO_PMEM to this value)"
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "export REMOTE_FIO_PATH=/custom/fio/path/"
		echo "export REMOTE_JOB_PATH=/custom/jobs/path"
		echo "export REMOTE_JOB_MEM_PATH=/path/to/mem"
		echo "export BUSY_WAIT_POLLING=0/1"
		echo "export CPU_LOAD_RANGE=00_99/75_99"
	else
		echo "export REMOTE_AUX_PARAMS='-d mlx5_0 -R'"
		echo "export REMOTE_IB_PATH=/custom/ib tool/path/"
	fi
	echo
	echo "export REMOTE_ANOTHER_NUMA=1"
	echo "export REMOTE_RESULTS_DIR=/tmp/"
	echo
	echo "In case you would like to collect sar data:"
	echo "export REMOTE_CMD_PRE='rm -f \${REMOTE_RESULTS_DIR}sar.dat; numactl -N \${REMOTE_ANOTHER_NUMA} sar -u -P \${REMOTE_JOB_NUMA_CPULIST} -o \${REMOTE_RESULTS_DIR}sar.dat 5 > /dev/null'"
	echo "export REMOTE_CMD_POST='sleep 10; killall -9 sar; sadf -d -- -u -P \${REMOTE_JOB_NUMA_CPULIST} \${REMOTE_RESULTS_DIR}sar.dat > \${REMOTE_RESULTS_DIR}sar_\${RUN_NAME}.csv'"
	echo
	echo "In case you would like to collect emon data:"
	echo "export EVENT_LIST=/path/to/edp/events/list"
	echo "export REMOTE_CMD_PRE='source /opt/intel/sep/sep_vars.sh; numactl -N \${REMOTE_ANOTHER_NUMA} emon -i \${EVENT_LIST} > \${REMOTE_RESULTS_DIR}\${RUN_NAME}_emon.dat'"
	echo "export REMOTE_CMD_POST='sleep 10; source /opt/intel/sep/sep_vars.sh; emon -stop'"
	echo
	echo "Note:"
	echo "The 'REMOTE_CMD_PRE' and 'REMOTE_CMD_POST' environment variables"
	echo "can use the 'RUN_NAME' environment variable internally,"
	echo "which contains a unique name of each run."
	echo
	echo "Debug:"
	echo "export SHORT_RUNTIME=0 (adequate for functional verification only)"
	if [ "$FILENAME" = "rpma_fio_bench.sh" ]; then
		echo "export TRACER='gdbserver localhost:2345'"
		echo "export REMOTE_TRACER='gdbserver localhost:2345'"
	fi
	echo "export DO_NOTHING=1 (create empty output files; do not run the actual execution)"
	echo "export DUMP_CMDS=1 (dump all commands that would be executed; do not run the actual execution)"
	echo
	echo "export OUTPUT_FILE=output_file.csv (or .json)"
	exit 1
}

function check_env()
{
	if [ -z "$JOB_NUMA" ]; then
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
	fi

	if [ "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
		echo "WARNING: sudo (called on the remote side) will prompt for password!"
		echo "         Toggling DDIO will be skipped!"
		echo
		echo "         In order to change it:"
		echo "           1) set permissions of sudo to NOPASSWD in '/etc/sudoers' and"
		echo "           2) set REMOTE_SUDO_NOPASSWD=1"
		echo
		if [ -n "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" -a \
		   "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" != "$REMOTE_DIRECT_WRITE_TO_PMEM" ]; then
			echo "Error: FORCE_REMOTE_DIRECT_WRITE_TO_PMEM != REMOTE_DIRECT_WRITE_TO_PMEM
			($FORCE_REMOTE_DIRECT_WRITE_TO_PMEM != $REMOTE_DIRECT_WRITE_TO_PMEM),"
			echo "       and REMOTE_SUDO_NOPASSWD does not equal 1."
			echo "       Change sudo permissions in order to force setting REMOTE_DIRECT_WRITE_TO_PMEM."
			echo "Exiting..."
			exit 1
		fi
	fi
}

function set_ddio()
{
	DIR=$1

	if [ "$DIR" == "" ]; then
		echo "Error: the remote path for the 'ddio.sh' script is not set!"
		exit 1
	fi

	if [ ! -f ../ddio.sh ]; then
		echo "Error: ../ddio.sh script does not exist!"
		exit 1
	fi

	if [ -n "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" ] && \
	   [ $FORCE_REMOTE_DIRECT_WRITE_TO_PMEM -ne $REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM ]; then
		REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM
	fi

	case "$REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM" in
	0)
		DDIO_MODE="enable"
		DDIO_QUERY=1
		;;
	1)
		DDIO_MODE="disable"
		DDIO_QUERY=0
		;;
	esac

	if [ "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		if [ "$DO_RUN" == "1" ]; then
			# copy the ddio.sh script to the server
			sshpass -p "$REMOTE_PASS" scp -o StrictHostKeyChecking=no \
				../ddio.sh $REMOTE_USER@$SERVER_IP:$DIR 2>>$LOG_ERR
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
		exit 0
	fi

	if [ "$DUMP_CMDS" == "1" ]; then
		echo "REMOTE_DIRECT_WRITE_TO_PMEM=$REMOTE_DIRECT_WRITE_TO_PMEM" >> $SERVER_DUMP
		echo >> $SERVER_DUMP
	fi
}

function prepare_RUN_NAME_and_CMP__SUBST()
{
	export RUN_NAME=${NAME}_${ITER}
	echo "Name of this run: ${RUN_NAME}"

	REMOTE_CMD_PRE_SUBST=$(echo "$REMOTE_CMD_PRE" | envsubst)
	REMOTE_CMD_POST_SUBST=$(echo "$REMOTE_CMD_POST" | envsubst)
}

function remote_command()
{
	case "$1" in
	--pre)
		if [ "x$REMOTE_CMD_PRE_SUBST" != "x" ]; then
			echo "$REMOTE_CMD_PRE_SUBST"
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$REMOTE_CMD_PRE_SUBST" 2>>$LOG_ERR &
		fi
		;;
	--post)
		if [ "x$REMOTE_CMD_POST_SUBST" != "x" ]; then
			echo "$REMOTE_CMD_POST_SUBST"
			sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
				$REMOTE_USER@$SERVER_IP "$REMOTE_CMD_POST_SUBST" 2>>$LOG_ERR
		fi
		;;
	esac
}
