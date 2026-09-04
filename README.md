# BATOU-CC

### Zero-Heap AOT C++ Compiler for Embedded Neural Networks

**BATOU-CC** (*Bare-Metal Architecture Target Optimization Utility*) is an **Ahead-of-Time (AOT) C++17 neural network compiler** designed for microcontrollers, bare-metal systems, and resource-constrained real-time operating systems (RTOS).

Instead of relying on a runtime interpreter, dynamic graph parsing, or heap allocation, BATOU-CC **compiles neural-network graphs directly into static C inference code** with deterministic memory requirements.

> **Compile the graph once. Run inference without a heap.**

---

## Architecture

BATOU-CC transforms a high-level neural-network graph into a standalone, statically allocated inference implementation:

```text
                    Neural Network Graph
                         model.json
                             │
                             ▼
                  ┌─────────────────────┐
                  │      BATOU-CC       │
                  │    C++17 Compiler   │
                  └──────────┬──────────┘
                             │
                 ┌───────────┴───────────┐
                 │                       │
                 ▼                       ▼
          Graph Parsing            Optimization
                                   Passes
                                      │
                         ┌────────────┼────────────┐
                         │            │            │
                         ▼            ▼            ▼
                       Graph      Operator     Memory
                      Analysis      Fusion     Planning
                         │            │            │
                         └────────────┼────────────┘
                                      ▼
                           Static C Code Generation
                                      │
                                      ▼
                         generated/model_compiled.c
                                      │
                                      ▼
                     ┌─────────────────────────────┐
                     │        GCC / Clang          │
                     │                             │
                     │  model_compiled.c           │
                     │  runtime/kernels.c          │
                     │  runtime/runner.c           │
                     └──────────────┬──────────────┘
                                    │
                                    ▼
                             test_runner
```

---

## Key Features

### Zero-Heap Inference

BATOU-CC eliminates dynamic memory allocation from the generated inference path.

**`malloc()` / `free()` usage: `0 bytes`**

Inference operates using two statically allocated **ping-pong workspaces**:

```text
workspace_A  ◄──────────►  workspace_B
     │                         │
     ▼                         ▼
   Layer 1                   Layer 2
     │                         │
     └──────────► swap ◄───────┘
```

This provides:

* Deterministic memory usage
* Predictable execution behavior
* No heap fragmentation
* No runtime allocation failures
* Suitable for bare-metal environments

---

### Graph-Level Operator Fusion

BATOU-CC performs compiler-level optimization passes before emitting C code.

For example:

```text
CONV2D → RELU
```

can be transformed into:

```text
FUSED_CONV2D_RELU
```

This reduces unnecessary intermediate memory traffic and avoids redundant passes through workspace buffers.

---

### Static Memory Planning

The compiler analyzes tensor shapes and output sizes to determine the required workspace before execution.

Rather than discovering memory requirements at runtime:

```text
Neural Network
      │
      ▼
Tensor Shapes
      │
      ▼
Lifetime / Size Analysis
      │
      ▼
Static Workspace Bound
      │
      ▼
Generated C
```

The result is a bounded and deterministic memory footprint.

---

### Separation of Concerns

The project separates compilation, runtime execution, and generated artifacts:

| Component    | Responsibility                                      |
| ------------ | --------------------------------------------------- |
| `src/`       | Graph parsing, optimization passes, code generation |
| `include/`   | Compiler IR and interfaces                          |
| `runtime/`   | Target-agnostic inference kernels                   |
| `models/`    | Neural-network graph definitions                    |
| `generated/` | AOT-generated inference source                      |
| `bin/`       | Compiler and test-runner binaries                   |

This allows the compiler frontend to remain independent from the low-level runtime implementation.

---

## Project Structure

```text
BATOU-CC/
│
├── Makefile
├── README.md
│
├── include/
│   ├── compiler.hpp       # Compiler IR and class definitions
│   └── json.hpp           # Header-only JSON parser
│
├── models/
│   └── model.json         # Layer, tensor, and activation configuration
│
├── runtime/
│   ├── kernels.h          # Bare-metal kernel interface
│   ├── kernels.c          # Target-agnostic INT8 kernels
│   └── runner.c           # Standalone inference test runner
│
├── src/
│   └── compiler.cpp       # Compiler driver, optimization, C emitter
│
├── generated/
│   └── model_compiled.c   # Generated AOT inference implementation
│
└── bin/
    ├── batou-cc           # Compiler executable
    └── test_runner        # Generated model test binary
```

---

# Compilation Pipeline

BATOU-CC currently implements the following end-to-end pipeline:

```text
┌──────────────────────────┐
│     models/model.json    │
│                          │
│  Network Graph           │
│  Tensor Shapes           │
│  Activations             │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│      Graph Parsing       │
│                          │
│  • Read model structure  │
│  • Construct IR          │
│  • Validate graph        │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│   Optimization Passes    │
│                          │
│  • CONV2D + RELU Fusion  │
│  • Tensor size analysis  │
│  • Static memory plan    │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│      C Code Emitter      │
│                          │
│  Generates static        │
│  model_inference()       │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│ generated/               │
│ model_compiled.c         │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│       GCC / Clang        │
│                          │
│  + kernels.c             │
│  + runner.c              │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│    bin/test_runner       │
│                          │
│  Standalone inference    │
└──────────────────────────┘
```

---

#  Getting Started

## Prerequisites

* **GCC / G++** with C++17 support
* **C99-compatible GCC**
* **GNU Make**
* `mingw32-make` on Windows when using MinGW

---

## Build & Run

