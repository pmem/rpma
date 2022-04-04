---
title: basic-direct-write-to-pmem-ICX
layout: pmdk
---

# Direct Write to PMem

*Direct Write to PMem* is a readiness of hardware and software configuration which allows effectively and persistently write data to PMem physically attached to one system from another system using RDMA-capable network. How to achive *Direct Write to PMem* capability may differs from system to system because of variety of potential 'interruptions' (e.g. caching) in data's way to be stored persistently on PMem.

## 3rd Generation Intel&reg; Xeon&reg; Scalable Processors

For the 3rd Generation Intel&reg; Xeon&reg; Scalable Processors (**Ice Lake**) the only configuration item one should take care of is Intel&reg; Data Direct I/O Technology (**DDIO**). **DDIO** assumes the good enough place to effectively store data comming from the RDMA (and any other DMA traffic) is the CPU LLC from which CPU can access it more easily than when it would be stored in DRAM or on PMem [[1]][ddio]. This is beneficial for any other DMA traffic but prevents from achiving *Direct Write to PMem*.

For the **Ice Lake** processor **DDIO** can be turned off (at the same time enabling *Direct Write to PMem*) globally for all DMA traffic in the system, please contact your BIOS provider for details.

For more information on **DDIO** and its configuration please see the list of references [[1]][ddio][[2]][dpdk].

## References

* [1] [Intel&reg; Data Direct I/O Technology][ddio]
* [2] [DPDK: Hardware-Level Performance Analysis of Platform I/O][dpdk]

[ddio]: https://www.intel.co.uk/content/www/uk/en/io/data-direct-i-o-technology.html
[dpdk]: https://www.dpdk.org/wp-content/uploads/sites/35/2018/09/Roman-Sudarikov-DPDK_PRC_Summit_Sudarikov.pptx

## Disclaimer

Performance varies by use, configuration and other factors.

No product or component can be absolutely secure.

Your costs and results may vary.

Intel technologies may require enabled hardware, software or service activation.

Intel disclaims all express and implied warranties, including without limitation, the implied warranties of merchantability, fitness for a particular purpose, and non-infringement, as well as any warranty arising from course of performance, course of dealing, or usage in trade.
