---
title: basic-direct-write-to-pmem
layout: pmdk
---

# Direct write to PMem

Direct write to PMem is a basic capability provided by the librpma library.

This guide is Work-in-progress.

**XXX** elaborate

## Intel® Data Direct I/O Technology (DDIO) [[1]][xxx-ddio]

**XXX** Here probably should go a part of the [DDIO - The journey to the datasheet and back][xxx-ddio] which describes why users may be very interested in turning DDIO off on the per-PCIe Root Port basis.

## Prepare for direct write to PMem

It is hard to show a general way to obtain an address of the PCIe Root Port you desire. Let’s assume you have a Mellanox RDMA-capable Network Adapter plugged into PCIe slot. How to find out an address of PCIe Root Port of the Mellanox RNIC? If you ask me I will argue the simplest way is as follows:

```sh
$ lspci -vt | grep Mellanox
 +-[0000:17]-+-00.0-[18]--+-00.0  Mellanox Technologies MT27800 Family [ConnectX-5]
 |           |            \-00.1  Mellanox Technologies MT27800 Family [ConnectX-5]
```

The [lspci(8)](https://man7.org/linux/man-pages/man8/lspci.8.html) manual pages will tell you that -t show a tree-like diagram containing all buses, bridges, devices and connections between them whereas `-v` displays detailed information about all devices. So we end up with a tree like structure of named devices which allows us track down the PCIe Root Port and its address. In this case it should be written as `0000:17:00.0`.

For easy turning on and off DDIO on per-PCIe Root Port basis you can use [ddio.sh](https://github.com/pmem/rpma/blob/master/tools/ddio.sh) available in the RPMA repository.

```sh
$ PCIe_Root_Port=0000:17:00.0
$ sudo ./ddio -d $PCIe_Root_Port -q
$ echo $?
1
```

The `1` at the end of the output in this case means the DDIO feature is turned on for this PCIe Root Port which is the default for today Intel platforms. But you want to turn it off for each system with PMem if you want to write to PMem directly via RDMA. For details please see [rpma_peer_cfg_set_direct_write_to_pmem(3)](https://pmem.io/rpma/manpages/master/rpma_peer_cfg_set_direct_write_to_pmem.3) and [rpma_flush(3)](https://pmem.io/rpma/manpages/master/rpma_flush.3).

```sh
$ sudo /.ddio -d $PCIe_Root_Port -s disable
$ sudo ./ddio -d $PCIe_Root_Port -q
$ echo $?
0
```

## References

* [1] [XXX DDIO - The journey to the datasheet and back][xxx-ddio]

[xxx-ddio]: http://janekmi.github.io/2020/06/21/ddio.html
