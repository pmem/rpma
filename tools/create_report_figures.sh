#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# create_report_figures.sh -- generate Figures and Appendix charts for
# a performance report (EXPERIMENTAL)
#
# XXX Describe assumed directory hierarchy.
# XXX add DDIO_ON / DDIO_OFF to the directory hierarchy.
#

echo "This tool is EXPERIMENTAL"

function usage()
{
    echo "Error: $1"
    echo
    echo "usage: $0"
    echo
    echo "export DATA_PATH=/custom/data/path"
    echo "export STAMP=CUSTOM_REPORT_STAMP"
    echo "export READ_LAT_MACHINE=NAME_OF_THE_MACHINE # a machine used to generate the Figure_2_3_4"
    echo "export READ_BW_MACHINE=NAME_OF_THE_MACHINE # a machine used to generate the Figure_5 and Figure_6"
    echo "export WRITE_LAT_MACHINE=NAME_OF_THE_MACHINE # a machine used to generate the Figure_7"
    echo "export WRITE_BW_MACHINE=NAME_OF_THE_MACHINE"
    exit 1
}

if [ "$#" -ne "0" ]; then
    usage "too many arguments"
elif [ -z "$DATA_PATH" ]; then
	usage "DATA_PATH not set"
fi

function echo_filter()
{
    for f in $*; do
        echo $f
    done
}

function files_to_machines()
{
    for f in $*; do
        echo "$f" | sed -E 's/.*MACHINE_([0-9A-Za-z_]+).*/\1/'
    done
}

function lat_figure()
{
    filter="$1"
    title="$2"
    output="$3"
    shift 3

    if [ "$#" -gt "0" ]; then
        legend=( "$@" )
    else
        legend=( $(files_to_machines $filter) )
    fi

    echo_filter $filter
    $TOOLS_PATH/csv_compare.py \
        --output_title "$title" \
        --output_layout 'lat' \
        --output_with_table \
        --legend "${legend[@]}" \
        --output_file "$output.png" \
        $filter
    echo
}

function bw_figure()
{
    filter="$1"
    title="$2"
    arg_axis="$3"
    output="$4"
    shift 4

    if [ "$#" -gt 0 ]; then
        legend=( "$@" )
    else
        legend=( $(files_to_machines $filter) )
    fi

    echo_filter $filter
    $TOOLS_PATH/csv_compare.py \
        --output_title "$title" \
        --output_layout 'bw' \
        --arg_axis "$arg_axis" \
        --output_with_table \
        --legend "${legend[@]}" \
        --output_file "$output.png" \
        $filter
    echo
}

TOOLS_PATH=$(pwd)

# prepare a report directory
TIMESTAMP=$(date +%y-%m-%d-%H%M%S)
STAMP=${STAMP:-$TIMESTAMP}
REPORT_DIR=report_$STAMP
rm -rdf $REPORT_DIR
mkdir $REPORT_DIR
cd $REPORT_DIR

echo "Output directory: $REPORT_DIR"
echo

echo 'READ LAT'
mkdir 02_read_lat
cd 02_read_lat

echo '- compare all machines ib_read_lat'
lat_figure \
    "$DATA_PATH/READ/*/DRAM/*/CSV/ib*lat*" \
    'ib_read_lat (DRAM)' \
    'Appendix_B1_ib_read_lat'

echo '- compare all machines rpma_read() from DRAM'
lat_figure \
    "$DATA_PATH/READ/*/DRAM/*/CSV/rpma*lat*" \
    'rpma_read() from DRAM' \
    'Appendix_B2_rpma_read_lat_DRAM'

echo '- compare all machines rpma_read() from PMEM'
lat_figure \
    "$DATA_PATH/READ/*/PMEM/*/CSV/rpma*lat*" \
    'rpma_read() from PMEM' \
    'Appendix_B3_rpma_read_lat_PMEM'

echo '- ib_read_lat vs rpma_read() from DRAM vs rpma_read() from PMEM'
if [ -z "$READ_LAT_MACHINE" ]; then
	echo "SKIP: READ_LAT_MACHINE not set"
    echo
else
    lat_figure \
        "$DATA_PATH/READ/$READ_LAT_MACHINE/DRAM/*/CSV/*lat* $DATA_PATH/READ/$READ_LAT_MACHINE/PMEM/*/CSV/*lat*" \
        'ib_read_lat vs rpma_read() from DRAM and PMEM' \
        'Figure_2_3_4_rpma_read_lat_vs_ib' \
        'ib_read_lat' 'rpma_read() from DRAM' 'rpma_read() from PMEM'
fi

cd ..

echo "READ BW(BS)"
mkdir 03_read_bw_bs
cd 03_read_bw_bs

