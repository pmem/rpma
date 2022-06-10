#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation

#
# run-all-examples.sh - run all examples (optionally under valgrind or with fault injection)
#
# Usage: run-all-examples.sh <binary-examples-directory> [--valgrind|--fault-injection]
#                            [--stop-on-failure] [IP_address] [port]
#
# If RPMA_TEST_PMEM_PATH is set, the examples will be run on the given PMem device.
#

# value used to get the maximum reachable value of fault injection for each example
GET_FI_MAX=999999

# timeout value for both the server and the client
TIMEOUT=3s

BIN_DIR=$1
if [ "$BIN_DIR" == "" ]; then
	echo "Error: missing required argument"
	echo -n "Usage: run-all-examples.sh <binary-examples-directory> "
	echo "[--valgrind|--fault-injection] [--stop-on-failure] [IP_address] [port]"
	exit 1
fi

MODE="none"
if [ "$2" == "--fault-injection" ]; then
	MODE="fault-injection"
	shift
elif [ "$2" == "--valgrind" ]; then
	MODE="valgrind"
	shift
fi

STOP_ON_FAILURE=0
if [ "$2" == "--stop-on-failure" ]; then
	STOP_ON_FAILURE=1
	shift
fi

IP_ADDRESS=$2
PORT=$3
if [ "$IP_ADDRESS" != "" -a "$PORT" == "" ]; then
	PORT="7204"
fi

function print_out_log_file() {
	echo
	echo "*** file $1 (START) ***"
	cat $1
	echo "*** file $1 (END) ***"
	echo
}

function get_max_fault_injection() {
	LOG_FILE=$1
	FI_MAX=$(grep -e '\[#' $LOG_FILE | cut -d'#' -f2 | cut -d']' -f1 | sort -n | tail -n1)
	echo $FI_MAX
}

function error_out_if_no_max_fault_injection() {
	FI_MAX=$1
	LOG_FILE=$2
	if [ "$FI_MAX" == "" ]; then
		print_out_log_file $LOG_FILE
		echo
		echo "Error checking the maximum value of fault injection - exiting ..."
		echo
		exit 1
	fi
}

function run_command_of() {
	WHO=$1
	shift
	if [ "$MODE" != "fault-injection" ]; then
		echo "[${WHO}]$ $*"
		eval $*
	elif [ "$LOG_OUTPUT" == "yes" ]; then
		if [ "$WHO" == "server" ]; then
			rm -f $S_LOG_FILE
			echo "[${WHO}]$ $S_FI $* > $S_LOG_FILE 2>&1"
			eval $S_FI $* > $S_LOG_FILE 2>&1
		else
			rm -f $C_LOG_FILE
			echo "[${WHO}]$ $C_FI $* > $C_LOG_FILE 2>&1"
			eval $C_FI $* > $C_LOG_FILE 2>&1
		fi
	else
		S_TIME=""
		C_TIME=""
		[ "$S_FI" == "" ] && S_TIME="timeout --preserve-status $TIMEOUT" # run the server with timeout
		[ "$C_FI" == "" ] && C_TIME="timeout --preserve-status $TIMEOUT" # run the client with timeout

		if [ "$WHO" == "server" ]; then
			echo "[${WHO}]$ $S_FI $S_TIME $*"
			eval $S_FI $S_TIME $*
		else
			echo "[${WHO}]$ $C_FI $C_TIME $*"
			eval $C_FI $C_TIME $*
		fi
	fi
}

function start_server() {
	echo "Starting the server ..."
	run_command_of server $* &
}

function start_client() {
	echo "Starting the client ..."
	run_command_of client $*
	RV=$?
}

function print_FI_if_failed() {
	if [ $SFAILED -eq 1 -o $CFAILED -eq 1 ]; then
		echo
		echo "=========================================="
		echo "Fault injection ERROR"
		[ $S_FI_VAL -gt 0 ] && \
		echo "Server's fault injection value = $S_FI_VAL"
		[ $C_FI_VAL -gt 0 ] && \
		echo "Client's fault injection value = $C_FI_VAL"
		echo "=========================================="
		echo
		[ $STOP_ON_FAILURE -eq 1 ] && exit 1
	fi
}

