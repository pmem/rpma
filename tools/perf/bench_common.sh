#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# bench_common.sh -- a single-sided ib_read_lat/bw and Fio RPMA tool (EXPERIMENTAL)
#

function show_environment() {
	echo
	echo "Environment variables used by the script:"
	echo
	echo "export JOB_NUMA=$JOB_NUMA"
	echo "export AUX_PARAMS=$AUX_PARAMS"
	echo "export IB_PATH=$IB_PATH"
	echo "export FIO_PATH=$FIO_PATH"
	echo "export COMMENT=$COMMENT"
	echo
	echo "export REMOTE_USER=$REMOTE_USER"
	echo "export REMOTE_PASS=$REMOTE_PASS"
	echo "export REMOTE_JOB_NUMA=$REMOTE_JOB_NUMA"
	echo "export REMOTE_AUX_PARAMS=$REMOTE_AUX_PARAMS"
	echo "export REMOTE_SUDO_NOPASSWD=$REMOTE_SUDO_NOPASSWD"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=$REMOTE_RNIC_PCIE_ROOT_PORT"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=$REMOTE_DIRECT_WRITE_TO_PMEM"
	echo "export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM"
	echo "export REMOTE_IB_PATH=$REMOTE_IB_PATH"
	echo "export REMOTE_FIO_PATH=$REMOTE_FIO_PATH"
	echo "export REMOTE_JOB_PATH=$REMOTE_JOB_PATH"
	echo "export REMOTE_JOB_MEM_PATH=$REMOTE_JOB_MEM_PATH"
	echo "export BUSY_WAIT_POLLING=$BUSY_WAIT_POLLING"
	echo
	echo "export REMOTE_ANOTHER_NUMA=$REMOTE_ANOTHER_NUMA"
	echo "export REMOTE_CMD_PRE='$REMOTE_CMD_PRE'"
	echo "export REMOTE_CMD_POST='$REMOTE_CMD_POST'"
	echo
	echo "Debug:"
	echo "export SHORT_RUNTIME=$SHORT_RUNTIME"
	echo "export TRACER=$TRACER"
	echo "export REMOTE_TRACER=$REMOTE_TRACER"
	echo "export DO_NOTHING=$DO_NOTHING"
	echo "export DUMP_CMDS=$DUMP_CMDS"
	exit 0
}

function ib_usage()
{
	echo "Error: $1"
	echo
	echo "usage: $0 <server_ip> <all|bw-bs|bw-dp-exp|bw-dp-lin|bw-th|lat>"
	echo "       $0 --env - show environment variables used by the script"
	echo
	common_usage
	exit 1
}

function rpma_usage()
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
	common_usage
	exit 1
}

function common_usage()
{
	echo "export JOB_NUMA=0"
	echo "export AUX_PARAMS='-d mlx5_0 -R'"
	echo "export IB_PATH=/custom/ib tool/path/"
	echo "export FIO_PATH=/custom/fio/path/"
	echo "export COMMENT=any_text_to_be_added_to_every_file_name"
	echo
	echo "export REMOTE_USER=user"
	echo "export REMOTE_PASS=pass"
	echo "export REMOTE_JOB_NUMA=0"
	echo "export REMOTE_AUX_PARAMS='-d mlx5_0 -R'"
	echo "export REMOTE_IB_PATH=/custom/ib tool/path/"
	echo "export REMOTE_SUDO_NOPASSWD=0/1"
	echo "export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>"
	echo "export REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html)"
	echo "export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (forces setting REMOTE_DIRECT_WRITE_TO_PMEM to this value)"
	echo "export REMOTE_FIO_PATH=/custom/fio/path/"
	echo "export REMOTE_JOB_PATH=/custom/jobs/path"
	echo "export REMOTE_JOB_MEM_PATH=/path/to/mem"
	echo "export BUSY_WAIT_POLLING=0/1"
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
	echo "export TRACER='gdbserver localhost:2345'"
	echo "export REMOTE_TRACER='gdbserver localhost:2345'"
	echo "export DO_NOTHING=1 (create empty output files; do not run the actual execution)"
	echo "export DUMP_CMDS=1 (dump all commands that would be executed; do not run the actual execution)"
}

function check_parameters()
{
	if [ "$0" = "./ib_read.sh" ]; then
		USAGE="ib_usage"
	elif [ "$0" = "./rpma_fio_bench.sh" ]; then
		USAGE="rpma_usage"
	fi

	if [ -z "$JOB_NUMA" ]; then
		$USAGE "JOB_NUMA not set"
	elif [ -z "$REMOTE_USER" ]; then
		$USAGE "REMOTE_USER not set"
	elif [ -z "$REMOTE_PASS" ]; then
		$USAGE "REMOTE_PASS not set"
	elif [ -z "$REMOTE_JOB_NUMA" ]; then
		$USAGE "REMOTE_JOB_NUMA not set"
	elif [ -z "$REMOTE_RNIC_PCIE_ROOT_PORT" -a "$REMOTE_SUDO_NOPASSWD" == "1" ]; then
		$USAGE "REMOTE_RNIC_PCIE_ROOT_PORT not set"
	elif [ -z "$REMOTE_DIRECT_WRITE_TO_PMEM" -a "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
		$USAGE "REMOTE_DIRECT_WRITE_TO_PMEM not set"
	fi

	if [ "$REMOTE_SUDO_NOPASSWD" != "1" ]; then
		echo "WARNING: sudo (called on the remote side) will prompt for password!"
		echo "         Toggling DDIO will be skipped!"
		echo
		echo "         In order to change it:"
		echo "           1) set permissions of sudo to NOPASSWD in '/etc/sudoers' and"
		echo "           2) set REMOTE_SUDO_NOPASSWD=1"
		echo
		if [ -n "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" -a "$FORCE_REMOTE_DIRECT_WRITE_TO_PMEM" != "$REMOTE_DIRECT_WRITE_TO_PMEM" ]; then
			echo "Error: FORCE_REMOTE_DIRECT_WRITE_TO_PMEM != REMOTE_DIRECT_WRITE_TO_PMEM ($FORCE_REMOTE_DIRECT_WRITE_TO_PMEM != $REMOTE_DIRECT_WRITE_TO_PMEM),"
			echo "       and REMOTE_SUDO_NOPASSWD does not equal 1."
			echo "       Change sudo permissions in order to force setting REMOTE_DIRECT_WRITE_TO_PMEM."
			echo "Exiting..."
			exit 1
		fi
	fi
}

function ddio_off()
{
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
}

function ddio_on()
{
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
}

function set_ddio()
{
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
		return
	fi
}

function prepare_variables()
{
	export RUN_NAME=${NAME}_${ITER}
	echo "Name of this run: ${RUN_NAME}"

	REMOTE_CMD_PRE_SUBST=$(echo "$REMOTE_CMD_PRE" | envsubst)
	REMOTE_CMD_POST_SUBST=$(echo "$REMOTE_CMD_POST" | envsubst)
}

function collect_statistics()
{
	if [ "$1" = "--start" ]; then
		REMOTE_CMD=$REMOTE_CMD_PRE_SUBST
	elif [ "$1" = "--stop" ]; then
		REMOTE_CMD=$REMOTE_CMD_POST_SUBST
	fi

	echo "$REMOTE_CMD"
	sshpass -p "$REMOTE_PASS" -v ssh -o StrictHostKeyChecking=no \
		$REMOTE_USER@$SERVER_IP "$REMOTE_CMD" 2>>$LOG_ERR &
}
