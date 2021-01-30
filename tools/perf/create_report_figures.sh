#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# create_report_figures.sh -- generate Figures and Appendix charts for
# a performance report (EXPERIMENTAL)
#
# Note: The DATA_PATH variable has to point directories of the following
# structure:
# .
# ├── MACHINE_A
# │   └── *.csv
# ├── MACHINE_B
# │   └── *.csv
# └── ...
#
# Note: It is assumed APM is ALWAYS used when DDIO is turned OFF on the target
# side whereas for GPSPM DDIO is turned ON on the target side. If one would like
# to e.g. generate a comparison between either APM or GPSPM for both DDIO
# turned ON and OFF but on the client-side, you should use other available
# mechanisms e.g. by creating separate directories:
# - MACHINE_A_DDIO_ON
# - MACHINE_A_DDIO_OFF
#

echo "This tool is EXPERIMENTAL"

function usage()
{
    echo "Error: $1"
    echo
    echo "usage: $0 report|appendix"
    echo
    echo "export DATA_PATH=/custom/data/path"
    echo "export STAMP=CUSTOM_REPORT_STAMP"
    echo
    echo "export READ_LAT_MACHINE=<machine>"
    echo "export READ_BW_MACHINE=<machine>"
    echo "export WRITE_LAT_MACHINE=<machine>"
    echo "export WRITE_BW_MACHINE=<machine>"
    echo "export MIX_LAT_MACHINE=<machine>"
    echo "export MIX_BW_MACHINE=<machine>"
    exit 1
}

if [ $# -lt 1 ]; then
    usage "Too few arguments"
elif [ -z "$DATA_PATH" ]; then
	usage "DATA_PATH not set"
fi

case "$1" in
report|appendix)
    ;;
*)
    usage "Unknown mode: $1"
esac

SET=$1
N_DIGITS=3

set -e # stop on the first error

function echo_filter()
{
    for f in $*; do
        echo $f
    done
}

function mode_seqrand()
{
    case "$1" in
    *rand*)
        echo '_rand'
        ;;
    *)
        echo '_seq'
        ;;
    esac
}

function mode_single()
{
    echo ''
}

function mode_rw()
{
    case "$1" in
    *read*)
        echo '_read'
        ;;
    *write*)
        echo '_write'
        ;;
    esac
}

function files_to_machines()
{
    mode=$1
    shift 1

    for f in $*; do
        machine=$(echo "$f" | sed -E 's/.*MACHINE_([0-9A-Za-z_]+).*/\1/')
        suffix=$(mode_${mode} "$f")

        echo "${machine}${suffix}"
    done
}

function lat_appendix()
{
    local filter="$1"
    local no="$2"
    local title="$3"
    local mode="$4"
    local output="$5"
    shift 5

    if [ $# -gt 0 ]; then
        local legend=( "$@" )
    else
        local legend=( $(files_to_machines $mode $filter) )
    fi

    echo_filter $filter
    $TOOLS_PATH/csv_compare.py \
        --output_title "Appendix $no. Latency: $title (all platforms)" \
        --output_layout 'lat_all' \
        --output_with_table \
        --legend "${legend[@]}" \
        --output_file "$output.png" \
        $filter
    echo
}

function bw_appendix()
{
    local filter="$1"
    local no="$2"
    local title="$3"
    local arg_axis="$4"
    local mode="$5"
    local output="$6"

    local legend=( $(files_to_machines $mode $filter) )

    echo_filter $filter
    $TOOLS_PATH/csv_compare.py \
        --output_title "Appendix $no. Bandwidth: $title (all platforms)" \
        --output_layout 'bw' \
        --arg_axis "$arg_axis" \
        --arg_xscale 'log' \
        --output_with_table \
        --legend "${legend[@]}" \
        --output_file "$output.png" \
        $filter
    echo
}

function appendix_set()
{
    local filter="$1"
    local memtype="$2"
    local letter="$3"
    local OP="$4" # APM, GPSPM, rpma_read(), rpma_write()
    local suffix="$5"
    local mode=${6-seqrand}

    case "$memtype" in
    dram)
        src="dram"
        ;;
    dax)
        src="pmem"
        ;;
    esac

    # uppercase
    SRC=${src^^}

    case "$OP" in
     *read*)
        dir='from'
        ;;
     *MIX*)
        dir='against'
        ;;
    *)
        dir='to'
        ;;
    esac

    # lowercase and drop parentheses
    op=${OP,,}
    op=${op/(/}
    op=${op/)/}

    echo "Appendix $letter $OP $dir $SRC $suffix"
    lat_appendix \
        "$DATA_PATH/*/${filter}_lat*${memtype}*" \
        "${letter}1" "$OP $dir $SRC $suffix" "$mode" \
        "Appendix_${letter}1_${op}_${src}_lat"
    bw_appendix \
        "$DATA_PATH/*/${filter}_bw-bs*${memtype}*" \
        "${letter}2" "$OP $dir $SRC $suffix" \
        'bs' "$mode" \
        "Appendix_${letter}2_${op}_${src}_bw_bs"
    bw_appendix \
        "$DATA_PATH/*/${filter}_bw-th*${memtype}*" \
        "${letter}3" "$OP $dir $SRC $suffix" \
        'threads' "$mode" \
        "Appendix_${letter}3_${op}_${src}_bw_th"
}

