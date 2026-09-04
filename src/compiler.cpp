#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "json.hpp" // Header-only JSON library
#include <filesystem>
#include "compiler.hpp"

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// 1. IR Data Structures
// -----------------------------------------------------------------------------

//commented because already declared in compiler.hpp included above
/*enum class OpType {
    CONV2D,
    RELU,
    MAXPOOL2D,
    DENSE,
    FUSED_CONV2D_RELU
};*/

OpType string_to_op(const std::string& str) {
    if (str == "CONV2D")    return OpType::CONV2D;
    if (str == "RELU")      return OpType::RELU;
    if (str == "MAXPOOL2D") return OpType::MAXPOOL2D;
    if (str == "DENSE")     return OpType::DENSE;
    throw std::invalid_argument("Unknown OpType string: " + str);
}

std::string op_to_string(OpType op) {
    switch (op) {
        case OpType::CONV2D:            return "CONV2D";
        case OpType::RELU:              return "RELU";
        case OpType::MAXPOOL2D:         return "MAXPOOL2D";
        case OpType::DENSE:             return "DENSE";
        case OpType::FUSED_CONV2D_RELU: return "FUSED_CONV2D_RELU";
        default:                        return "UNKNOWN";
    }
}

//commented because already declared in compiler.hpp included above
/*struct LayerNode {
    std::string name;
    OpType type;
    
    int in_h, in_w, in_c;
    int out_h, out_w, out_c;
    
    int input_buffer_id = -1;
    int output_buffer_id = -1;

    size_t get_output_size_bytes() const {
        return static_cast<size_t>(out_h * out_w * out_c);
    }
};*/

// -----------------------------------------------------------------------------
// 2. Neural Network Compiler
// -----------------------------------------------------------------------------
class TinyMLCompiler {
private:
    std::vector<LayerNode> nodes;
    size_t buffer_a_size = 0;
    size_t buffer_b_size = 0;

public:
    // --- Parse Model Architecture from JSON File ---
    void load_model_from_json(const std::string& json_path) {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open JSON file: " + json_path);
        }

        json j;
        file >> j;

        nodes.clear();
        for (const auto& item : j) {
            LayerNode node;
            node.name  = item.at("name").get<std::string>();
            node.type  = string_to_op(item.at("type").get<std::string>());
            node.in_h  = item.at("in_h").get<int>();
            node.in_w  = item.at("in_w").get<int>();
            node.in_c  = item.at("in_c").get<int>();
            node.out_h = item.at("out_h").get<int>();
            node.out_w = item.at("out_w").get<int>();
            node.out_c = item.at("out_c").get<int>();
            
            nodes.push_back(node);
        }
        std::cout << "Loaded " << nodes.size() << " layers from " << json_path << "\n";
    }

    // --- Optimization Pass 1: Operator Fusion ---
    void pass_fuse_conv_relu() {
        std::vector<LayerNode> optimized_nodes;
        
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (i + 1 < nodes.size() && 
                nodes[i].type == OpType::CONV2D && 
                nodes[i + 1].type == OpType::RELU) {
                
                LayerNode fused = nodes[i];
                fused.name = nodes[i].name + "_relu";
                fused.type = OpType::FUSED_CONV2D_RELU;
                
                optimized_nodes.push_back(fused);
                i++; // Skip original standalone RELU node
            } else {
                optimized_nodes.push_back(nodes[i]);
            }
        }
        nodes = optimized_nodes;
    }

    // --- Optimization Pass 2: Static Memory Allocator ---
    void pass_allocate_static_memory() {
        if (nodes.empty()) return;

        int current_buffer = 0; // 0 = Buffer A, 1 = Buffer B

        for (size_t i = 0; i < nodes.size(); ++i) {
            nodes[i].input_buffer_id = (i == 0) ? -1 : nodes[i - 1].output_buffer_id;

            if (i == nodes.size() - 1) {
                nodes[i].output_buffer_id = -1; // Output port
            } else {
                nodes[i].output_buffer_id = current_buffer;
                
                size_t req_bytes = nodes[i].get_output_size_bytes();
                if (current_buffer == 0) {
                    buffer_a_size = std::max(buffer_a_size, req_bytes);
                } else {
                    buffer_b_size = std::max(buffer_b_size, req_bytes);
                }

                current_buffer = 1 - current_buffer;
            }
        }
    }

    // --- Backend Emitter: Generate Target C Code ---
    void emit_c_code(const std::string& filename) const {
        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Failed to open output file: " << filename << "\n";
            return;
        }

        out << "// =============================================================================\n";
        out << "// Auto-generated by TinyML C++ Compiler\n";
        out << "// Target: Zero-heap static C execution graph\n";
        out << "// =============================================================================\n\n";
        out << "#include <stdint.h>\n";
        out << "#include \"kernels.h\"\n\n";

        out << "// Peak static memory workspace buffers\n";
        out << "static int8_t workspace_A[" << buffer_a_size << "];\n";
        out << "static int8_t workspace_B[" << buffer_b_size << "];\n\n";

        out << "void model_inference(const int8_t* input, int8_t* output) {\n";

        for (const auto& node : nodes) {
            std::string in_buf  = (node.input_buffer_id == -1)  ? "input" : 
                                  (node.input_buffer_id == 0)   ? "workspace_A" : "workspace_B";
            std::string out_buf = (node.output_buffer_id == -1) ? "output" : 
                                  (node.output_buffer_id == 0)  ? "workspace_A" : "workspace_B";

            out << "    // Node: " << node.name << " (" << op_to_string(node.type) << ")\n";

            switch (node.type) {
                case OpType::FUSED_CONV2D_RELU:
                    out << "    conv2d_relu_int8(" << in_buf << ", " << out_buf 
                        << ", " << node.in_h << ", " << node.in_w << ", " 
                        << node.in_c << ", " << node.out_c << ");\n\n";
                    break;

                case OpType::MAXPOOL2D:
                    out << "    maxpool2d_int8(" << in_buf << ", " << out_buf 
                        << ", " << node.in_h << ", " << node.in_w << ", " 
                        << node.in_c << ");\n\n";
                    break;

                case OpType::DENSE:
                    out << "    dense_int8(" << in_buf << ", " << out_buf 
                        << ", " << (node.in_h * node.in_w * node.in_c) 
                        << ", " << node.out_c << ");\n\n";
                    break;

                default:
                    out << "    // Unsupported operation\n\n";
                    break;
            }
        }

        out << "}\n";
        std::cout << "Successfully generated " << filename << "!\n";
    }
};

// -----------------------------------------------------------------------------
// 3. Driver Entry Point
// -----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // Default fallback values
    std::string json_file = "models/model.json";
    std::string output_file = "generated/model_compiled.c";

    // 1. Parse Command-Line Arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg[0] != '-') {
            json_file = arg;
        }
    }

    try {
        // 2. Ensure output directory exists (creates 'generated/' if missing)
        std::filesystem::path out_path(output_file);
        if (out_path.has_parent_path()) {
            std::filesystem::create_directories(out_path.parent_path());
        }

        // 3. Run Compiler Passes
        TinyMLCompiler compiler;

        std::cout << "=== Reading Model Graph ===\n";
        compiler.load_model_from_json(json_file);

        std::cout << "=== Running Compiler Passes ===\n";
        compiler.pass_fuse_conv_relu();
        compiler.pass_allocate_static_memory();

        // 4. Emit file to target path
        compiler.emit_c_code(output_file);
        std::cout << "Successfully generated " << output_file << "!\n";

    } catch (const std::exception& e) {
        std::cerr << "Compiler Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}