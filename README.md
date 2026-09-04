================================================================================
BATOU-CC: Zero-Heap AOT C++ Compiler for Embedded Neural NetworksBATOU-CC (Bare-Metal Architecture Target Optimization Utility) is an Ahead-of-Time (AOT) C++17 neural network compiler designed for microcontrollers, bare-metal hardware, and resource-constrained real-time operating systems (RTOS).
================================================================================

By compiling trained network graphs directly into static, deterministic C code, BATOU-CC removes the overhead of runtime interpreters, dynamic graph parsers, and heap-based memory management (malloc/free).1. Key Architectural FeaturesZero-Heap Allocation ($0$ bytes malloc): Uses a static ping-pong buffer system (workspace_A and workspace_B) to run the complete inference graph in a bounded, deterministic memory footprint.Separation of Concerns:src/: High-level graph parsing, optimizations, and C code emission.include/: IR representations and interface declarations.runtime/: Target-agnostic, bare-metal math kernels (kernels.c, kernels.h).generated/: Clean build-artifact target location for emitted AOT static inference code (model_compiled.c).Operator Fusion Pass: Fuses sequential operations (such as CONV2D + RELU) at the graph level to eliminate redundant intermediate memory passes.Clean Build Pipeline: Isolated build outputs (bin/ and generated/) to guarantee deterministic compilation across platforms.2. Directory LayoutPlaintextBATOU-CC/
├── Makefile                # Automated build, code-gen, link, and test orchestration
├── readme.md               # Project documentation
├── include/
│   ├── compiler.hpp        # Compiler IR data structures and class definitions
│   └── json.hpp            # Header-only JSON model graph reader
├── models/
│   └── model.json          # Layer, shape, and activation configurations
├── runtime/
│   ├── kernels.h           # Low-level bare-metal math kernel interface
│   ├── kernels.c           # Target-agnostic INT8 kernel implementations
│   └── runner.c            # Standalone test runner and inference entry point
├── src/
│   └── compiler.cpp        # Compiler driver, optimization passes, and C emitter
├── bin/                    # Build output directory (Compiler & Test Runner binaries)
└── generated/              # Output target for emitted model_compiled.c source code
3. End-to-End Compilation Workflow                       [ models/model.json ]
                                 │
                                 ▼
                     ┌──────────────────────┐
                     │    bin/batou-cc      │  (C++17 Frontend Compiler)
                     └──────────────────────┘
                                 │
       ┌─────────────────────────┴─────────────────────────┐
       │ Passes:                                           │
       │ 1. Graph Parsing                                  │
       │ 2. CONV2D + RELU Operator Fusion                  │
       │ 3. Static Ping-Pong Buffer Memory Planning        │
       └─────────────────────────┬─────────────────────────┘
                                 │
                                 ▼
                   [ generated/model_compiled.c ]
                                 │
                                 ▼
┌───────────────────────────────────────────────────────────────────┐
│                             GCC / Clang                           │
│  Links: model_compiled.c + runtime/kernels.c + runtime/runner.c   │
└───────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
                       [ bin/test_runner ]
4. Current Release & Verification (v0.1.0-alpha)The initial release establishes the complete AOT toolchain, verified end-to-end on Windows (MinGW GCC/g++) and Unix environments.What Is Currently Functional:JSON Graph Ingestion: Parses multi-layer models (CONV2D, RELU, MAXPOOL2D, DENSE).Memory Buffer Optimization: Evaluates tensor output sizes per layer and computes exact static memory workspace bounds.C Source Emission: Generates clean, zero-heap model_inference() functions that manage workspace buffer swaps without dynamic allocation.Build Automation: Complete Makefile orchestration supporting make, make run, and make clean.Compilation & Execution GuidePrerequisitesg++ with C++17 supportgcc (C99 compliant)GNU make (or mingw32-make on Windows)CommandsBash# Compile compiler, generate code, link runner, and execute inference in one step:
make run

# Alternatively, run build steps individually:
make                  # Compiles batou-cc and builds bin/test_runner
./bin/test_runner     # Executes the bare-metal test binary

# Flush all generated binaries and build artifacts:
make clean
Sample Test OutputPlaintext====================================================
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
5. Future Scope & RoadmapNow that the graph-parsing and static code-generation pipeline is fully established, development focuses on kernel optimization, target platform porting, and quantized weight integration.Phase 1: Real INT8 Quantized Weight Matrix IntegrationCurrent State: Kernels utilize accumulation dummy loops to validate execution flow.Target: Update model.json schema and emission code to embed const int8_t weight matrices and int32_t bias arrays directly into Flash memory (PROGMEM/const segment), allowing real classification inference without RAM expansion.Phase 2: Kernel Optimization & Hardware AccelerationLoop Unrolling & Cache Tiling: Re-architect nested spatial loops inside kernels.c to maximize CPU L1 cache locality and minimize register spilling.SIMD & Vector Extensions: Implement target-specific kernels leveraging vector instructions:ARM Cortex-M / Cortex-A: ARM CMSIS-NN and NEON vector primitives.RISC-V: Vector Extension (RVV) assembly kernels.x86_64: AVX2/AVX-512 INT8 dot-product acceleration (VPDPBUSD).Phase 3: Advanced Compiler Passes & Graph VisualizationGraphviz DOT Exporter: Add a -dot CLI flag (./bin/batou-cc model.json --dot graph.dot) to generate graphical representations of fused memory nodes and ping-pong allocations.Dead Code & Buffer Reuse Elimination: Expand memory pass algorithms to perform lifetime analysis, enabling buffer reuse across non-adjacent nodes in multi-branch topologies (e.g., Residual Networks / ResNet skip connections).