#include "Halide.h"

using namespace Halide;

int main(int argc, char **argv) {
    ImageParam input(UInt(32), 2, "input");
    Func clamped("clamped");
    Var x, y;
    clamped(x, y) = input(
    clamp(x, 0, input.width() - 1),
    clamp(y, 0, input.height() - 1));

    Func f("f");
    f(x, y) = clamped(x, y);

    Func g("g");
    g(x, y) = f(x, y);

    f.compute_root().shader(x, y, DeviceAPI::Renderscript);
    g.compute_root().shader(x, y, DeviceAPI::Renderscript);
    Target target = get_target_from_environment();
    g.compile_to_file(target.arch == Target::Arch::ARM? "test_arm": "test", {input});
}