function verify_SoftRoCE() {
	SCRIPT_DIR=$(dirname $0)
	$SCRIPT_DIR/../tools/config_softroce.sh verify
	[ $? -ne 0 ] && exit 1

	STATE_OK="state ACTIVE physical_state LINK_UP"

	if [ "$IP_ADDRESS" == "" ]; then
		NETDEV=$(rdma link show | grep -e "$STATE_OK" | head -n1 | cut -d' ' -f8)
		IP_ADDRESS=$(ip address show dev $NETDEV | grep -e inet | grep -v -e inet6 | cut -d' ' -f6 | cut -d/ -f1)
		if [ "$IP_ADDRESS" == "" ]; then
			echo "Error: not found any RDMA-capable network interface"
			exit 1
		fi
	fi

	if [ "$IP_ADDRESS" != "" -a "$PORT" == "" ]; then
		PORT="7204"
	fi

	echo "Notice: running examples for IP address $IP_ADDRESS and port $PORT"
	echo
}

function get_PID_of_server() {
	IP_ADDRESS=$1
	PORT=$2
	ARGS="server $IP_ADDRESS $PORT"
	PID=$(ps aux | grep -e "$ARGS" | grep -v -e "grep -e $ARGS" | awk '{print $2}')
	echo $PID
}

function run_example() {
	DIR=$1
	S_FI_VAL=$2 # server's fault injection value
	C_FI_VAL=$3 # client's fault injection value
	EXAMPLE=$(basename $DIR)

	LOG_OUTPUT="no"
	VLD_SCMD=$VLD_SCMD_ORIG
	VLD_CCMD=$VLD_CCMD_ORIG

	S_FI="" # server's fault injection string
	C_FI="" # client's fault injection string
	if [ "$S_FI_VAL" != "" -a "$C_FI_VAL" != "" ]; then
		[ $S_FI_VAL -ge $GET_FI_MAX -o $C_FI_VAL -ge $GET_FI_MAX ] && LOG_OUTPUT="yes"
		if [ $S_FI_VAL -gt 0 ]; then
			S_FI="RPMA_FAULT_INJECTION=$S_FI_VAL"
			VLD_CCMD="" # do not run the client under valgrind
		else
			S_FI=""
		fi
		if [ $C_FI_VAL -gt 0 ]; then
			C_FI="RPMA_FAULT_INJECTION=$C_FI_VAL"
			VLD_SCMD="" # do not run the server under valgrind
		else
			C_FI=""
		fi
	fi

	SFAILED=0
	CFAILED=0

	echo "*** Running example: $EXAMPLE $VLD_MSG"

	start_server $VLD_SCMD $DIR/server $IP_ADDRESS $PORT $RPMA_TEST_PMEM_PATH
	sleep 1

	RV=0
	case $EXAMPLE in
	06-multiple-connections)
		[ "$MODE" == "fault-injection" ] && SEEDS="8" || SEEDS="8 9 11 12"
		for SEED in $SEEDS; do
			start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT $SEED
			[ $RV -ne 0 ] && break
		done
		;;
	07-atomic-write)
		[ "$MODE" == "fault-injection" ] && WORDS="1st_word" || WORDS="1st_word 2nd_word 3rd_word"
		start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT $WORDS
		;;
	08-messages-ping-pong)
		SEED=7
		[ "$MODE" == "fault-injection" ] && ROUNDS=1 || ROUNDS=3
		start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT $SEED $ROUNDS
		;;
	10-send-with-imm)
		start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT "1234" "1st_word"
		;;
	11-write-with-imm)
		start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT "1234"
		;;
	12-receive-completion-queue)
		START_VALUE=7
		[ "$MODE" == "fault-injection" ] && ROUNDS=1 || ROUNDS=3
		start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT $START_VALUE $ROUNDS
		;;
	*)
		start_client $VLD_CCMD $DIR/client $IP_ADDRESS $PORT $RPMA_TEST_PMEM_PATH
		;;
	esac

	if [ "$MODE" != "fault-injection" -a $RV -ne 0 ]; then
		echo "Error: example $EXAMPLE FAILED!"
		N_FAILED=$(($N_FAILED + 1))
		LIST_FAILED="${LIST_FAILED}${EXAMPLE}\n"
	elif [ "$VLD_CCMD" != "" ]; then
		cerrno=$(grep "ERROR SUMMARY:" ${VLD_CLOG_FILE} | grep -Eoh "[0-9]+ errors" | awk '{print $1}')
		if [ "$cerrno" == "" ]; then
			cat ${VLD_CLOG_FILE}
			echo "Error: missing ERROR SUMMARY"
			exit 1
		fi
		if [ $cerrno -gt 0 ]; then
			echo "Error: example $EXAMPLE client $VLD_MSG FAILED!"
			CFAILED=1
			N_CFAILED=$(($N_CFAILED + 1))
			LIST_CFAILED="${LIST_CFAILED}${EXAMPLE}-client\n"
			mv ${VLD_CLOG_FILE} ${BIN_DIR}/$EXAMPLE-valgrind-client.log
			print_out_log_file ${BIN_DIR}/$EXAMPLE-valgrind-client.log
		fi
	fi

	# make sure the server's process is finished
	if [ "$MODE" != "fault-injection" -o $S_FI_VAL -gt 0 ]; then
		PID=$(get_PID_of_server $IP_ADDRESS $PORT)
		if [ "$PID" != "" ]; then
			echo "Notice: server is still running, waiting 1 sec ..."
			sleep 1
		fi
	fi
	PID=$(get_PID_of_server $IP_ADDRESS $PORT)
	if [ "$PID" != "" ]; then
		echo "Notice: server is still running, killing it ..."
		if [ "$MODE" != "fault-injection" -o $S_FI_VAL -gt 0 ]; then
			kill $PID
			sleep 1
		fi
		kill -9 $PID 2>/dev/null
	elif [ "$VLD_SCMD" != "" ]; then
		serrno=$(grep "ERROR SUMMARY:" ${VLD_SLOG_FILE} | grep -Eoh "[0-9]+ errors" | awk '{print $1}')
		if [ "$serrno" == "" ]; then
			cat ${VLD_SLOG_FILE}
			echo "Error: missing ERROR SUMMARY"
			exit 1
		fi
		if [ $serrno -gt 0 ]; then
			echo "Error: example $EXAMPLE server $VLD_MSG FAILED!"
			SFAILED=1
			N_SFAILED=$(($N_SFAILED + 1))
			LIST_SFAILED="${LIST_SFAILED}${EXAMPLE}-server\n"
			mv ${VLD_SLOG_FILE} ${BIN_DIR}/$EXAMPLE-valgrind-server.log
			print_out_log_file ${BIN_DIR}/$EXAMPLE-valgrind-server.log
		fi
	fi

	S_FI_MAX=0
	C_FI_MAX=0
	if [ "$LOG_OUTPUT" == "yes" ]; then
		if [ "$S_FI" != "" ]; then
			S_FI_MAX=$(get_max_fault_injection $S_LOG_FILE)
			error_out_if_no_max_fault_injection "$S_FI_MAX" "$S_LOG_FILE"
			echo "Detected S_FI_MAX = \"$S_FI_MAX\""
		elif [ "$C_FI" != "" ]; then
			C_FI_MAX=$(get_max_fault_injection $C_LOG_FILE)
			error_out_if_no_max_fault_injection "$C_FI_MAX" "$C_LOG_FILE"
			echo "Detected C_FI_MAX = \"$C_FI_MAX\""
		fi
	fi

	echo
}

