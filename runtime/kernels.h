#ifndef KERNELS_H
#define KERNELS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fused Conv2D (3x3, stride 1, padding 1) + ReLU in INT8
void conv2d_relu_int8(const int8_t* input, int8_t* output, 
                      int in_h, int in_w, int in_c, int out_c);

// Max Pooling (2x2, stride 2) in INT8
void maxpool2d_int8(const int8_t* input, int8_t* output, 
                    int in_h, int in_w, int in_c);

// Dense / Fully Connected Layer in INT8
void dense_int8(const int8_t* input, int8_t* output, 
                int in_features, int out_features);

#ifdef __cplusplus
}
#endif

#endif // KERNELS_H