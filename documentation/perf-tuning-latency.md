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

* Platform and CPU related [[1]][opt-part-1]
    * Ensure that Turbo is on.
    * Disable lower CPU power states. Settings vary among different vendors, so after turning C-states off, you should check whether there are extra settings like C1E, and memory and PCI-e power saving states, which should also be disabled.
    * Check for other settings that might influence performance. This varies greatly by OEM, but should include anything power related, such as fan speed settings.
* PMem-related
    * configure maximum available operating power for your PMem devices **[XXX source and details are missing]**. **Note**: Different sizes of PMem devices have different performance capabilites. If it is important for you, pick the right one for your application. [[2]][pmem-200-brief]

### Not yet confirmed [[1]][opt-part-1]

* Disable hyper-threading to reduce variations in latency (jitter).
* Disable any virtualization options.
* Disable any monitoring options.
* Disable Hardware Power Management, introduced in the Intel® Xeon® processor E5-2600 v4 product family. It provides more control over power management, but it can cause jitter and so is not recommended for latency-sensitive applications.

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

## References

* [1] [Optimizing Computer Applications for Latency: Part 1: Configuring the Hardware][opt-part-1]
* [2] [Intel® Optane™ Persistent Memory 200 Series Brief][pmem-200-brief]

[opt-part-1]: https://software.intel.com/content/www/us/en/develop/articles/optimizing-computer-applications-for-latency-part-1-configuring-the-hardware.html
[pmem-200-brief]: https://www.intel.com/content/www/us/en/products/docs/memory-storage/optane-persistent-memory/optane-persistent-memory-200-series-brief.html