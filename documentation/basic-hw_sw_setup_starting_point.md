---
title: hw-sw-setup-starting-point
layout: pmdk
---

## HW & SW setup starting point description

In order to make the optimal use of your Remote Persistent Memory:

1) Make sure you have all following required HW components [[1]][start-1] and check: how many and what kind of those items do you have? Which socket/slot/channel are they located in?:
   - main system board supporting 2nd generation Intel® Xeon® Scalable processors,
   - 2nd generation Intel® Xeon® Scalable CPUs,
   - Intel® Optane™ persistent memory modules,
   - DRAM DIMMs and
   - RDMA-capable NICs

2) Verify that you have the required Linux kernel version: the Linux NVDIMM/PMem drivers are enabled by default, starting with Linux mainline kernel 4.2. We recommend mainline kernel version 4.19 or later to deliver Reliability, Availability and Serviceability (RAS) features required by the Persistent Memory Development Kit (PMDK) [[2]][start-2]

3) Configure your memory modules according to "Provision Intel® Optane™ DC Persistent Memory in Linux" [[3]][start-3] and "Optimizing Memory Performance" [[4]][start-4] in order to configure your memory properly and optimize its performance.

### References

* [1] [Quick Start Guide Part 1: Persistent Memory Provisioning Introduction - Hardware Requirements][start-1]
* [2] [Quick Start Guide Part 2: Linux Provisioning for Intel® Optane™ Persistent Memory][start-2]
* [3] [Provision Intel® Optane™ DC Persistent Memory in Linux][start-3]
* [4] [Optimizing Memory Performance][start-4]

[start-1]: https://software.intel.com/content/www/us/en/develop/articles/qsg-intro-to-provisioning-pmem.html
[start-2]: https://software.intel.com/content/www/us/en/develop/articles/qsg-part2-linux-provisioning-with-optane-pmem.html
[start-3]: https://software.intel.com/content/www/us/en/develop/videos/provisioning-intel-optane-dc-persistent-memory-modules-in-linux.html
[start-4]: https://pmem.io/rpma/documentation/basic-reqs-optimizing-memory-performance.html