echo "- compare all machines ib_read_bw"
bw_figure \
    "$DATA_PATH/READ/*/DRAM/*/CSV/ib*bw-bs*" \
    'ib_read_bw (DRAM)' \
    'bs' \
    'Appendix_C1_ib_read_bw_bs'


echo "- compare all machines rpma_read() from DRAM"
bw_figure \
    "$DATA_PATH/READ/*/DRAM/*/CSV/rpma*bw-bs*" \
    'rpma_read() from DRAM' \
    'bs' \
    'Appendix_C2_rpma_read_bw_bs_dram'

echo "- compare all machines rpma_read() from PMEM"
bw_figure \
    "$DATA_PATH/READ/*/PMEM/*/CSV/rpma*bw-bs*" \
    'rpma_read() from PMEM' \
    'bs' \
    'Appendix_C3_rpma_read_bw_bs_pmem'

echo "- ib_read_bw vs rpma_read() from DRAM vs rpma_read() from PMEM"
if [ -z "$READ_BW_MACHINE" ]; then
	echo "SKIP: READ_BW_MACHINE not set"
    echo
else
    bw_figure \
        "$DATA_PATH/READ/$READ_BW_MACHINE/DRAM/*/CSV/*bw-bs* $DATA_PATH/READ/$READ_BW_MACHINE/PMEM/*/CSV/*bw-bs*" \
        'ib_read_bw vs rpma_read() from DRAM and PMEM' \
        'bs' \
        'Figure_5_rpma_read_bw_bs_vs_ib' \
        'ib_read_bw' 'rpma_read() from DRAM' 'rpma_read() from PMEM'
fi

cd ..

echo "READ BW(TH)"
mkdir 04_read_bw_th
cd 04_read_bw_th

echo "- compare all machines ib_read_bw"
bw_figure \
    "$DATA_PATH/READ/*/DRAM/*/CSV/ib*bw-th*" \
    'ib_read_bw (DRAM)' \
    'threads' \
    'Appendix_D1_ib_read_bw_th'

echo "- compare all machines rpma_read() from DRAM"
bw_figure \
    "$DATA_PATH/READ/*/DRAM/*/CSV/rpma*bw-th*" \
    'rpma_read() from DRAM' \
    'threads' \
    'Appendix_D2_rpma_read_bw_th_dram'

echo "- compare all machines rpma_read() from PMEM"
bw_figure \
    "$DATA_PATH/READ/*/PMEM/*/CSV/rpma*bw-th*" \
    'rpma_read() from PMEM' \
    'threads' \
    'Appendix_D3_rpma_read_bw_th_pmem'

echo "- ib_read_bw vs rpma_read() from DRAM vs rpma_read() from PMEM"
if [ -z "$READ_BW_MACHINE" ]; then
	echo "SKIP: READ_BW_MACHINE not set"
    echo
else
    bw_figure \
        "$DATA_PATH/READ/$READ_BW_MACHINE/DRAM/*/CSV/*bw-th* $DATA_PATH/READ/$READ_BW_MACHINE/PMEM/*/CSV/*bw-th*" \
        'ib_read_bw vs rpma_read() from DRAM and PMEM' \
        'threads' \
        'Figure_6_rpma_read_bw_th_vs_ib' \
        'ib_read_bw' 'rpma_read() from DRAM' 'rpma_read() from PMEM'
fi

cd ..

echo 'WRITE LAT'
mkdir 05_write_lat
cd 05_write_lat

echo '- compare all machines rpma_write() + rpma_flush() to DRAM'
lat_figure \
    "$DATA_PATH/WRITE/*/DRAM/*/CSV/rpma*lat*" \
    'rpma_write() + rpma_flush() to DRAM' \
    'Appendix_E1_rpma_write_flush_lat_DRAM'

echo '- compare all machines rpma_write() + rpma_flush() to PMEM'
lat_figure \
    "$DATA_PATH/WRITE/*/PMEM/*/CSV/rpma*lat*" \
    'rpma_write() + rpma_flush() to PMEM' \
    'Appendix_E2_rpma_write_flush_lat_PMEM'

echo '- rpma_write() + rpma_flush() to DRAM vs to PMEM'
if [ -z "$WRITE_LAT_MACHINE" ]; then
	echo "SKIP: WRITE_LAT_MACHINE not set"
    echo
else
    lat_figure \
        "$DATA_PATH/WRITE/$WRITE_LAT_MACHINE/DRAM/*/CSV/*lat* $DATA_PATH/WRITE/$WRITE_LAT_MACHINE/PMEM/*/CSV/*lat*" \
        'rpma_write() + rpma_flush() to DRAM vs to PMEM' \
        'Figure_7_rpma_write_flush_lat' \
        'to DRAM' 'to PMEM'
fi

cd ..
