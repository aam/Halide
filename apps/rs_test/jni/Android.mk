LOCAL_PATH:= $(call my-dir)

# === test-rs ===

include $(CLEAR_VARS)

LOCAL_MODULE           := test-rs
LOCAL_SRC_FILES        := test-rs.cpp
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_STATIC_LIBRARIES += libpng
LOCAL_STATIC_LIBRARIES += libRScpp_static
LOCAL_LDLIBS           := -lm -llog -landroid -lz test_arm.o
LOCAL_ARM_MODE         := arm

LOCAL_NDK_STL_VARIANT := stlport_static
intermediates := $(call intermediates-dir-for,STATIC_LIBRARIES,libRS,TARGET,)
LOCAL_C_INCLUDES += $(intermediates)
LOCAL_C_INCLUDES +=  /Volumes/Android/lmp-mr1/frameworks/rs/
LOCAL_C_INCLUDES +=  /Volumes/Android/lmp-mr1/frameworks/rs/cpp

LOCAL_CPPFLAGS += -g -std=c++11 -I../support -I../../include

LOCAL_C_INCLUDES += ./

include $(BUILD_EXECUTABLE)

$(call import-module,android/native_app_glue)
