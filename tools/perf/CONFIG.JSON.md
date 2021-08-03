## Additional parameters that can be used to configure the report_bench.py tool.
```json
    "REMOTE_SUDO_NOPASSWD": true,
    "REMOTE_RNIC_PCIE_ROOT_PORT": "0000:17:00.0",
    "FORCE_REMOTE_DIRECT_WRITE_TO_PMEM": true,
    "BUSY_WAIT_POLLING": true,
    "CPU_LOAD_RANGE": "00_99",
    "REMOTE_ANOTHER_NUMA": 1,
    "REMOTE_RESULTS_DIR": "path",
    "OUTPUT_FILE": "output_file.csv"
```
### In case you would like to collect sar data:
```json
    "REMOTE_CMD_PRE": "rm -f ${REMOTE_RESULTS_DIR}sar.dat; numactl -N ${REMOTE_ANOTHER_NUMA} sar -u -P ${REMOTE_JOB_NUMA_CPULIST} -o ${REMOTE_RESULTS_DIR}sar.dat 5 > /dev/null",
    "REMOTE_CMD_POST": "sleep 10; killall -9 sar; sadf -d -- -u -P ${REMOTE_JOB_NUMA_CPULIST} ${REMOTE_RESULTS_DIR}sar.dat > ${REMOTE_RESULTS_DIR}sar_${RUN_NAME}.csv"
```
### In case you would like to collect emon data:
```json
    "EVENT_LIST": "path",
    "REMOTE_CMD_PRE": "source /opt/intel/sep/sep_vars.sh; numactl -N ${REMOTE_ANOTHER_NUMA} emon -i ${EVENT_LIST} > ${REMOTE_RESULTS_DIR}${RUN_NAME}_emon.dat",
    "REMOTE_CMD_POST": "sleep 10; source /opt/intel/sep/sep_vars.sh; emon -stop",
```
### Debug:
```json
    "SHORT_RUNTIME": false,
    "TRACER": "gdbserver localhost:2345",
    "REMOTE_TRACER": "gdbserver localhost:2345",
    "DO_NOTHING": false,
    "DUMP_CMDS": false,
```