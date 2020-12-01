#!/bin/bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# rpma_read_lat.py -- a single-sided RPMA latency benchmark (EXPERIMENTAL)
#
# Spawns both server and the client, collects the results for multile data
# sizes (1KiB, 4KiB, 64KiB) and generates a single CSV file with all
# the resutls.
#

DATA_SIZE="1024 4096 65536"
TIMESTAMP=$(date +%y-%m-%d-%H%M%S)
TEMP_JSON=/dev/shm/rpma_read_lat_temp-${TIMESTAMP}.json
TEMP_CSV=/dev/shm/rpma_read_lat_temp-${TIMESTAMP}.csv
OUTPUT=rpma_read_lat-${TIMESTAMP}.csv
LOG_ERR=/dev/shm/rpma_read_lat-${TIMESTAMP}.log

function usage()
{
    echo "Error: $1"
    echo
    echo "usage: $0 <SERVER_IP>"
    echo
    echo "export JOB_NUMA=0"
    echo "export FIO_PATH=/custom/fio/path"
    echo 
    echo "export REMOTE_USER=user"
    echo "export REMOTE_PASS=pass"
    echo "export REMOTE_JOB_NUMA=0"
    echo "export REMOTE_FIO_PATH=/custom/fio/path"
    echo "export REMOTE_JOB_PATH=/custom/jobs/path"
    echo "export REMOTE_JOB_MEM=mmap:/path/to/mem"
    exit 1
}

if [ "$#" -lt 1 ]; then
    usage "Too few arguments"
elif [ -z "$JOB_NUMA" ]; then
    usage "JOB_NUMA not set"
elif [ -z "$REMOTE_USER" ]; then
    usage "REMOTE_USER not set"
elif [ -z "$REMOTE_PASS" ]; then
    usage "REMOTE_PASS not set"
elif [ -z "$REMOTE_JOB_NUMA" ]; then
    usage "REMOTE_JOB_NUMA not set"
fi

SERVER_IP=$1

rm -f $LOG_ERR $OUTPUT

if [ -z "$REMOTE_JOB_PATH" ]; then
    REMOTE_JOB_PATH=/dev/shm/librpma-server-${TIMESTAMP}.fio
fi
if [ -z "$REMOTE_JOB_MEM" ]; then
    REMOTE_JOB_MEM=mmap
fi

for ds in $DATA_SIZE; do
    # copy config to the server
    sshpass -p "$REMOTE_PASS" scp ./fio_jobs/librpma-server.fio \
        $REMOTE_USER@$SERVER_IP:$REMOTE_JOB_PATH
    # run the server
    sshpass -p "$REMOTE_PASS" ssh $REMOTE_USER@$SERVER_IP \
        "bindname=$SERVER_IP mem=$REMOTE_JOB_MEM num_conns=1 \
        numactl -N $REMOTE_JOB_NUMA \
            ${REMOTE_FIO_PATH}fio $REMOTE_JOB_PATH > $LOG_ERR" 2>>$LOG_ERR &
    sleep 1

    echo "[size: $ds]"
    # run FIO
    hostname=$SERVER_IP blocksize=$ds \
        numactl -N $JOB_NUMA ${FIO_PATH}fio \
        ${JOBS_PATH}librpma-client-read.fio --output-format=json+ \
        > $TEMP_JSON
    [ "$?" -ne 0 ] && {
        "Error: FIO job failed"
        exit 1
    }
    # convert JSON to CSV
    ./fio_json2csv.py $TEMP_JSON --output_file $TEMP_CSV \
        --op read
    # append CSV to the output
    cat $TEMP_CSV >> $OUTPUT
done

# remove redundant headers
cat $OUTPUT | head -1 > $TEMP_CSV
cat $OUTPUT | grep -v 'lat' >> $TEMP_CSV
cp $TEMP_CSV $OUTPUT
