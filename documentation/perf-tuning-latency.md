---
title: perf-tuning-latency
layout: pmdk
---

# Performance - Tuning for latency (WIP)

A collection of the best methods, practices, and configuration values known for delivering the smallest possible latency in a consistent and predictable way for the workloads built on the librpma library.

This guide is Work-in-progress.

## Disclaimer

The *Performance - Tuning for latency* aims to collect all tested and proven procedures with known beneficial effects on the latency of the workloads built on the librpma library. Nonetheless, each change of configuration should be set up and tested in a testing environment before being applied to a production system. Backing up all data and configuration settings prior to tuning is also recommended.

## BIOS settings

**XXX**: Can we describe this in any greater detail?

* Platform and CPU related [[1.1]][opt-part-1]
    * Disable lower CPU power states: C-states[[1.2]][power-states], C1E, and memory and PCI-e power saving states. Settings vary from vendor to vendor so some of them may not be available for you.
    * Check for other settings that might influence performance. This varies greatly by OEM, but should include anything power related, such as fan speed settings (more is better).
* PMem-related
    * configure maximum available operating power for your PMem devices **[XXX source and details are missing]**. **Note**: Different sizes of PMem devices have different performance capabilites. If it is important for you, pick the right one for your application. [[1.3]][pmem-200-brief]

### Not yet confirmed [[1.1]][opt-part-1]

* Ensure that *Intel® Turbo Boost*[[1.4]][turbo] is on.
* Disable hyper-threading to reduce variations in latency (jitter).
* Disable any virtualization options.
* Disable any monitoring options.
* Disable Hardware Power Management, introduced in the Intel® Xeon® processor E5-2600 v4 product family. It provides more control over power management, but it can cause jitter and so is not recommended for latency-sensitive applications.

### References

* [1.1] [Optimizing Computer Applications for Latency: Part 1: Configuring the Hardware][opt-part-1]
* [1.2] [Intel: Power Management States: P-States, C-States, and Package C-States][power-states]
* [1.3] [Intel® Optane™ Persistent Memory 200 Series Brief][pmem-200-brief]
* [1.4] [Intel® Turbo Boost Technology 2.0 - Higher Performance When You Need It Most][turbo]

[opt-part-1]: https://software.intel.com/content/www/us/en/develop/articles/optimizing-computer-applications-for-latency-part-1-configuring-the-hardware.html
[power-states]: https://software.intel.com/content/www/us/en/develop/articles/power-management-states-p-states-c-states-and-package-c-states.html
[pmem-200-brief]: https://www.intel.com/content/www/us/en/products/docs/memory-storage/optane-persistent-memory/optane-persistent-memory-200-series-brief.html
[turbo]: https://www.intel.com/content/www/us/en/architecture-and-technology/turbo-boost/turbo-boost-technology.html

## CPU

**Note**: It is very probable this chapter will be updated in the near future.

### CPU driver

To disable `intel_pstate` CPU driver **[XXX why? the source is missing]** edit your `/etc/default/grub` and add the following to the kernel command line:

```sh
GRUB_CMDLINE_LINUX_DEFAULT="intel_pstate=disable"
```

Rebuild the grub.cfg file as follows (make sure you use the path adjusted to your OS):

* On BIOS-based machines, issue the following command:

```sh
sudo grub2-mkconfig -o /boot/grub2/grub.cfg
```

* On UEFI-based machines, issue the following command:

```sh
sudo grub2-mkconfig -o /boot/efi/EFI/redhat/grub.cfg
```

This change requires a reboot to take effect.

### CPU scaling governor

Set the most performant scaling governor:

```sh
sudo cpupower frequency-set --governor performance
```

### CPU frequency

List available frequencies and pick the biggest one.

```sh
cpupower frequency-info | grep steps
  available frequency steps:  2.30 GHz, 2.30 GHz, 2.20 GHz, 2.10 GHz, 2.00 GHz, 1.90 GHz, 1.80 GHz, 1.70 GHz, 1.60 GHz, 1.50 GHz, 1.40 GHz, 1.30 GHz, 1.20 GHz, 1.10 GHz, 1000 MHz
```

Set the frequency range to the single biggest available value:

```sh
FREQ=2.3Ghz
sudo cpupower frequency-set --min $FREQ
sudo cpupower frequency-set --max $FREQ
```
## RDMA-capable network interface

XXX placeholder

## Running workloads

Before running your workload make sure your PMem and RDMA-capable network interface you want to use belong to the same NUMA node and it is the NUMA node you want to run your software on.

**XXX** more details needed here.

When you have everything in check you can run your software attaching it to the dedicated NUMA node.

```sh
$ NUMA_NODE=0
$ numactl -N $NUMA_NODE ./fio ~/fio_jobs/librpma-client.fio
```
