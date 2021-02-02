---
title: optimizing-memory-performance
layout: pmdk
---

### Remote Persistent Memory Access

## Optimizing Memory Performance for Remote Memory Access

In order to optimize memory performance:

1) Use identical DIMM types throughout the platform:
   - same size, speed and number of ranks

2) Maximize the same number of channels populated in each memory controller

3) Use a "balanced" platform configuration:
   - all available memory channels populated equally
   - identical DIMMs in all locations (size/speed/rank)

4) Use a "near-balanced" platform configuration:
   - all available memory channels and sockets populated equally
   - identical DIMMs in each "row", but different sized DIMMs in row #1 vs. row #2
