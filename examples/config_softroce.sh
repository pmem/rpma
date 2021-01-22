#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation

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
	NET_IF=""
else
	VERIFY=0
	NET_IF=$1
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

if [ "$NET_IF" == "" ]; then
	RDMA_LINKS=$(rdma link show | grep -e "$STATE_OK" | wc -l)
	if [ $RDMA_LINKS -gt 0 ]; then
		if [ $VERIFY -eq 0 ]; then
			echo "SoftRoCE has been already configured:"
			rdma link show | grep -e "$STATE_OK"
		fi
		exit 0
	elif [ $VERIFY -eq 1 ]; then
		echo "Error: SoftRoCE has not been configured yet!"
		exit 1
	fi

	# pick up the first 'up' network interface
	NET_IF=$(ip link | grep -v -e "LOOPBACK" | grep -e "state UP" | head -n1 | cut -d: -f2 | cut -d' ' -f2)
	if [ "$NET_IF" == "" ]; then
		#
		# Look for a USB Ethernet network interfaces,
		# which may not have 'state UP',
		# but only 'UP' and 'state UNKNOWN', for example:
		# ... <BROADCAST,MULTICAST,UP,LOWER_UP> ... state UNKNOWN ...
		#
		NET_IF=$(ip link | grep -v -e "LOOPBACK" | grep -e "UP" | grep -e "state UNKNOWN" | head -n1 | cut -d: -f2 | cut -d' ' -f2)
		if [ "$NET_IF" == "" ]; then
			echo "Error: cannot find an active and up network interface"
			exit 1
		fi
	fi
fi

echo "Configuring SoftRoCE for the '$NET_IF' network interface..."
RXE_NAME="rxe_$NET_IF"
sudo rdma link add $RXE_NAME type rxe netdev $NET_IF
if [ $? -ne 0 ]; then
	echo "Error: configuring SoftRoCE failed"
	exit 1
fi

RDMA_LINKS=$(rdma link show | grep -e "$STATE_OK" | grep -e "$NET_IF" | wc -l)
if [ $RDMA_LINKS -lt 1 ]; then
	echo "Error: configuring SoftRoCE for the '$NET_IF' network interface failed"
	exit 1
fi

echo "SoftRoCE for the '$NET_IF' network interface was successfully configured:"
rdma link show | grep -e "$NET_IF"
