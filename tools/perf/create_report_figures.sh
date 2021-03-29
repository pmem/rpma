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
    echo "usage: $0 report|appendix|cmp"
    echo
    echo "export DATA_PATH=/custom/data/path"
    echo "export STAMP=CUSTOM_REPORT_STAMP"
    echo
    echo "For 'report':"
    echo "export REPORT_MACHINE=<machine>"
    echo "export READ_LAT_MACHINE=<machine>"
    echo "export READ_BW_MACHINE=<machine>"
    echo "export WRITE_LAT_MACHINE=<machine>"
    echo "export WRITE_BW_MACHINE=<machine>"
    echo "export MIX_LAT_MACHINE=<machine>"
    echo "export MIX_BW_MACHINE=<machine>"
    echo
    echo "For 'cmp':"
    echo "export A_MACHINE=<machine>"
    echo "export B_MACHINE=<machine>"
    echo "export CMP_LAT_YAXIS_MAX=<y_max>"
    echo "export CMP_BW_YAXIS_MAX=<y_max>"
    exit 1
}

if [ $# -lt 1 ]; then
    usage "Too few arguments"
elif [ -z "$DATA_PATH" ]; then
	usage "DATA_PATH not set"
fi

case "$1" in
report|appendix|cmp)
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
        "$DATA_PATH/*/ib_read_lat_*" \
        'A' 'ib_read_lat from DRAM' 'single' \
        'Appendix_A_ib_read_lat'

    echo 'Appendix B ib_read_bw bw(bs) bw(th)'
    bw_appendix \
        "$DATA_PATH/*/ib_read_bw-bs_*" \
        'B1' 'ib_read_bw from DRAM' \
        'bs' 'single' \
        'Appendix_B1_ib_read_bw_bs'
    bw_appendix \
        "$DATA_PATH/*/ib_read_bw-th_*" \
        'B2' 'ib_read_bw from DRAM' \
        'threads' 'single' \
        'Appendix_B2_ib_read_bw_th'

    appendix_set '*apm_*read' 'dram' 'C' 'rpma_read()'
    appendix_set '*apm_*read' 'dax' 'D' 'rpma_read()'
    appendix_set '*apm_*write' 'dram' 'E' 'rpma_write()' '(DDIO=ON)'
    appendix_set '*apm_*write' 'dax' 'F' 'APM'
    appendix_set '*gpspm_busy-wait_*write' 'dax' 'G' 'GPSPM-RT'
    appendix_set '*gpspm_no-busy-wait_*write' 'dax' 'H' 'GPSPM'
    mix_desc='70% read, 30% write'
    appendix_set '*apm_*_rw' 'dax' 'I' 'MIX' "($mix_desc, seq)" 'rw'
    appendix_set '*apm_*_randrw' 'dax' 'J' 'MIX' "($mix_desc, rand)" 'rw'
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
    arg_axis="$2"
    title="$3"
    output="$4"
    shift 4

    if [ $# -gt 0 ]; then
        legend=( "$@" )
    else
        legend=( $(files_to_machines $filter) )
    fi

    case "$arg_axis" in
    bs)
        arg_xscale='log'
        ;;
    cpuload)
        arg_xscale='linear'
        ;;
    esac

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
            --arg_axis $arg_axis \
            --arg_xscale $arg_xscale \
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

    axes=('bs' 'threads')
    axis_filters=('bw-bs' 'bw-th')

    for index in "${!axes[@]}"; do
        axis="${axes[$index]}"
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

