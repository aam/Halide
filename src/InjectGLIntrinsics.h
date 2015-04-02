#ifndef HALIDE_INJECT_GL_INTRINSICS_H
#define HALIDE_INJECT_GL_INTRINSICS_H

/** \file
 * Defines the lowering pass that injects texture loads and texture
 * stores for general texture-based target.
 */

#include "IR.h"
#include "Scope.h"

namespace Halide {
namespace Internal {

/** Take a statement with for kernel for loops and turn loads and
 * stores inside the loops into GL texture load and store
 * intrinsics. Should only be run when the GL target is active. */
Stmt inject_gl_intrinsics(Stmt s);
}
}

#endif
