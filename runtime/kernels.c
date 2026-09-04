#include "kernels.h"

// Basic Fused Conv2D + ReLU kernel implementation
void conv2d_relu_int8(const int8_t* input, int8_t* output, 
                      int in_h, int in_w, int in_c, int out_c) {
    for (int h = 0; h < in_h; ++h) {
        for (int w = 0; w < in_w; ++w) {
            for (int oc = 0; oc < out_c; ++oc) {
                int32_t acc = 0; // 32-bit accumulator to prevent overflow
                
                // Simplified dummy dot-product for runtime demonstration
                for (int ic = 0; ic < in_c; ++ic) {
                    int idx = (h * in_w * in_c) + (w * in_c) + ic;
                    acc += input[idx]; 
                }

                // ReLU activation: max(0, val)
                if (acc < 0) acc = 0;
                if (acc > 127) acc = 127; // INT8 clamping

                int out_idx = (h * in_w * out_c) + (w * out_c) + oc;
                output[out_idx] = (int8_t)acc;
            }
        }
    }
}

// Basic Max Pooling 2x2 implementation
void maxpool2d_int8(const int8_t* input, int8_t* output, 
                    int in_h, int in_w, int in_c) {
    int out_h = in_h / 2;
    int out_w = in_w / 2;

    for (int h = 0; h < out_h; ++h) {
        for (int w = 0; w < out_w; ++w) {
            for (int c = 0; c < in_c; ++c) {
                int8_t max_val = -128;
                
                for (int dh = 0; dh < 2; ++dh) {
                    for (int dw = 0; dw < 2; ++dw) {
                        int ih = h * 2 + dh;
                        int iw = w * 2 + dw;
                        int idx = (ih * in_w * in_c) + (iw * in_c) + c;
                        if (input[idx] > max_val) {
                            max_val = input[idx];
                        }
                    }
                }
                int out_idx = (h * out_w * in_c) + (w * in_c) + c;
                output[out_idx] = max_val;
            }
        }
    }
}

// Basic Dense / Linear Layer implementation
void dense_int8(const int8_t* input, int8_t* output, 
                int in_features, int out_features) {
    for (int oc = 0; oc < out_features; ++oc) {
        int32_t acc = 0;
        for (int ic = 0; ic < in_features; ++ic) {
            acc += input[ic]; // Accumulated sum demo
        }
        if (acc < -128) acc = -128;
        if (acc > 127) acc = 127;
        output[oc] = (int8_t)acc;
    }
}