The entire pipeline can be executed with:

```bash
make run
```

This performs:

```text
Compile BATOU-CC
       ↓
Parse model.json
       ↓
Run optimization passes
       ↓
Generate model_compiled.c
       ↓
Compile runtime + generated model
       ↓
Execute inference
```

### Build Manually

```bash
# Build compiler and test runner
make

# Execute inference
./bin/test_runner
```

### Clean Build Artifacts

```bash
make clean
```

---

#  Verification

### v0.1.0-alpha

The initial release establishes and verifies the complete AOT compilation pipeline across **Windows (MinGW GCC/G++) and Unix environments**.

Currently functional:

* [x] JSON graph ingestion
* [x] Multi-layer model parsing
* [x] `CONV2D`
* [x] `RELU`
* [x] `MAXPOOL2D`
* [x] `DENSE`
* [x] Tensor shape analysis
* [x] Static workspace sizing
* [x] Ping-pong memory buffers
* [x] Operator fusion
* [x] C source generation
* [x] Zero-heap generated inference
* [x] Automated Makefile build pipeline
* [x] Standalone inference test runner

---

## Sample Execution

```text
====================================================
  BATOU-CC: Bare-Metal Model Inference Test Runner
====================================================

[Runner] Initializing input tensor (784 bytes)...
[Runner] Executing model_inference()...
[Runner] Inference execution complete!

--- Output Class Logits (INT8) ---
Class [0]: 127
Class [1]: 127
Class [2]: 127
Class [3]: 127
Class [4]: 127
Class [5]: 127
Class [6]: 127
Class [7]: 127
Class [8]: 127
Class [9]: 127
----------------------------------
```

The current release focuses on validating the **compiler → generated code → runtime → inference** path. Real quantized model weights are part of the next development phase.

---

# Roadmap

BATOU-CC is being developed toward a lightweight compiler/runtime stack for **hardware-aware neural-network deployment**.

## Phase 1 — Real INT8 Weight Integration

**Current:** Kernel execution uses placeholder accumulation loops to validate the generated execution path.

**Target:**

* Extend the `model.json` schema with quantized weights
* Emit `const int8_t` weight matrices
* Emit `int32_t` bias arrays
* Store model parameters directly in Flash / read-only memory
* Support real quantized classification inference
* Prevent weight storage from increasing runtime RAM usage

Target architecture:

```text
             Flash / ROM
                 │
       ┌─────────┴─────────┐
       │                   │
 INT8 Weights          INT32 Bias
       │                   │
       └─────────┬─────────┘
                 ▼
          Static Kernels
                 │
                 ▼
        SRAM Workspaces
```

---

## Phase 2 — Kernel Optimization & Hardware Acceleration

### Cache-Aware Optimization

Rework nested kernel loops using:

* Loop unrolling
* Cache-aware tiling
* Improved data locality
* Register utilization
* Reduced memory traffic

### SIMD / Vector Acceleration

Introduce architecture-specific kernel backends:

| Target       | Planned Backend |
| ------------ | --------------- |
| ARM Cortex-M | CMSIS-NN        |
| ARM Cortex-A | NEON            |
| RISC-V       | RVV             |
| x86-64       | AVX2 / AVX-512  |

For x86-64, INT8 dot-product acceleration can leverage instructions such as:

```text
VPDPBUSD
```

The compiler/runtime architecture is intended to keep these optimizations isolated from the high-level graph compiler.

---

## Phase 3 — Advanced Compiler Passes

### Graph Visualization

Add Graphviz DOT export:

```bash
./bin/batou-cc models/model.json --dot graph.dot
```

This will provide visualizations of:

* Graph topology
* Fused operators
* Tensor dependencies
* Workspace allocation
* Memory transitions

---

### Lifetime-Based Buffer Reuse

Expand the current memory planner into a full tensor lifetime analysis system.

Instead of treating buffers as strictly sequential:

```text
A → B → C → D
```

the compiler will identify non-overlapping tensor lifetimes and reuse memory:

```text
Tensor A ─────────┐
                  │
                  ▼
             ┌─────────┐
             │ Buffer  │
             │ Reuse   │
             └─────────┘
                  ▲
                  │
Tensor C ─────────┘
```

This will enable more efficient memory planning for multi-branch architectures such as:

* Residual networks
* Skip connections
* ResNet-style graphs
* Multi-path inference graphs

---

#  Design Goals

BATOU-CC is built around a few core principles:

```text
        ┌─────────────────────────┐
        │     HARDWARE AWARE      │
        └────────────┬────────────┘
                     │
       ┌─────────────┼─────────────┐
       ▼             ▼             ▼
   Deterministic   Static       Optimized
     Memory       Execution      Kernels
       │             │             │
       └─────────────┼─────────────┘
                     ▼
              Embedded Inference
```

### The long-term objective

Move neural-network deployment from:

```text
Model
  ↓
Runtime Interpreter
  ↓
Dynamic Graph
  ↓
Heap Allocation
  ↓
Inference
```

toward:

```text
Model
  ↓
AOT COMPILATION
  ↓
Optimized Static C
  ↓
Bare-Metal Runtime
  ↓
Deterministic Inference
```

---

#  Status

**Version:** `v0.1.0-alpha`

**Status:** 🚧 Active Development

BATOU-CC currently demonstrates the complete **graph ingestion → optimization → static memory planning → C code generation → compilation → inference** pipeline.

The next major milestone is integrating real **INT8 quantized weights and biases**, followed by architecture-specific kernel optimization and more advanced compiler memory passes.

---

## License

See the repository license for details.
