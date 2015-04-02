#ifndef HALIDE_CODEGEN_RS_DEV_H
#define HALIDE_CODEGEN_RS_DEV_H

/** \file
 * Defines the code-generator for producing RS host code
 */

#include "CodeGen_LLVM.h"
#include "CodeGen_GPU_Host.h"
#include "CodeGen_GPU_Dev.h"

namespace llvm {
class BasicBlock;
}

namespace Halide {
namespace Internal {

/** A code generator that emits RS code from a given Halide stmt. */
class CodeGen_RS_Dev : public CodeGen_LLVM, public CodeGen_GPU_Dev {
public:
    friend class CodeGen_GPU_Host<CodeGen_X86>;
    friend class CodeGen_GPU_Host<CodeGen_ARM>;

    /** Create a RS device code generator. */
    CodeGen_RS_Dev(Target host);
    ~CodeGen_RS_Dev();

    void add_kernel(Stmt stmt, const std::string &name,
//                    const std::string bounds_names[4],
                    const std::vector<GPU_Argument> &args);

    static void test();

    std::vector<char> compile_to_src();
    std::string get_current_kernel_name();

    void dump();

    virtual std::string print_gpu_name(const std::string &name);

    std::string api_unique_name() { return "rs"; }

protected:
    using CodeGen_LLVM::visit;

    /** (Re)initialize the RS module. This is separate from compile, since
     * a RS device module will often have many kernels compiled into it for
     * a single pipeline. */
    /* override */ virtual void init_module();

    /* override */ virtual llvm::Triple get_target_triple() const;
    /* override */ virtual llvm::DataLayout get_data_layout() const;

    /** We hold onto the basic block at the start of the device
     * function in order to inject allocas */
    llvm::BasicBlock *entry_block;

    /** Nodes for which we need to override default behavior for the RS runtime
     */
    // @{
    void visit(const For *);
    void visit(const Allocate *);
    void visit(const Free *);
    // @}

    void visit(const Call *op);

    std::string march() const;
    std::string mcpu() const;
    std::string mattrs() const;
    bool use_soft_float_abi() const;
    int native_vector_bits() const;

    /** Map from simt variable names (e.g. foo.__block_id_x) to the coresponding
     * parameter name. */
    std::string params_mapping(const std::string &name);
};
}
}

#endif
