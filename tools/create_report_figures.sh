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
#

function usage()
{
	echo "Error: $1"
	echo
	echo "usage: $0"
	echo
	echo "export DATA_PATH=/custom/data/path"
    echo "export STAMP=CUSTOM_REPORT_STAMP"
    echo "export LAT_MACHINE=NAME_OF_THE_MACHINE"
    echo "export BW_MACHINE=NAME_OF_THE_MACHINE"
	exit 1
}

function echo_filter()
{
    for f in $*; do
        echo $f
    done
}

function files_to_machines()
{
    for f in $*; do
        echo "$f" | sed -E 's/.*(WP_[0-9]{3}_[0-9]{3}).*/\1/'
    done
}

function lat_figure()
{
    filter="$1"
    title="$2"
    output="$3"
    shift 3

    if [ "$#" -gt 0 ]; then
        legend="$*"
    else
        legend="$(files_to_machines $filter)"
    fi

    echo_filter $filter
    ./csv_compare.py \
        --output_title "$title" \
        --output_layout 'lat' \
        --output_with_table \
        --legend $legend \
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
        legend="$*"
    else
        legend="$(files_to_machines $filter)"
    fi

    echo_filter $filter
    ./csv_compare.py \
        --output_title "$title" \
        --output_layout 'bw' \
        --arg_axis "$arg_axis" \
        --output_with_table \
        --legend $legend \
        --output_file "$output.png" \
        $filter
    echo
}

# prepare a report directory
TIMESTAMP=$(date +%y-%m-%d-%H%M%S)
STAMP=${STAMP:-TIMESTAMP}
REPORT_DIR=report_$STAMP
rm -rdf $REPORT_DIR
mkdir $REPORT_DIR
cd $REPORT_DIR

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
lat_figure \
    "$DATA_PATH/READ/$LAT_MACHINE/DRAM/*/CSV/*lat* $DATA_PATH/READ/$LAT_MACHINE/PMEM/*/CSV/*lat*" \
    'ib_read_lat vs rpma_read() from DRAM and PMEM' \
    'Figure_2_3_4_rpma_read_lat_vs_ib' \
    'ib_read_lat' 'rpma_read() from DRAM' 'rpma_read() from PMEM'

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
bw_figure \
    "$DATA_PATH/READ/$machine/DRAM/*/CSV/*bw-bs* $DATA_PATH/READ/$machine/PMEM/*/CSV/*bw-bs*" \
    'ib_read_bw vs rpma_read() from DRAM and PMEM' \
    'bs' \
    'Figure_5_rpma_read_bw_bs_vs_ib' \
    'ib_read_bw' 'rpma_read() from DRAM' 'rpma_read() from PMEM'

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
bw_figure \
    "$DATA_PATH/READ/$BW_MACHINE/DRAM/*/CSV/*bw-th* $DATA_PATH/READ/$BW_MACHINE/PMEM/*/CSV/*bw-th*" \
    'ib_read_bw vs rpma_read() from DRAM and PMEM' \
    'threads' \
    'Figure_6_rpma_read_bw_th_vs_ib' \
    'ib_read_bw' 'rpma_read() from DRAM' 'rpma_read() from PMEM'
