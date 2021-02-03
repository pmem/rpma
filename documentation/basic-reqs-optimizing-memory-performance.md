---
title: optimizing-memory-performance
layout: pmdk
---

## Optimizing Memory Performance

In order to optimize memory performance:

1) Use identical DIMM types throughout the platform:
   - same size, speed and number of ranks

2) Maximize the same number of channels populated in each memory controller

3) Use:

   a) a "balanced" platform configuration:

   - identical DIMMs in all locations (size/speed/rank)

   or:

   b) a "near-balanced" platform configuration:
   - identical DIMMs in each "row", but different sized DIMMs in row #1 vs. row #2
