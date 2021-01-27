#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# ddio -- query and toggle DDIO state per PCIe root port (EXPERIMENTAL)
#

#
# usage -- print usage message and exit
#
# usage [<error-msg>]
#
function usage()
{
	[ -n "$1" ] && echo Error: $1
	cat >&2 <<EOF
THIS TOOL IS EXPERIMENTAL. YOU ARE USING IT ON YOUR OWN RISK.

Usage:
	$0 -d device -s DDIO-state [ -v ]
	$0 -d device -q [ -v ]
	$0 -h

-d device	select which devices the operation affects
		format: [[[[<domain>]:]<bus>]:][<slot>][.[<func>]]
		For details please see setpci(8).

-s DDIO-state	desired DDIO state, either enable or disable
		Note: 'enable' is an equivalent of restoring default registers values
		For details please use -e.

-q		query DDIO state
		exit code 1 means DDIO is enabled (on)
		exit code 0 means DDIO is disabled ('off')

-v		be verbose

-h		print this help message

-e		print an excerpt from the documentation
		describing a principle of operation of this tool

EOF
	exit 1
}


#
# excerpt -- print a principle of operation and exit
#
function excerpt()
{
	cat <<EOF
Excerpt from XXX:
---
Use Allocating Flows for "Normal Writes" on VC0 and VCp
1: Use allocating flows for the writes that meet the following criteria.
0: Use non-allocating flows for writes that meet the following criteria.

Criteria:
(TPH=0 OR TPHDIS=1 OR
    (TPH=1 AND Tag=0 AND CIPCTRL[28]=1)) AND
(NS=0 OR NoSnoopOrWrEn=0) AND
Non-DCA Write
---

So allocating flows (aka DDIO) are disabled if:
(use_allocating_flow_wr=0 AND
    TPHDIS=1 AND
    NoSnoopOrWrEn=0)
and enabled otherwise.

Where:
perfctrlsts_0[7] is use_allocating_flow_wr (default: 0x1)
perfctrlsts_0[3] is NoSnoopOrWrEn (default: 0x0)
perfctrlsts_1[9] is TPHDIS (default: 0x0)

EOF
	exit 0
}


#
# constants
#
perfctrlsts_0_reg="180.b"
use_alloacting_flow_wr_bit=$((1 << 7))
nosnoopopwren_bit=$((1 << 3))

perfctrlsts_1_reg="184+1.b"
tphdis_bit=$((1 << 0))


#
# _log -- optionally echo a provided string to a log file
#
function _log()
{
	[ "x$DDIO_LOG" != "x" ] && echo "$1" >> $DDIO_LOG
	true # do not fail if the above condition is not true
}


