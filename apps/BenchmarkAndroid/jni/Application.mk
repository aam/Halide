# Can't use "APP_ABI = all" as 64-bit MIPS currently does not build since
# llvm will not compile for the R6 version of the ISA without Nan2008
# and the gcc toolchain used by the Android build setup requires those
# two options together.
APP_ABI := armeabi-v7a # armeabi arm64-v8a mips x86_64 x86
APP_PLATFORM := android-21
APP_CFLAGS += -std=c++11
APP_STL := gnustl_static
LOCAL_C_INCLUDES += ${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/include