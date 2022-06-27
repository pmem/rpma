# RPMA benchmark in multilink configuration

The RPMA performance analysis runtime environment allows tests to be carried out in a configuration of multiple parallel connections between nodes.
To execute such a benchmark, changes must be applied.

## Memory Setup
Prepare an additional Device DAX on an RPMA target. Then, in the config.json file, add a new parameter REMOTE_PMEM_PATH_2 and assign it the path to the Device DAX.
```json
"REMOTE_PMEM_PATH_2": "/dev/dax0.1"
```
## Modification of job files
### Adding a second IP address
You have to define one **IP address** (e.g. 192.168.101.4) in the *config.json* file which is used by *client.fio* and *server.fio* (**${serverip}**).
For every *client.fio* and *server.fio* file from the [Fio jobs files](./fio_jobs) folder serverip from `[global]` section shall be moved to the `[client]` or `[server]` section:
```
[global]
...
# serverip=${serverip} # comment out or removed from the [global] section
...
[client] or [server]
...
serverip=${serverip} # ${serverip} is used only by the first client or server
...
```
Additionally, in *client.fio* and *server.fio* you have to encode a different IP address than in *config.json* (e.g. 192.168.102.4).
You have to create a new separate `[client]` or `[server]` section with an explicitly set **server IP** as well:
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
### Adding a second {filename} parameter
For every *server.fio* file from the [Fio jobs files](./fio_jobs) folder you have to add a **filename**
```
[server]
...
filename=${filename1}
```
Additionally, you have to create a new separate `[server]` section with a **filename** ( if you haven't done that before ) :
```
[server]
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
serverip=192.168.102.4
filename=${filename2}
```
### Removed [cpuio] section from the server
For every *server.fio* file from the [Fio jobs files](./fio_jobs) folder you need to remove the `[cpuio]`
```
# [cpuio]
# ioengine=cpuio
# cpuload=${cpuload}
# numjobs=${cores_per_socket}
```
## Performance Tools Source Code Modification (Runner module)
### Second filename

**Note**: File System DAX is not supported.
In the [fio.py](./lib/benchmark/runner/fio.py) file you need to add env of the **filename2** variable in the if condition.
```python
if pmem_path is None:
        env.append('filename1=malloc')
        env.append('filename2=malloc')
        args.extend(['--create_on_open=1'])
    else '/dev/dax' in pmem_path:
        # DeviceDAX
        env.append('filename1={}'.format(pmem_path))
        env.append('filename2={}'.format(self.__config['REMOTE_PMEM_PATH_2']))
```
The code that we have based on is following.
```python
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
```python
# __THREADS_VALUES = [1, 2, 4, 8, 12, 16, 32, 64]
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
```
