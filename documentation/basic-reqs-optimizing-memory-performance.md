---
title: optimizing-memory-performance
layout: pmdk
---

## Optimizing Memory Performance

In order to optimize memory performance:

1) Use identical DIMM types throughout the platform (same size, speed and number of ranks)
2) Maximize the same number of channels populated in each memory controller

The "balanced" platform configuration is as follows:
   - all available memory channels and sockets populated equally
   - identical DIMMs in all locations (size/speed/rank)

The "near-balanced" platform configuration is as follows:
   - all available memory channels and sockets populated equally
   - identical DIMMs in each "row", but different sized DIMMs in row #1 vs. row #2

All recommended topologies of DIMM populations are described in detail
in the 1.2.4.1 chapter "Recommended Topologies" of
[Intel Optane DC Persistent Memory Quick Start Guide](https://www.intel.com/content/dam/support/us/en/documents/memory-and-storage/data-center-persistent-mem/Intel-Optane-DC-Persistent-Memory-Quick-Start-Guide.pdf)
