#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation

#
# run-all-on-SoftRoCE.sh - run all examples on SoftRoCE
#
# Usage: run-all-on-SoftRoCE.sh <binary-examples-directory> [IP_address] [port]
#

BIN_DIR=$1
IP_ADDRESS=$2
PORT=$3

MODULE="rdma_rxe"

function verify_SoftRoCE() {
	SCRIPT_DIR=$(dirname $0)
	$SCRIPT_DIR/config_softroce.sh verify

	if [ "$IP_ADDRESS" == "" ]; then
		NETDEV=$(rdma link show | grep -e "$STATE_OK" | head -n1 | cut -d' ' -f8)
		IP_ADDRESS=$(ip address show dev $NETDEV | grep -e inet | grep -v -e inet6 | cut -d' ' -f6 | cut -d/ -f1)
	fi

	if [ "$PORT" == "" ]; then
		PORT="8765"
	fi

	echo "Running examples for IP address $IP_ADDRESS and port $PORT:"
	echo
}

function run_example() {
	DIR=$1
	EXAMPLE=$(basename $DIR)

	echo "*** Running example: $EXAMPLE"

	# start the server
	$DIR/server $IP_ADDRESS $PORT &
	sleep 1

	# start the client(s)
	RV=0
	case $EXAMPLE in
	06-multiple-connections)
		for SEED in 8 9 11 12; do
			$DIR/client $IP_ADDRESS $PORT $SEED
			RV=$?
			[ $RV -ne 0 ] && break
		done
		;;
	*)
		$DIR/client $IP_ADDRESS $PORT
		RV=$?
		;;
	esac

	if [ $RV -ne 0 ]; then
		echo Error: example $EXAMPLE FAILED!
		N_FAILED=$(($N_FAILED + 1))
		FAILED="$FAILED$EXAMPLE\n"
	fi
	echo
}

### SCRIPT STARTS HERE ###

if [ "$BIN_DIR" == "" ]; then
	echo "Error: missing required argument"
	echo "Usage: run-all-on-SoftRoCE.sh <binary-examples-directory> [IP_address] [port]"
	exit 1
fi

verify_SoftRoCE

JOBS=$(ps aux | grep -e "server $IP_ADDRESS $PORT" -e "client $IP_ADDRESS $PORT" | grep -v "grep -e")
if [ "$JOBS" != "" ]; then
	echo "Wait for the following processes to finish or kill them:"
	echo "$JOBS"
	echo "Error: cannot run examples, because some of them are still running"
	exit 1
fi

N_FAILED=0
FAILED=""
for srv in $(find $BIN_DIR -name server | sort); do
	DIR=$(dirname $srv)
	run_example $DIR
done

if [ $N_FAILED -gt 0 ]; then
	echo "$N_FAILED example(s) failed:"
	echo -e "$FAILED"
	exit 1
fi

echo "All examples succeeded"
