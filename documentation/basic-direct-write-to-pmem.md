---
title: basic-direct-write-to-pmem
layout: pmdk
---

# Direct Write to PMem

*Direct Write to PMem* is a readiness of hardware and software configuration which allows effectively and persistently write data to PMem physically attached to one system from another system using RDMA-capable network. How to achive *Direct Write to PMem* capability may differs from system to system because of variety of potential 'interruptions' (e.g. caching) in data's way to be stored persistently on PMem.

## 2nd Generation Intel&reg; Xeon&reg; Scalable Processors

For the 2nd Generation Intel&reg; Xeon&reg; Scalable Processors (**Cascade Lake**) the only configuration item one should take care of is Intel&reg; Data Direct I/O Technology (**DDIO**). **DDIO** assumes the good enough place to effectively store data comming from the RDMA (and any other DMA traffic) is the CPU LLC from which CPU can access it more easily than when it would be stored in DRAM or on PMem [[1]][ddio]. This is beneficial for any other DMA traffic but prevents from achiving *Direct Write to PMem*.

For the **Cascade Lake** processor **DDIO** can be turned off (at the same time enabling *Direct Write to PMem*) at least at two possible levels:

- globally for all DMA traffic in the system or

- for a PCIe Root Port which affects only the DMA traffic coming from PCIe devices physically attached to this PCIe Root Port

Because having **DDIO** turned on is a desirable state for most of the workloads turning it off globally is not considered here. Nonetheless, if one would like to do this please contact your BIOS provider for details.

For more information on **DDIO** and its configuration please see the list of references [[1]][ddio][[2]][dpdk].

### Finding the right PCIe Root Port

You can finding the PCIe Root Port of the network interface knowing its producer and model in a single query:

```sh
$ lspci -vt | grep Mellanox
 +-[0000:17]-+-00.0-[18]--+-00.0  Mellanox Technologies MT27800 Family [ConnectX-5]
 |           |            \-00.1  Mellanox Technologies MT27800 Family [ConnectX-5]
```

The `lspci -vt` command shows a tree-like diagram containing all buses, bridges, devices and connections between them. The top most level of this tree is the PCIe Root Port address. In this case it should be written as `0000:17:00.0`. [[3]][lspci]

### Turning off DDIO

For turning on and off DDIO on per-PCIe Root Port basis please use the [ddio.sh](https://github.com/pmem/rpma/blob/master/tools/ddio.sh) utility available in the librpma repository.

```sh
$ PCIe_Root_Port=0000:17:00.0
$ sudo ./ddio.sh -d $PCIe_Root_Port -q
$ echo $?
1
```

The `1` at the end of the output in this case means the DDIO feature is turned on for this PCIe Root Port which is the default for the **Cascade Lake** platforms. In this case, it is required to turn it off for each **Cascade Lake** system with PMem if you want to have *Direct Write to PMem* capability via RDMA. For details please see [rpma_peer_cfg_set_direct_write_to_pmem(3)](https://pmem.io/rpma/manpages/master/rpma_peer_cfg_set_direct_write_to_pmem.3) and [rpma_flush(3)](https://pmem.io/rpma/manpages/master/rpma_flush.3).

```sh
$ sudo ./ddio.sh -d $PCIe_Root_Port -s disable
$ sudo ./ddio.sh -d $PCIe_Root_Port -q
$ echo $?
0
```

## References

* [1] [Intel&reg; Data Direct I/O Technology][ddio]
* [2] [DPDK: Hardware-Level Performance Analysis of Platform I/O][dpdk]
* [3] [lspci(8)][lspci]

[ddio]: https://www.intel.co.uk/content/www/uk/en/io/data-direct-i-o-technology.html

[dpdk]: https://www.dpdk.org/wp-content/uploads/sites/35/2018/09/Roman-Sudarikov-DPDK_PRC_Summit_Sudarikov.pptx

[lspci]: https://man7.org/linux/man-pages/man8/lspci.8.html