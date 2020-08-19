#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation

#
# config_softroce.sh - configure SoftRoCE
#
# Usage: config_softroce.sh [<network_interface>|verify]
#
# Options:
# <network_interface> - configure SoftRoCE for the given <network_interface>
#                       or for the first active and up one if no argument
#                       is given
# verify              - verify if SoftRoCE is already configured
#

if [ "$1" == "verify" ]; then
	VERIFY=1
	LINK=""
else
	VERIFY=0
	LINK=$1
fi

MODULE="rdma_rxe"
DIR="/lib/modules/$(uname -r)"

if [ $(lsmod | grep -e $MODULE | wc -l) -lt 1 ]; then
	N_MODULES=$(find $DIR -name "$MODULE.ko*" | wc -l)
	if [ $N_MODULES -lt 1 ]; then
		echo "Error: cannot find the '$MODULE' module in the '$DIR' directory"
		exit 1
	fi

	if ! sudo modprobe $MODULE; then
		echo "Error: cannot load the '$MODULE' module"
		sudo modprobe -v $MODULE
		exit 1
	fi
fi

if ! which ip > /dev/null; then
	echo "Error: cannot find the 'ip' command. Install the 'iproute/iproute2' package"
	exit 1
fi

if ! which rdma > /dev/null; then
	echo "Error: cannot find the 'rdma' command. Install the 'iproute/iproute2' package"
	exit 1
fi

if ! rdma link show > /dev/null ; then
	echo "Error: the 'rdma link show' command failed"
	exit 1
fi

STATE_OK="state ACTIVE physical_state LINK_UP"

if [ "$LINK" == "" ]; then
	RDMA_LINKS=$(rdma link show | grep -e "$STATE_OK" | wc -l)
	if [ $RDMA_LINKS -gt 0 ]; then
		if [ $VERIFY -eq 0 ]; then
			echo "SoftRoCE has been already configured:"
			rdma link show
		fi
		exit 0
	fi

	# pick up the first 'up' one
	LINK=$(ip link | grep -v -e "LOOPBACK" | grep -e "state UP" | head -n1 | cut -d: -f2 | cut -d' ' -f2)
	if [ "$LINK" == "" ]; then
		echo "Error: cannot find an active and up network interface"
		exit 1
	fi

fi

echo "Configuring SoftRoCE for the '$LINK' network interface:"
sudo rdma link add rxe_$LINK type rxe netdev $LINK
if [ $? -ne 0 ]; then
	echo "Error: configuring SoftRoCE failed"
	exit 1
fi

rdma link show

RDMA_LINKS=$(rdma link show | grep -e "$STATE_OK" | wc -l)
if [ $RDMA_LINKS -lt 1 ]; then
	echo "Error: configuring SoftRoCE failed"
	exit 1
fi

echo "SoftRoCE successfully configured"

