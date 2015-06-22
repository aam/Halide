#include <iostream>
#include <iomanip>
#include "test_arm.h"

template<typename T> buffer_t make_1d_image(int width, T host[]) {
    buffer_t bt = { 0 };
    bt.host = (uint8_t*)&host[0];
    bt.stride[0] = 1;
    bt.extent[0] = width;
    bt.elem_size = sizeof(T);
    return bt;
}

template<typename T> buffer_t make_2d_image(int width, int height, T host[]) {
    buffer_t bt = { 0 };
    bt.host = (uint8_t*)&host[0];
    bt.stride[0] = 1;
    bt.extent[0] = width;
    bt.stride[1] = width;
    bt.extent[1] = height;
    bt.elem_size = sizeof(T);
    return bt;
}

extern "C" void halide_set_renderscript_cache_dir(const char *c);
extern "C" int halide_copy_to_host(void *, buffer_t *);

int main(int argc, char** argv) {
    const char *cacheDir = "/data/tmp";
    halide_set_renderscript_cache_dir(cacheDir);

	int size = 10;
	float *input = (float*)malloc(sizeof(float) * size * size);
	float *output = (float*)malloc(sizeof(float) * size * size);

	buffer_t input_buft = make_2d_image(size, size, input);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			((float*)(input_buft.host))[x + size * y] = y + x;
		}
	}

	std::cout << "Input :\n";
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			std::cout << std::setfill('0') << std::setw(2) <<
				((float*)(input_buft.host))[x + size * y] << " ";
		}
		std::cout << "\n";
	}

	buffer_t output_buft = make_2d_image(size, size, output);

	input_buft.host_dirty = true;
	test_arm(&input_buft, &output_buft);
	halide_copy_to_host(NULL, &output_buft);

	std::cout << "Output :\n";
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			std::cout << std::setfill('0') << std::setw(2) <<
				((float*)(output_buft.host))[x + size * y] << " ";
		}
		std::cout << "\n";
	}
}