function figures_appendix
{
    echo 'Appendix A ib_read_lat'
    lat_appendix \
        "$DATA_PATH/*/ib_read_lat-*" \
        'A' 'ib_read_lat from DRAM' 'single' \
        'Appendix_A_ib_read_lat'

    echo 'Appendix B ib_read_bw bw(bs) bw(th)'
    bw_appendix \
        "$DATA_PATH/*/ib_read_bw-bs-*" \
        'B1' 'ib_read_bw from DRAM' \
        'bs' 'single' \
        'Appendix_B1_ib_read_bw_bs'
    bw_appendix \
        "$DATA_PATH/*/ib_read_bw-th-*" \
        'B2' 'ib_read_bw from DRAM' \
        'threads' 'single' \
        'Appendix_B2_ib_read_bw_th'

    appendix_set '*apm_*read' 'dram' 'C' 'rpma_read()'
    appendix_set '*apm_*read' 'dax' 'D' 'rpma_read()'
    appendix_set '*apm_*write' 'dram' 'E' 'rpma_write()' '(DDIO=ON)'
    appendix_set '*apm_*write' 'dax' 'F' 'APM'
    appendix_set '*gpspm_*write' 'dax' 'G' 'GPSPM'
    mix_desc='70% read, 30% write'
    appendix_set '*apm_rw' 'dax' 'H' 'MIX' "($mix_desc, seq)" 'rw'
    appendix_set '*apm_randrw' 'dax' 'I' 'MIX' "($mix_desc, rand)" 'rw'
}

function set_data_path()
{
    VAR=$1

    if [ -z "${!VAR}" ]; then
        echo "SKIP: $VAR not set"
        echo
        data_path='skip'
    else
        data_path="$DATA_PATH/MACHINE_${!VAR}"
    fi
}