### SCRIPT STARTS HERE ###

if [ "$RPMA_TEST_PMEM_PATH" != "" ]; then
	echo "Notice: running examples on PMem: $RPMA_TEST_PMEM_PATH (RPMA_TEST_PMEM_PATH)."
else
	echo "Notice: PMem path (RPMA_TEST_PMEM_PATH) is not set, examples will be run on DRAM."
fi

N_FAILED=0
LIST_FAILED=""
N_SFAILED=0
LIST_SFAILED=""
N_CFAILED=0
LIST_CFAILED=""

S_LOG_FILE="nohup_server.out"
C_LOG_FILE="nohup_client.out"

JOBS=$(ps aux | grep -e "server $IP_ADDRESS $PORT" -e "client $IP_ADDRESS $PORT" | grep -v "grep -e")
if [ "$JOBS" != "" ]; then
	echo "Wait for the following processes to finish or kill them:"
	echo "$JOBS"
	echo "Error: cannot run examples, because some of them are still running"
	exit 1
fi

verify_SoftRoCE

if [ "$MODE" == "valgrind" -o "$MODE" == "fault-injection" ]; then
	VLD_CMD="valgrind --leak-check=full"
	VLD_SUPP_PATH=$(dirname $0)/../tests/
	VLD_SUPP="--suppressions=${VLD_SUPP_PATH}/memcheck-libibverbs.supp"
	VLD_SUPP="${VLD_SUPP} --suppressions=${VLD_SUPP_PATH}/memcheck-libnl.supp"
	VLD_SUPP="${VLD_SUPP} --gen-suppressions=all"
	# prepare the server command
	VLD_SLOG_FILE="${BIN_DIR}/valgrind-server.log"
	VLD_SLOG="--log-file=${VLD_SLOG_FILE}"
	VLD_SCMD="${VLD_CMD} ${VLD_SUPP} ${VLD_SLOG}"
	# prepare the client command
	VLD_CLOG_FILE="${BIN_DIR}/valgrind-client.log"
	VLD_CLOG="--log-file=${VLD_CLOG_FILE}"
	VLD_CCMD="${VLD_CMD} ${VLD_SUPP} ${VLD_CLOG}"
	VLD_MSG="(under Valgrind)"
	# save the original values
	VLD_SCMD_ORIG=$VLD_SCMD
	VLD_CCMD_ORIG=$VLD_CCMD

	echo -n "Notice: running examples with Valgrind is tuned for debug build of librpma "
	echo -n "on Ubuntu 22.04 (see the CircleCI build). It may fail for any other OS, "
	echo "OS version, rdma-core version and for the release build."
	echo
