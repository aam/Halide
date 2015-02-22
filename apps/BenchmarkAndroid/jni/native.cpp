#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <random>

#include "halide_sgemm_notrans.h"
#include <HalideRuntime.h>

#include <sstream>

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"halide_native",__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,"halide_native",__VA_ARGS__)

#define DEBUG 1

extern "C" int64_t halide_current_time_ns();

int handler(void */* user_context */, const char *msg) {
    LOGE("%s", msg);
}

template<class T>
struct BenchmarksBase {
    typedef T Scalar;

    std::random_device rand_dev;
    std::default_random_engine rand_eng{rand_dev()};

    Scalar random_scalar() {
        std::uniform_real_distribution<T> uniform_dist(0.0, 1.0);
        return uniform_dist(rand_eng);
    }

    buffer_t random_matrix(int N) {
        Scalar* A = (Scalar*)malloc(N * N * sizeof(Scalar));
        for (int i=0; i<N*N; ++i) {
            A[i] = random_scalar();
        }
        buffer_t buf_t = {0};
        buf_t.host = (uint8_t *)A;
        buf_t.extent[0] = N;
        buf_t.extent[1] = N;
        buf_t.extent[2] = 0;
        buf_t.extent[3] = 0;
        buf_t.stride[0] = 1;
        buf_t.stride[1] = N;
        buf_t.elem_size = sizeof(Scalar);
        return buf_t;
    }

    BenchmarksBase() {}
};

struct BenchmarksFloat : public BenchmarksBase<float> {
    Scalar a;
    buffer_t A;
    buffer_t B;
    Scalar b;
    buffer_t C;

    BenchmarksFloat(unsigned N):
        BenchmarksBase(),
        a(random_scalar()),
        A(random_matrix(N)),
        B(random_matrix(N)),
        b(random_scalar()),
        C(random_matrix(N)) {}

    void run() {
        halide_sgemm_notrans(a, &A, &B, b, &C, &C);
    }
};

extern "C" {
JNIEXPORT jstring JNICALL Java_com_example_hellohalide_BenchmarkActivity_runTest(
    JNIEnv *env, jobject callingObject, jint size) {
    LOGD("Ready to run with size: %d", size);
    BenchmarksFloat bf(size);
    int64_t t1 = halide_current_time_ns();
    bf.run();
    int64_t t2 = halide_current_time_ns();
    unsigned elapsed_us = (t2 - t1)/1000;
    LOGD("For %d size time taken: %d", size, elapsed_us);
    std::ostringstream ss;
    ss << "For " << size << " time taken is " << elapsed_us << "ns";
    return env->NewStringUTF(ss.str().c_str());
}

}
