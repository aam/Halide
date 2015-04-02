#include "Halide.h"

#include <iostream>

using namespace Halide;

using Halide::Image;
#include "../../tutorial/image_io.h"

Expr u8(Expr x) { return cast(UInt(8), x); }

Expr i16(Expr x) { return cast(Int(16), x); }

Expr f32(Expr x) { return cast(Float(32), x); }

void SharperAndToneCurve() {
    const int nChannels = 4;

    ImageParam input8(UInt(8), 3, "input");
    input8.set_stride(0, nChannels)
        .set_stride(1, Halide::Expr())
        .set_stride(2, 1)
        .set_bounds(2, 0, nChannels);  // expecting
    // chunky image

    // ImageParam input(UInt(8), 2, "input");
    //    Image<uint8_t> input = load<uint8_t>("../../tutorial/images/rgb.png");

    Var x, y, c;
    Func input;
    input(x, y, c) = input8(clamp(x, input8.left(), input8.right()),
                            clamp(y, input8.top(), input8.bottom()), c);

    // Func tone_curve;
    // tone_curve(x) = i16(pow(f32(x)/256.0f, 1.8f) * 256.0f);

    // Func clamped;
    // clamped(x, y, c) = input(clamp(x, 0, input.width()-1),
    //                          clamp(y, 0, input.height()-1),
    //                          clamp(c, 0, 3));

    // Func curved;
    // curved(x, y, c) = tone_curve(clamped(x, y, c));

    // Func sharper;
    // sharper(x, y, c) = 9*curved(x, y, c) - 2*(curved(x-1, y, c) + curved(x+1,
    // y, c) + curved(x, y-1, c) + curved(x, y+1, c));

    // result(x, y, c) = u8(clamp(sharper(x, y, c), 0, 255));
    // result(x, y, c) = cast<uint8_t>(
    //     (input(x + 1, y, c) + input(x + 2, y, c) + input(x + 3, y, c)) / 3);

    Func blur_x("blur_x");
    blur_x(x, y, c) = cast<uint8_t>(
        (input(x, y, c) + input(x + 1, y, c) + input(x + 2, y, c)) / 3);
    blur_x.output_buffer()
        .set_stride(0, nChannels)
        .set_stride(1, Halide::Expr())
        .set_stride(2, 1)
        .set_bounds(2, 0, nChannels);  // expecting chunky image
    blur_x.bound(c, 0, 4);

    Func result("result");
    result(x, y, c) = cast<uint8_t>(
        (blur_x(x, y, c) + blur_x(x, y + 1, c) + blur_x(x, y + 2, c)) / 3);
    result.output_buffer()
        .set_stride(0, nChannels)
        .set_stride(1, Halide::Expr())
        .set_stride(2, 1)
        .set_bounds(2, 0, nChannels);  // expecting chunky image
    // result(x, y) = input(x, y) + 10;

    // tone_curve.compute_root();
    // Var yi;

    // result.split(y, y, yi, 60).vectorize(x, 8).parallel(y);
    // curved.store_at(result, y).compute_at(result, yi);

    // result.compute_root().vectorize(x, 8).gpu_tile(x, y, 2, 16,
    // DeviceAPI::OpenCL);
    // curved.compute_root().vectorize(x, 8).gpu_tile(x, y, 2, 16,
    // DeviceAPI::OpenCL);

    // result.gpu_blocks(DeviceAPI::RS);

    // result.reorder(c, x, y)
    //     .bound(c, 0, 4)
    //     .reorder_storage(c, x, y)
    //     .gpu_blocks(x, y, DeviceAPI::RS)
    //     .vectorize(c);

    result.bound(c, 0, 4);
    // result.glsl(x, y, c);
    result.rs(x, y, c).vectorize(c);

    // Target target = get_host_target();
    // target.set_feature(Target::Debug);
    // target.set_feature(Target::RS);

    std::vector<Argument> args;
    args.push_back(input8);
    result.compile_to_file("rs_halide_generated", args);
    // result.compile_to_assembly("rs_halide_generated.asm", args);
    // result.compile_to_lowered_stmt("rs_halide_generated.stmt.html", HTML);
    // result.compile_to_bitcode("rs_halide_generated", args);
    // result.compile_jit(target);

    // uint8_t input_image[10 * 10];
    // for (int i = 0; i < 100; i++) { input_image[i] = i; }

    // uint8_t output_image[10 * 10];

    // buffer_t bt_input = {0};
    // bt_input.host = &input_image[0];
    // bt_input.stride[0] = 1;
    // bt_input.extent[0] = 10;
    // bt_input.stride[1] = 10;
    // bt_input.extent[1] = 10;
    // bt_input.elem_size = 1;

    // buffer_t bt_output = {0};
    // bt_output.host = &output_image[0];
    // bt_output.stride[0] = 1;
    // bt_output.extent[0] = 10;
    // bt_output.stride[1] = 10;
    // bt_output.extent[1] = 10;
    // bt_output.elem_size = 1;

    // int error = result.realize(&bt_input, &bt_output);
    // if (error) {
    //     std::cout << "Halide returned error: " << error << std::endl;
    // }

    // for (int i = 0; i < bt_output.extent[0]; i++) {
    //     for (int j = 0; j < bt_output.extent[1]; j++) {
    //         std::cout.width(8);

    //         char buffer[33];
    //         sprintf(buffer, "%d", bt_output.host[i * bt_output.stride[0]
    //                                              + j * bt_output.stride[1]]);
    //         std::cout << buffer;
    //     }
    //     std::cout << std::endl;
    // }

    // Image<uint8_t> output_image = result.realize(input.width(),
    // input.height(), input.channels());

    // save(output_image, "brighter.png");
}

int main(int argc, char **argv) {
    SharperAndToneCurve();

    std::cout << "Done!" << std::endl;
}