#
# _register_get -- read a register value
#
function _register_get()
{
	register="$1"

	valhex=$(setpci -s "$device" "$register")
	valdec=$((16#$valhex))
	
	_log "setpci -s $device $register"
	_log "=$valhex"

	echo $valdec
}


#
# _register_set -- write a register value
#
function _register_set()
{
	register="$1"
	valdec="$2"

	valhex=`printf "%x\n" $valdec`
	out=$(setpci -s $device $register=$valhex 2>&1)

	_log "setpci -s $device $register=$valhex"
	_log "=$?"
	_log "$out"
}


#
# _bit_git -- get bit from a value
#
function _bit_get() {
	valdec="$1"
	bit="$2"
	
	valdec=$(($valdec & $bit))
	echo "$valdec"
}


#
# _bit_enable -- enable bit in a value
#
function _bit_enable() {
	valdec="$1"
	bit="$2"
	
	valdec=$(($valdec | $bit))
	echo "$valdec"
}


#
# _bit_disable -- disable bit in a value
#
function _bit_disable() {
	valdec="$1"
	bit="$2"
	
	valdec=$(($valdec & ~($bit)))
	echo "$valdec"
}


#
# _require_device -- verify the provided device is valid
#
function _require_device()
{
	[ -z "$device" ] && usage "device required: $device" 
	setpci -s "$device" "$perfctrlsts_0_reg" 2>&1 > /dev/null
	[ $? -ne 0 ] && usage "invalid device: $device"
	true # do not fail if the above condition is not true
}


#
# _require_root -- verify the script is run with root privileges
#
function _require_root()
{
	[ $EUID -ne 0 ] && usage "root privileges required"
	true # do not fail if the above condition is not true
}

#
# ddio_query -- read DDIO state
#
function ddio_query()
{
	_require_device
	_require_root

	# read registers value
	pcs0=$(_register_get "$perfctrlsts_0_reg")
	pcs1=$(_register_get "$perfctrlsts_1_reg")
	
	# read meaningful  bits from the registers
	use_alloacting_flow_wr=$(_bit_get "$pcs0" "$use_alloacting_flow_wr_bit")
	nosnoopopwren=$(_bit_get "$pcs0" "$nosnoopopwren_bit")
	tphdis=$(_bit_get "$pcs1" "$tphdis_bit")

	ddio_on="DDIO is enabled"
	ddio_off="DDIO is disabled"
	[ $use_alloacting_flow_wr -ne 0 ] && {
		[ $verbose -eq 1 ] && echo "$ddio_on: use_allocating_flow_wr != 0" 1>&2
		return 1
	}
	[ $nosnoopopwren -ne 0 ] && {
		[ $verbose -eq 1 ] && echo "$ddio_on: nosnoopopwren != 0" 1>&2
		return 1
	}
	[ $tphdis -eq 0 ] && {
		[ $verbose -eq 1 ] && echo "$ddio_on: TPHDIS == 0" 1>&2
		return 1
	}
	
	[ $verbose -eq 1 ] && echo "$ddio_off" 1>&2
	return 0
}


#
# _ddio_set_disable -- turn off DDIO
#
function _ddio_set_disable()
{
	[ $use_alloacting_flow_wr -ne 0 ] && {
		[ $verbose -eq 1 ] && echo "set use_allocating_flow_wr = 0" 1>&2
		pcs0=$(_bit_disable "$pcs0" "$use_alloacting_flow_wr_bit")
		pcs0_mod=1
	}
	[ $nosnoopopwren -ne 0 ] && {
		[ $verbose -eq 1 ] && echo "set nosnoopopwren = 0" 1>&2
		pcs0=$(_bit_disable "$pcs0" "$nosnoopopwren_bit")
		pcs0_mod=1
	}
	[ $tphdis -eq 0 ] && {
		[ $verbose -eq 1 ] && echo "set TPHDIS = 1" 1>&2
		pcs1=$(_bit_enable "$pcs1" "$tphdis_bit")
		pcs1_mod=1
	}
	true # do not fail if the above condition is not true
}


#
# _ddio_set_enable -- turn on DDIO (restore defaults)
#
function _ddio_set_enable()
{
	def="(default)"
	[ $use_alloacting_flow_wr -eq 0 ] && {
		[ $verbose -eq 1 ] && echo "set use_allocating_flow_wr = 1 $def" 1>&2
		pcs0=$(_bit_enable "$pcs0" "$use_alloacting_flow_wr_bit")
		pcs0_mod=1
	}
	[ $nosnoopopwren -ne 0 ] && {
		[ $verbose -eq 1 ] && echo "set nosnoopopwren = 0 $def" 1>&2
		pcs0=$(_bit_disable "$pcs0" "$nosnoopopwren_bit")
		pcs0_mod=1
	}
	[ $tphdis -ne 0 ] && {
		[ $verbose -eq 1 ] && echo "set TPHDIS = 0" 1>&2
		pcs1=$(_bit_disable "$pcs1" "$tphdis_bit")
		pcs1_mod=1
	}
	true # do not fail if the above condition is not true
}


#
# ddio_set -- write DDIO state
#
function ddio_set()
{
	_require_device
	_require_root

	# read current registers value
	pcs0=$(_register_get "$perfctrlsts_0_reg")
	pcs1=$(_register_get "$perfctrlsts_1_reg")
	
	# read meaningful  bits from the registers
	use_alloacting_flow_wr=$(_bit_get "$pcs0" "$use_alloacting_flow_wr_bit")
	nosnoopopwren=$(_bit_get "$pcs0" "$nosnoopopwren_bit")
	tphdis=$(_bit_get "$pcs1" "$tphdis_bit")
	
	# setup the registers modification indicators
	pcs0_mod=0
	pcs1_mod=0
	
	_ddio_set_$ddio
	
	# write the new registers value
	[ $pcs0_mod -eq 1 ] && _register_set "$perfctrlsts_0_reg" "$pcs0"
	[ $pcs1_mod -eq 1 ] && _register_set "$perfctrlsts_1_reg" "$pcs1"
	true # do not fail if the above condition is not true
}


#
# defaults
#
device=
op=usage
ddio=
verbose=0


#
# command-line argument processing...
#
args=`getopt d:s:qvhe $*`
[ $? != 0 ] && usage
set -- $args
for arg
do
	case "$arg"
	in
	-d)
		device="$2"
		shift 2
		;;
	-s)
		op="ddio_set"
		case "$2"
		in
		enable|disable)
			;;
		*)
			usage "invalid DDIO-state value '$2'"
			;;
		esac
		ddio="$2"
		shift 2
		;;
	-q)
		op="ddio_query"
		shift
		;;
	-v)
		verbose=1
		shift
		;;
	-h)
		usage
		;;
	-e)
		excerpt
		;;
	esac
done


#
# run a required operation
#
$op

