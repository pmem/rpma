# config.json parameters

Mandatory and optional parameters available to control the `report_bench.py` behaviour via configuration file (`--config`).

## Mandatory parameters

All mandatory parameters are listed in the [`config.json.example`](./config.json.example).

## Optional parameters

For Cascade Lake platforms you can allow to set up DDIO automatically during the benchmark execution. To make it possible you have to allow password-less sudo for the user you are using on the target system and provide a PCIe Root Port of the RNIC you are using on the target system. For details please read [Direct Write to PMem][direct-write]. e.g.:

```json
    "REMOTE_SUDO_NOPASSWD": true,
    "REMOTE_RNIC_PCIE_ROOT_PORT": "0000:17:00.0"
```

[direct-write]: https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html

### Collecting SAR data

```json
    "REMOTE_ANOTHER_NUMA": 1,
    "REMOTE_RESULTS_DIR": "path",
    "REMOTE_CMD_PRE": "rm -f ${REMOTE_RESULTS_DIR}sar.dat; numactl -N ${REMOTE_ANOTHER_NUMA} sar -u -P ${REMOTE_JOB_NUMA_CPULIST} -o ${REMOTE_RESULTS_DIR}sar.dat 5 > /dev/null",
    "REMOTE_CMD_POST": "sleep 10; killall -9 sar; sadf -d -- -u -P ${REMOTE_JOB_NUMA_CPULIST} ${REMOTE_RESULTS_DIR}sar.dat > ${REMOTE_RESULTS_DIR}sar_${RUN_NAME}.csv"
```

### Collecting EMON data

```json
    "REMOTE_ANOTHER_NUMA": 1,
    "REMOTE_RESULTS_DIR": "path",
    "EVENT_LIST": "path",
    "REMOTE_CMD_PRE": "source /opt/intel/sep/sep_vars.sh; numactl -N ${REMOTE_ANOTHER_NUMA} emon -i ${EVENT_LIST} > ${REMOTE_RESULTS_DIR}${RUN_NAME}_emon.dat",
    "REMOTE_CMD_POST": "sleep 10; source /opt/intel/sep/sep_vars.sh; emon -stop"
```

### Debug options

For functional testing, you can run each benchmark only for a brief period of time.

```json
    "SHORT_RUNTIME": true
```

For debugging purposes, you can attach tracers to each or both ends of the connections e.g. `gdbserver`.

```json
    "TRACER": "gdbserver localhost:2345",
    "REMOTE_TRACER": "gdbserver localhost:2345"
```

During in-depth analysis of the benchmarking process itself, you may find useful these two options. Where `DO_NOTHING` allows running all the machinery but prevents the actual benchmark binaries to run. Whereas `DUMP_CMDS` allows dumping all commands running the benchmark binaries. You can use either or both of them.

```json
    "DO_NOTHING": true,
    "DUMP_CMDS": true
```