fi

EXAMPLES=$(find $BIN_DIR -name server | sort)

if [ "$MODE" != "fault-injection" ]; then
	for srv in $EXAMPLES; do
		DIR=$(dirname $srv)
		run_example $DIR
	done
else
	# run the CLIENT with fault-injection
	for srv in $EXAMPLES; do
		DIR=$(dirname $srv)
		# get the maximum reachable value of fault-injection
		run_example $DIR 0 $GET_FI_MAX
		for fault_inject in $(seq 1 $(($C_FI_MAX + 1))); do
			run_example $DIR 0 $fault_inject
			print_FI_if_failed
		done
	done

	# run the SERVER with fault-injection
	for srv in $EXAMPLES; do
		DIR=$(dirname $srv)
		# get the maximum reachable value of fault-injection
		run_example $DIR $GET_FI_MAX 0
		for fault_inject in $(seq 1 $(($S_FI_MAX + 1))); do
			run_example $DIR $fault_inject 0
			print_FI_if_failed
		done
	done
fi


if [ $N_FAILED -gt 0 ]; then
	echo "$N_FAILED example(s) failed:"
	echo -e "$LIST_FAILED"
	err=1
fi

if [ $N_SFAILED -gt 0 ]; then
	echo "$N_SFAILED example(s) server $VLD_MSG failed:"
	echo -e "$LIST_SFAILED"
	err=1
fi

if [ $N_CFAILED -gt 0 ]; then
	echo "$N_CFAILED example(s) client $VLD_MSG failed:"
	echo -e "$LIST_CFAILED"
	err=1
fi

if [[ $err == 1 ]]; then
	exit 1
fi

echo "All examples succeeded"
