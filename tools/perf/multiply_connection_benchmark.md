# RPMA benchmark in multilink configuration

The RPMA performance analysis runtime environment allows tests to be carried out in a configuration of multiple parallel connections between nodes.
To execute such a benchmark, changes must be applied.

## Second ip address
We have to add one `IP address` (e.g. **192.168.101.4**) in the `config.json` file which is used by `*client.fio` and `*server.fio`. 
Moreover we have to hardcode **`another`** (e.g. **192.168.102.4**) `IP address` in these files.

For every `*client.fio` file from the folder `/rpma/tools/perf/fio_jobs` `serverip` from `[global]` section shall be moved to the `[client]` section:
```
[global]
...
# serverip=${serverip} #comment out or removed from [global] section
...
[client] 
...
serverip=${serverip} # ${serverip} is used only by the first client
...
```
Additionally, we have to create a new separate `[client]` section with a explicitly set `server IP`:
```
[client]
sync=${sync}
numjobs=${numjobs}
group_reporting=1
iodepth=${iodepth}
readwrite=${readwrite}
rwmixread=70 # valid only for the 'rw' and 'randrw' readwrite modes
blocksize=${blocksize}
ramp_time=${ramp_time}
time_based
runtime=${runtime}
serverip=192.168.102.4
```

For every `*server.fio` file from the folder `rpma/tools/perf/fio_jobs` `serverip` from `[global]` section shall be moved to the `[server]` section: 
```
[global]
...
# serverip=${serverip} #comment out or removed from [global] section
...

[server]
...
serverip=${serverip} # ${serverip} is used only by the first server
```
Additionally, we have to create a new separate `[server]` section with a explicitly set `server IP`:
```sh
[server]
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
serverip=192.168.102.4
```
## Second filename
**Note**: file system DAX is not supported.
In the `/rpma/tools/perf/lib/benchmark/runner/fio.py` file you need to add env of `filename2` variable in the if condition.
```sh
 if pmem_path is None:
            env.append('filename1=malloc')
            env.append('filename2=malloc')
            args.extend(['--create_on_open=1'])
         else '/dev/dax' in pmem_path:
            # DeviceDAX
            env.append('filename1={}'.format(pmem_path))
            env.append('filename2={}'.format(self.__config['REMOTE_PMEM_PATH_2']))
```
Below is the code that we have based on
```sh
if pmem_path is None:
            args.extend(['--filename=malloc', '--create_on_open=1'])
        elif '/dev/dax' in pmem_path:
            # DeviceDAX
            args.append('--filename={}'.format(pmem_path))
        else:
            args.append('--filename_format={}.\\$jobnum'.format(pmem_path))
```


For every `*server.fio` file from the folder `rpma/tools/perf/fio_jobs` you have to add `filename`
```
[server]
...
filename=${filename1}
```
Additionally, we have to create a new separate `[server]` ( if you haven't created before ) section with a `filename`:
```sh
[server]
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
serverip=192.168.102.4
filename=${filename2}
```

## Removed [cpuio] section from the server
For every `*server.fio` file from the folder `rpma/tools/perf/fio_jobs` you need to remove the `cpuio`
```
# [cpuio]
# ioengine=cpuio
# cpuload=${cpuload}
# numjobs=${cores_per_socket}
```

## Finaly tuning

We recommend reducing the number of threads.
```sh
# __THREADS_VALUES = [1, 2, 4, 8, 12, 16, 32, 64]
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
```
We create 2 namespaces in 1 region in PMem.
Finally we have to add our `/dev/dax` in the `config.json` file.
```sh
"REMOTE_PMEM_PATH_2": "/dev/dax0.1"
```