function bw_cpu_figures()
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

    axes=('cpuload' 'cpuload')
    axis_filters=('bw-cpu' 'bw-cpu-mt')

    for index in "${!axes[@]}"; do
        axis="${axes[$index]}"
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
            --arg_xscale 'linear' \
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

    if [ -n "$REPORT_MACHINE" ]; then
        READ_LAT_MACHINE="$REPORT_MACHINE"
        READ_BW_MACHINE="$REPORT_MACHINE"
        WRITE_LAT_MACHINE="$REPORT_MACHINE"
        WRITE_BW_MACHINE="$REPORT_MACHINE"
        MIX_LAT_MACHINE="$REPORT_MACHINE"
        MIX_BW_MACHINE="$REPORT_MACHINE"
    fi

    echo "READ LAT"
    set_data_path READ_LAT_MACHINE
    if [ "$data_path" != 'skip' ]; then
        lat_figures \
            "$data_path/ib_read_lat_* $data_path/*apm_*read_lat_*dram*" \
            'bs' \
            'ib_read_lat vs rpma_read() from DRAM' \
            'ib_read_lat_vs_rpma_read_dram' \
            'ib_read_lat' 'rpma_read() rand' 'rpma_read() seq'

        lat_figures \
            "$data_path/*read_lat_*dram* $data_path/*read_lat_*dax*" \
            'bs' \
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
            "$data_path/*apm_*write_lat_*dram* $data_path/*apm_*write_lat_*dax*" \
            'bs' \
            'APM to DRAM (DDIO=ON) vs to PMEM' \
            'apm_dram_vs_pmem' \
            'DRAM rand' 'DRAM seq' 'PMEM rand' 'PMEM seq'

        lat_figures \
            "$data_path/*apm_*write_lat_*dax* $data_path/*gpspm_*write_lat_*dax*" \
            'bs' \
            'APM to PMEM vs GPSPM(-RT) to PMEM' \
            'apm_pmem_vs_gpspm_pmem' \
            'APM rand' 'APM seq' \
            'GPSPM-RT rand' 'GPSPM-RT seq' \
            'GPSPM rand' 'GPSPM seq'

        lat_figures \
            "$data_path/*apm_*write_lat-cpu_*dax* $data_path/*gpspm_*write_lat-cpu_*dax*" \
            'cpuload' \
            'APM to PMEM vs GPSPM(-RT) to PMEM' \
            'apm_pmem_vs_gpspm_pmem_cpuload' \
            'APM rand' 'APM seq' \
            'GPSPM-RT rand' 'GPSPM-RT seq' \
            'GPSPM rand' 'GPSPM seq'

        figno=$((figno - 2)) # XXX remove when cpuload will be added to the report
    else
        figno=$((figno + 2 * 3))
    fi

    echo "WRITE BW"
    set_data_path WRITE_BW_MACHINE
    if [ "$data_path" != 'skip' ]; then
        bw_figures \
            "$data_path/*apm_*write_{axis}_*dram* $data_path/*apm_*write_{axis}_*dax*" \
            'APM to DRAM (DDIO=ON) vs to PMEM' \
            'apm_dram_vs_pmem' \
            'DRAM rand' 'DRAM seq' 'PMEM rand' 'PMEM seq'

        bw_figures \
            "$data_path/*apm_*write_{axis}_*dax* $data_path/*gpspm_*write_{axis}_*dax*" \
            'APM to PMEM vs GPSPM(-RT) to PMEM' \
            'apm_pmem_vs_gpspm_pmem' \
            'APM rand' 'APM seq' \
            'GPSPM-RT rand' 'GPSPM-RT seq' \
            'GPSPM rand' 'GPSPM seq'

        bw_cpu_figures \
            "$data_path/*apm_*write_{axis}_*dax* $data_path/*gpspm_*write_{axis}_*dax*" \
            'APM to PMEM vs GPSPM(-RT) to PMEM' \
            'apm_pmem_vs_gpspm_pmem_cpuload' \
            'APM rand' 'APM seq' \
            'GPSPM-RT rand' 'GPSPM-RT seq' \
            'GPSPM rand' 'GPSPM seq'

        figno=$((figno - 2)) # XXX remove when cpuload will be added to the report
    else
        figno=$((figno + 2 * 3))
    fi

    echo 'MIX LAT'
    set_data_path MIX_LAT_MACHINE
    if [ "$data_path" != 'skip' ]; then
        lat_figures \
            "$data_path/*apm_*_read_lat_*dax* $data_path/*apm_*_write_lat_*dax* $data_path/*apm_*_rw_lat_*dax*" \
            'bs' \
            'MIX against PMEM vs rpma_read() + APM to PMEM (seq)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'

        lat_figures \
            "$data_path/*apm_*_randread_lat_*dax* $data_path/*apm_*_randwrite_lat_*dax* $data_path/*apm_*_randrw_lat_*dax*" \
            'bs' \
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
            "$data_path/*apm_*_read_{axis}*dax* $data_path/*apm_*_write_{axis}*dax* $data_path/*apm_*_rw_{axis}*dax*" \
            'MIX against PMEM vs rpma_read() + APM to PMEM (seq)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'
        bw_figures \
            "$data_path/*apm_*_randread_{axis}*dax* $data_path/*apm_*_randwrite_{axis}*dax* $data_path/*apm_*_randrw_{axis}*dax*" \
            'MIX against PMEM vs rpma_read() + APM to PMEM (rand)' \
            'mix_pmem_vs_rpma_read_apm_pmem' \
            'rpma_read()' 'APM to PMEM' 'MIX read' 'MIX write'
    else
        figno=$((figno + 4))
    fi
}

