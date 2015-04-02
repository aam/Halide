#include "InjectGLIntrinsics.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "CodeGen_GPU_Dev.h"
#include "Substitute.h"
#include "FuseGPUThreadLoops.h"

namespace Halide {
namespace Internal {

using std::string;
using std::vector;

class InjectGLIntrinsics : public IRMutator {
public:
    InjectGLIntrinsics() : inside_kernel_loop(false) {}
    Scope<int> scope;
    bool inside_kernel_loop;

private:
    using IRMutator::visit;

    void visit(const Provide *provide) {
        if (!inside_kernel_loop) {
            IRMutator::visit(provide);
            return;
        }

        internal_assert(provide->values.size() == 1)
            << "RS currently only supports single-valued stores.\n";
        user_assert(provide->args.size() == 3 || provide->args.size() == 2)
            << "RS stores requires either two or three coordinates.\n";

        // Create gl_texture_store(name.buffer, x, y, value)
        // intrinsic.
        vector<Expr> args(5);
        args[0] = provide->name;
        args[1] = Variable::make(Handle(), provide->name + ".buffer");
        args[2] = provide->args[0];  // x
        args[3] = provide->args[1];  // y
        debug(2) << "InjectGLIntrinsics: visit provide->values[0]="
                 << provide->values[0];
        args[4] = mutate(provide->values[0]);
        stmt = Evaluate::make(Call::make(args[4].type(), Call::gl_texture_store,
                                         args, Call::Intrinsic));
    }

    void visit(const Call *call) {
        if (!inside_kernel_loop || call->call_type == Call::Intrinsic ||
            call->call_type == Call::Extern) {
            IRMutator::visit(call);
            return;
        }

        string name = call->name;
        if (call->call_type == Call::Halide && call->func.outputs() > 1) {
            name = name + '.' + int_to_string(call->value_index);
        }

        // Check to see if we are reading from a one or two dimension function
        // and pad to three dimensions.
        vector<Expr> call_args = call->args;
        // RS: no padding to three as we support two as well.
        // while (call_args.size() < 3) {
        //     call_args.push_back(IntImm::make(0));
        // }

        // Create gl_texture_load(name, name.buffer, x, y, c) intrinsic.
        vector<Expr> args(5);
        args[0] = call->name;
        args[1] = Variable::make(Handle(), call->name + ".buffer");
        for (size_t i = 0; i < call_args.size(); i++) {
            string d = int_to_string(i);
            string min_name = name + ".min." + d;
            string min_name_constrained = min_name + ".constrained";
            if (scope.contains(min_name_constrained)) {
                min_name = min_name_constrained;
            }
            string extent_name = name + ".extent." + d;
            string extent_name_constrained = extent_name + ".constrained";
            if (scope.contains(extent_name_constrained)) {
                extent_name = extent_name_constrained;
            }

            Expr min = Variable::make(Int(32), min_name);
            Expr extent = Variable::make(Int(32), extent_name);

            // Remind users to explicitly specify the 'min' values of
            // ImageParams accessed by GLSL filters.
            if (i == 2 && call->param.defined()) {
                bool const_min_constraint =
                    call->param.min_constraint(i).defined() &&
                    is_const(call->param.min_constraint(i));
                if (!const_min_constraint) {
                    user_warning << "GLSL: Assuming min[2]==0 for ImageParam '"
                                 << name << "'. "
                                 << "Call set_min(2, min) or set_bounds(2, "
                                    "min, extent) to override.\n";
                    min = Expr(0);
                }
            }

            // Inject intrinsics into the call argument
            Expr arg = mutate(call_args[i]);
            debug(4) << "Subtracting min from arg. arg:" << arg
                     << " min:" << min << "\n";

            args[i + 2] = arg - min;
        }

        // This intrinsic represents the GLSL texture2D function, and that
        // function returns a vec4 result.
        Type load_type = call->type;
        load_type.width = 4;

        Expr load_call = Call::make(
            load_type, Call::gl_texture_load, vector<Expr>(&args[0], &args[4]),
            Call::Intrinsic, Function(), 0, call->image, call->param);

        // Add a shuffle_vector intrinsic to swizzle a single channel scalar out
        // of the vec4 loaded by glsl_texture_load. This may be widened to the
        // size of the Halide function color dimension during vectorization.
        expr = Call::make(call->type, Call::shuffle_vector,
                          vec(load_call, args[4]), Call::Intrinsic);
    }

    void visit(const LetStmt *let) {
        // Discover constrained versions of things.
        bool constrained_version_exists = ends_with(let->name, ".constrained");
        if (constrained_version_exists) {
            scope.push(let->name, 0);
        }

        IRMutator::visit(let);

        if (constrained_version_exists) {
            scope.pop(let->name);
        }
    }

    void visit(const For *loop) {
        bool old_kernel_loop = inside_kernel_loop;
        if (loop->for_type == ForType::Parallel &&
            (  // loop->device_api == DeviceAPI::GLSL ||
                loop->device_api == DeviceAPI::RS)) {
            inside_kernel_loop = true;
        }
        IRMutator::visit(loop);
        inside_kernel_loop = old_kernel_loop;
    }
};

Stmt inject_gl_intrinsics(Stmt s) {
    debug(4) << "InjectGLIntrinsics: inject_gl_intrinsics stmt: " << s << "\n";
    s = zero_gpu_loop_mins(s);
    InjectGLIntrinsics gl;
    return gl.mutate(s);
}
}
}