function lat_figures()
{
    filter="$1"
    title="$2"
    output="$3"
    shift 3

    if [ $# -gt 0 ]; then
        legend=( "$@" )
    else
        legend=( $(files_to_machines $filter) )
    fi

    echo "- $title"
    echo_filter $filter
    layouts=('lat_avg' 'lat_pctls')
    title_prefixes=( \
        'Latency' \
        'Latency (99.9% and 99.999% percentiles)')
    for index in "${!layouts[@]}"; do
        layout="${layouts[$index]}"
        title_prefix="${title_prefixes[$index]}"
        printf -v figno_name "%0${N_DIGITS}d" $figno
        $TOOLS_PATH/csv_compare.py \
            --output_title "Figure $figno. $title_prefix: $title" \
            --output_layout "$layout" \
            --output_with_table \
            --legend "${legend[@]}" \
            --output_file "Figure_${figno_name}_${output}_${layout}.png" \
            $filter
        figno=$((figno + 1))
    done
    echo
}

function bw_figures()
{
    filter="$1"
    title="$2"
    output="$3"
    shift 3

    if [ $# -gt 0 ]; then
        legend=( "$@" )
    else
        legend=( $(files_to_machines $filter) )
    fi

    axises=('bs' 'threads')
    axis_filters=('bw-bs' 'bw-th')

    for index in "${!axises[@]}"; do
        axis="${axises[$index]}"
        axis_filter="${axis_filters[$index]}"
        # replace '{axis}' with actual filter by axis
        eff_filter="${filter//\{axis\}/${axis_filter}}"
        echo "- $title - bw($axis)"
        printf -v figno_name "%0${N_DIGITS}d" $figno
        echo_filter $eff_filter
        $TOOLS_PATH/csv_compare.py \
            --output_title "Figure $figno. Bandwidth: $title" \
            --output_layout 'bw' \
            --arg_axis "$axis" \
            --arg_xscale 'log' \
            --output_with_table \
            --legend "${legend[@]}" \
            --output_file "Figure_${figno_name}_${output}_${axis_filter}.png" \
            $eff_filter
        figno=$((figno + 1))
    done
    echo
}

function figures_report()
{
    # a global Figure indexer
    figno=1

    echo "READ LAT"
    set_data_path READ_LAT_MACHINE
    if [ "$data_path" != 'skip' ]; then
        lat_figures \
            "$data_path/ib_read_lat* $data_path/*apm_*read_lat*dram*" \
            'ib_read_lat vs rpma_read() from DRAM' \
            'ib_read_lat_vs_rpma_read_dram' \
            'ib_read_lat' 'rpma_read() rand' 'rpma_read() seq'

        lat_figures \
            "$data_path/*read_lat*dram* $data_path/*read_lat*dax*" \
            'rpma_read() from DRAM vs from PMEM' \
            'rpma_read_dram_vs_pmem' \
            'DRAM rand' 'DRAM seq' 'PMEM rand' 'PMEM seq'
    else
        figno=$((figno + 4))
    fi

    echo "READ BW"
    set_data_path READ_BW_MACHINE
    if [ "$data_path" != 'skip' ]; then
        bw_figures \
            "$data_path/ib_read_{axis}* $data_path/*apm_*read_{axis}*dram*" \
            'ib_read_lat vs rpma_read() from DRAM' \
            'ib_read_bw_vs_rpma_read_dram' \
            'ib_read_lat' 'rpma_read() rand' 'rpma_read() seq'
        bw_figures \
            "$data_path/*read_{axis}*dram* $data_path/*read_{axis}*dax*" \
            'rpma_read() from DRAM vs from PMEM' \
            'rpma_read_dram_vs_pmem' \
            'DRAM rand' 'DRAM seq' 'PMEM rand' 'PMEM seq'
    else
        figno=$((figno + 4))
    fi

    echo 'WRITE LAT'
    set_data_path WRITE_LAT_MACHINE
    if [ "$data_path" != 'skip' ]; then
        lat_figures \
            "$data_path/*apm_*write_lat*dram* $data_path/*apm_*write_lat*dax*" \
            'APM to DRAM (DDIO=ON) vs to PMEM' \
            'apm_dram_vs_pmem' \
            'DRAM rand' 'DRAM seq' 'PMEM rand' 'PMEM seq'

        lat_figures \
            "$data_path/*apm_*write_lat*dax* $data_path/*gpspm_*write_lat*dax*" \
            'APM to PMEM vs GPSPM to PMEM' \
            'apm_pmem_vs_gpspm_pmem' \
            'APM rand' 'APM seq' 'GPSPM rand' 'GPSPM seq'
    else
        figno=$((figno + 4))
    fi

    echo "WRITE BW"
    set_data_path WRITE_BW_MACHINE
    if [ "$data_path" != 'skip' ]; then
        bw_figures \
            "$data_path/*apm_*write_{axis}*dram* $data_path/*apm_*write_{axis}*dax*" \
            'APM to DRAM (DDIO=ON) vs to PMEM' \
            'apm_dram_vs_pmem' \
            'DRAM rand' 'DRAM seq' 'PMEM rand' 'PMEM seq'
        bw_figures \
            "$data_path/*apm_*write_{axis}*dax* $data_path/*gpspm_*write_{axis}*dax*" \
            'APM to PMEM vs GPSPM to PMEM' \
            'apm_pmem_vs_gpspm_pmem' \
            'APM rand' 'APM seq' 'GPSPM rand' 'GPSPM seq'
    else
        figno=$((figno + 4))
    fi

    echo 'MIX LAT'
    set_data_path MIX_LAT_MACHINE
    if [ "$data_path" != 'skip' ]; then
        lat_figures \
            "$data_path/*apm_read_lat*dax* $data_path/*apm_write_lat*dax* $data_path/*apm_rw_lat*dax*" \
            'MIX against PMEM vs rpma_read() + APM to PMEM (seq)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'

        lat_figures \
            "$data_path/*apm_randread_lat*dax* $data_path/*apm_randwrite_lat*dax* $data_path/*apm_randrw_lat*dax*" \
            'MIX against PMEM vs rpma_read() + APM to PMEM (rand)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'
    else
        figno=$((figno + 4))
    fi

    echo "MIX BW"
    set_data_path MIX_BW_MACHINE
    if [ "$data_path" != 'skip' ]; then
        bw_figures \
            "$data_path/*apm_read_{axis}*dax* $data_path/*apm_write_{axis}*dax* $data_path/*apm_rw_{axis}*dax*" \
            'MIX against PMEM vs rpma_read() + APM to PMEM (seq)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'
        bw_figures \
            "$data_path/*apm_randread_{axis}*dax* $data_path/*apm_randwrite_{axis}*dax* $data_path/*apm_randrw_{axis}*dax*" \
            'MIX against PMEM vs rpma_read() + APM to PMEM (rand)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'
    else
        figno=$((figno + 4))
    fi
}

TOOLS_PATH=$(pwd)

# prepare a report directory
TIMESTAMP=$(date +%y-%m-%d-%H%M%S)
STAMP=${STAMP:-$TIMESTAMP}
REPORT_DIR=report_$STAMP
mkdir -p $REPORT_DIR
cd $REPORT_DIR

echo "Output directory: $REPORT_DIR"
echo

figures_${SET}