function lat_cmp()
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

    if [ -n "$CMP_LAT_YAXIS_MAX" ]; then
        yaxis_max="--yaxis_max $CMP_LAT_YAXIS_MAX"
    fi

    echo "- $title"
    echo_filter $filter
    layouts=('lat_avg' 'lat_pctl_99999')
    title_prefixes=( \
        'Latency' \
        'Latency (99.999% percentiles)')
    for index in "${!layouts[@]}"; do
        layout="${layouts[$index]}"
        title_prefix="${title_prefixes[$index]}"
        printf -v figno_name "%0${N_DIGITS}d" $figno
        $TOOLS_PATH/csv_compare.py \
            --output_title "Fig. $title_prefix: $title" \
            --output_layout "$layout" \
            --arg_axis 'bw' \
            --arg_xscale 'log' \
            $yaxis_max \
            --output_with_table \
            --legend "${legend[@]}" \
            --output_file "Figure_${figno_name}_${output}_${layout}.png" \
            $filter
        figno=$((figno + 1))
    done
    echo
}

function bw_cmp()
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

    if [ -n "$CMP_BW_YAXIS_MAX" ]; then
        yaxis_max="--yaxis_max $CMP_BW_YAXIS_MAX"
    fi

    axes=('bs' 'threads')
    axis_filters=('bw-bs' 'bw-th')

    for index in "${!axes[@]}"; do
        axis="${axes[$index]}"
        axis_filter="${axis_filters[$index]}"
        # replace '{axis}' with actual filter by axis
        eff_filter="${filter//\{axis\}/${axis_filter}}"
        echo "- $title - bw($axis)"
        printf -v figno_name "%0${N_DIGITS}d" $figno
        echo_filter $eff_filter
        $TOOLS_PATH/csv_compare.py \
            --output_title "Fig. Bandwidth: $title" \
            --output_layout 'bw' \
            --arg_axis "$axis" \
            --arg_xscale 'log' \
            $yaxis_max \
            --output_with_table \
            --legend "${legend[@]}" \
            --output_file "Figure_${figno_name}_${output}_${axis_filter}.png" \
            $eff_filter
        figno=$((figno + 1))
    done
    echo
}

function figures_cmp()
{
    # a global Figure indexer
    figno=1

    if [ -z "$A_MACHINE" ]; then
        echo "SKIP: A_MACHINE not set"
        exit 1
    fi

    if [ -z "$B_MACHINE" ]; then
        echo "SKIP: B_MACHINE not set"
        exit 1
    fi

    echo "'$A_MACHINE' vs '$B_MACHINE'"
    a_data_path="$DATA_PATH/MACHINE_$A_MACHINE"
    b_data_path="$DATA_PATH/MACHINE_$B_MACHINE"

    lat_cmp \
        "$a_data_path/*apm_*randread_lat*dram* $b_data_path/*apm_*randread_lat*dram*" \
        'random rpma_read() from DRAM' \
        'rpma_read_dram' \
        "$A_MACHINE" "$B_MACHINE"

    for machine in $A_MACHINE $B_MACHINE; do
        lat_cmp \
            "$DATA_PATH/MACHINE_$machine/*randwrite_lat*dax*" \
            "${machine}: APM to PMEM vs GPSPM to PMEM" \
            "${machine}_apm_vs_gpspm_pmem" \
            'APM' 'GPSPM'

        bw_cmp \
            "$DATA_PATH/MACHINE_$machine/*randwrite_{axis}*dax*" \
            "${machine}: APM to PMEM vs GPSPM to PMEM" \
            "${machine}_apm_vs_gpspm_pmem" \
            'APM' 'GPSPM'

        bw_cmp \
            "$DATA_PATH/MACHINE_$machine/*apm_randwrite_{axis}*dram* $DATA_PATH/MACHINE_$machine/*apm_randwrite_{axis}*dax*" \
            "${machine}: APM to DRAM (DDIO=ON) vs to PMEM" \
            "${machine}_apm_dram_vs_pmem" \
            'DRAM' 'PMEM'
    done

    for pm in apm gpspm; do
        PM=${pm^^}

        lat_cmp \
            "$a_data_path/*${pm}_*randwrite_lat*dax* $b_data_path/*${pm}_*randwrite_lat*dax*" \
            "${PM} to PMEM" \
            "${pm}_pmem" \
            "$A_MACHINE" "$B_MACHINE"

        bw_cmp \
            "$a_data_path/*${pm}_*randwrite_{axis}*dax* $b_data_path/*${pm}_*randwrite_{axis}*dax*" \
            "${PM} to PMEM" \
            "${pm}_pmem" \
            "$A_MACHINE" "$B_MACHINE"
    done
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
