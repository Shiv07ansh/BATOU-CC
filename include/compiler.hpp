#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>
#include <vector>
#include <cstddef>

// -----------------------------------------------------------------------------
// IR (Intermediate Representation) Definitions
// -----------------------------------------------------------------------------
enum class OpType {
    CONV2D,
    RELU,
    MAXPOOL2D,
    DENSE,
    FUSED_CONV2D_RELU
};

OpType string_to_op(const std::string& str);
std::string op_to_string(OpType op);

struct LayerNode {
    std::string name;
    OpType type;
    
    int in_h, in_w, in_c;
    int out_h, out_w, out_c;
    
    int input_buffer_id = -1;  // -1 = Network Input Port, 0 = Workspace A, 1 = Workspace B
    int output_buffer_id = -1; // -1 = Network Output Port, 0 = Workspace A, 1 = Workspace B

    size_t get_output_size_bytes() const {
        return static_cast<size_t>(out_h * out_w * out_c);
    }
};

// -----------------------------------------------------------------------------
// BATOU-CC Compiler Core
// -----------------------------------------------------------------------------
class BatouCompiler {
private:
    std::vector<LayerNode> nodes;
    size_t buffer_a_size = 0;
    size_t buffer_b_size = 0;

public:
    BatouCompiler() = default;

    void load_model_from_json(const std::string& json_path);
    void pass_fuse_conv_relu();
    void pass_allocate_static_memory();
    void emit_c_code(const std::string& output_path) const;
};

#endif // COMPILER_HPP