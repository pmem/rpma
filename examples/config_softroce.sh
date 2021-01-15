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
	LINK=""
else
	VERIFY=0
	LINK=$1
fi

MODULE="rdma_rxe"
DIR="/lib/modules/$(uname -r)"
STATE_OK="state ACTIVE physical_state LINK_UP"

function print_IP() {
	NETDEV=$(rdma link show | grep -e "$STATE_OK" | head -n1 | cut -d' ' -f8)
	IP=$(ip -4 -j -p a show $NETDEV | grep -e local | cut -d'"' -f4)
	echo "IP address of the SoftRoCE-configured network interface: $IP"
}

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

if [ "$LINK" == "" ]; then
	RDMA_LINKS=$(rdma link show | grep -e "$STATE_OK" | wc -l)
	if [ $RDMA_LINKS -gt 0 ]; then
		if [ $VERIFY -eq 0 ]; then
			echo "SoftRoCE has been already configured:"
			rdma link show | grep -e "$STATE_OK"
			print_IP
		fi
		exit 0
	elif [ $VERIFY -eq 1 ]; then
		echo "Error: SoftRoCE has not been configured yet!"
		exit 1
	fi

	# pick up the first 'up' one
	LINK=$(ip link | grep -v -e "LOOPBACK" | grep -e "state UP" | head -n1 | cut -d: -f2 | cut -d' ' -f2)
	if [ "$LINK" == "" ]; then
		#
		# Look for a USB Ethernet network interfaces,
		# which may not have 'state UP',
		# but only 'UP' and 'state UNKNOWN', for example:
		# ... <BROADCAST,MULTICAST,UP,LOWER_UP> ... state UNKNOWN ...
		#
		LINK=$(ip link | grep -v -e "LOOPBACK" | grep -e "UP" | grep -e "state UNKNOWN" | head -n1 | cut -d: -f2 | cut -d' ' -f2)
		if [ "$LINK" == "" ]; then
			echo "Error: cannot find an active and up network interface"
			exit 1
		fi
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
print_IP
