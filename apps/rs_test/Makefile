include ../support/Makefile.inc

TOP := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
.PHONY: all $(TOP)
all: test-arm
HALIDE_LIB := $(TOP)/$(LIB_HALIDE)
$(HALIDE_LIB): $(TOP)
	$(MAKE) -C $(TOP)

test_gen: ../../ test_gen.cpp $(HALIDE_LIB)
	$(CXX) $(CXXFLAGS) test_gen.cpp ../../$(LIB_HALIDE) -o test_gen -lz $(LDFLAGS) $(LLVM_SHARED_LIBS)

test_arm.o test_arm.h: test_gen
	 HL_TARGET=arm-32-android-armv7s-renderscript-debug ./test_gen

libs/armeabi-v7a/test-rs: $(HALIDE_LIB) \
                         jni/test-rs.cpp \
                         test_arm.o test_arm.h
	NDK_MODULE_PATH=../support/ndk_modules ndk-build libs/armeabi-v7a/test-rs libs/armeabi-v7a/libstlport_shared.so

deploy-test-arm: libs/armeabi-v7a/test-rs
	adb push libs/armeabi-v7a/test-rs /mnt/sdcard/
	adb push libs/armeabi-v7a/libstlport_shared.so /mnt/sdcard/

define DEPLOY_TEST_STEPS
su
mkdir /data/tmp
cd /data/tmp
pwd
cp /mnt/sdcard/test-rs .
chmod 777 /data/tmp/test-rs
cp /mnt/sdcard/libstlport_shared.so .
rm /mnt/sdcard/out-arm.png
LD_LIBRARY_PATH=. ./test-rs
exit
exit
endef
export DEPLOY_TEST_STEPS

test-arm: deploy-test-arm
	sh -c 'echo "$$DEPLOY_TEST_STEPS" | adb shell'
	echo "Done"

clean:
	rm -rf test_arm.* test_gen test_run test.* obj libs
