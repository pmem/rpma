# RPMA benchmark in multilink configuration

The RPMA performance analysis runtime environment allows tests to be carried out in a configuration of multiple parallel connections between nodes.
To execute such a benchmark, changes must be applied.

## Memory Setup
You create 2 namespaces in 1 region in PMem.
Finally, you have in add the second `/dev/dax` in the *config.json* file.
```sh
"REMOTE_PMEM_PATH_2": "/dev/dax0.1"
```
## Modification of job files
### Adding second IP address
You have to define one **IP address** (e.g. 192.168.101.4) in the *config.json* file which is used by *client.fio* and *server.fio* (**${serverip}**).
For every *client.fio* and *server.fio* file from the folder [Fio jobs files](./fio_jobs) serverip from `[global]` section shall be moved to the `[client]` or `[server]` section:
```
[global]
...
# serverip=${serverip} #comment out or removed from [global] section
...
[client] or [server]
...
serverip=${serverip} # ${serverip} is used only by the first client or server
...
```
Additionally, in *client.fio* and *server.fio* you have to encode a different IP address than in *config.json* (e.g. 192.168.102.4).
Additionally, we have to create a new separate `[client]` or `[server]` section with a explicitly set **server IP**:
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
or
```
[server]
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
serverip=192.168.102.4
```
### Adding second {filename} parameter
For every *server.fio* file from the folder [Fio jobs files](./fio_jobs) you have to add **filename**
```
[server]
...
filename=${filename1}
```
Additionally, we have to create a new separate `[server]` ( if you haven't created before ) section with a **filename**:
```sh
[server]
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
serverip=192.168.102.4
filename=${filename2}
```
### Removed [cpuio] section from the server
For every *server.fio* file from the folder [Fio jobs files](./fio_jobs) you need to remove the `[cpuio]`
```
# [cpuio]
# ioengine=cpuio
# cpuload=${cpuload}
# numjobs=${cores_per_socket}
```
## Performance Tools Source Code Modification (Runner module)
### Second filename

**Note**: File System DAX is not supported.
In the [fio.py](./lib/benchmark/runner/fio.py) file you need to add env of **filename2** variable in the if condition.
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
### Finaly tuning
We recommend reducing the number of threads.
```sh
# __THREADS_VALUES = [1, 2, 4, 8, 12, 16, 32, 64]
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
```
