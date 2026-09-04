#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
// Declarations from Auto-Generated C File (model_compiled.c)
// -----------------------------------------------------------------------------
// This function signature is emitted by BATOU-CC
extern void model_inference(const int8_t* input, int8_t* output);

// Example shape dimensions for a 28x28x1 image -> 10 output classes (e.g., MNIST)
#define INPUT_SIZE  (28 * 28 * 1)
#define OUTPUT_SIZE 10

int main(void) {
    printf("====================================================\n");
    printf("  BATOU-CC: Bare-Metal Model Inference Test Runner  \n");
    printf("====================================================\n\n");

    // 1. Allocate input/output buffers on stack/static memory
    int8_t input_tensor[INPUT_SIZE];
    int8_t output_tensor[OUTPUT_SIZE];

    // 2. Populate input tensor with synthetic test data (e.g., gradient pattern)
    printf("[Runner] Initializing input tensor (%d bytes)...\n", INPUT_SIZE);
    for (int i = 0; i < INPUT_SIZE; ++i) {
        input_tensor[i] = (int8_t)((i % 32) - 16); // Values between -16 and 15
    }

    // 3. Clear output buffer
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        output_tensor[i] = 0;
    }

    // 4. Execute the auto-generated static C pipeline
    printf("[Runner] Executing model_inference()...\n");
    model_inference(input_tensor, output_tensor);
    printf("[Runner] Inference execution complete!\n\n");

    // 5. Output classification results
    printf("--- Output Class Logits (INT8) ---\n");
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        printf("Class [%d]: %d\n", i, output_tensor[i]);
    }
    printf("----------------------------------\n");

    return 0;
}