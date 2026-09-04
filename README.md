# BATOU-CC: Bare-Metal Neural Network C Compiler

> *"Heavy cybernetic execution for resource-constrained edge hardware."*

`BATOU-CC` is a lightweight, zero-dependency ahead-of-time (AOT) neural network compiler written in C++17. It ingests high-level model specifications (JSON) and emits standalone, human-readable, zero-heap C code targeted at bare-metal embedded systems and microcontrollers.

---

## Features

- **Operator Fusion:** Combines adjacent execution nodes (`Conv2D` + `ReLU`) into unified execution calls to eliminate memory round-trips.
- **Ping-Pong Memory Allocator:** Static workspace optimization alternating between two fixed global buffers (`workspace_A` and `workspace_B`), eliminating dynamic allocation (`malloc`).
- **Zero Runtime Overhead:** Emits clean C code targeting a minimal C math runtime (`kernels.h`).

---

## Execution Pipeline

```text
 JSON Model Spec
        │
        ▼
 Graph Parsing (IR Construction)
        │
        ▼
 Fusion Optimization Pass (Conv2D + ReLU)
        │
        ▼
 Static Memory Planning (Ping-Pong Allocation)
        │
        ▼
 Standalone C File Emission (`model_compiled.c`)