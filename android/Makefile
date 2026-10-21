# Compile application C through the pinned ICK frontend. NDK owns its unmodified
# NativeActivity glue, assembly, compiler runtime and final Android link.
ABI ?= x86_64
PROFILE ?= Debug
OUT ?= build/android/$(ABI)/$(PROFILE)
ICK_ANDROID ?= ick
NDK ?=
NDK_BIN = $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT = $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/sysroot
HEADER_armeabi-v7a = arm-linux-androideabi
HEADER_arm64-v8a = aarch64-linux-android
HEADER_x86_64 = x86_64-linux-android
TARGET_armeabi-v7a = armv7a-linux-androideabi
TARGET_arm64-v8a = aarch64-linux-android
TARGET_x86_64 = x86_64-linux-android
FLAGS_armeabi-v7a = -marm -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp
FLAGS_arm64-v8a = -ffixed-x18
FLAGS_x86_64 = -march=x86-64-v2 -mno-avx -mno-movbe
NDK_CC = $(NDK_BIN)/$(TARGET_$(ABI))26-clang
ICK_PREFIX = -B$(dir $(ICK_ANDROID))
BUILTIN = $(shell "$(ICK_ANDROID)" $(ICK_PREFIX) -print-file-name=include)
OPT_Debug = -O0 -g -gdwarf-4 -gno-variable-location-views
OPT_Release = -O2 -DNDEBUG
APP_FLAGS = $(ICK_PREFIX) $(FLAGS_$(ABI)) --sysroot=$(SYSROOT) -nostdinc -isystem $(BUILTIN) -isystem $(SYSROOT)/usr/include -isystem $(SYSROOT)/usr/include/$(HEADER_$(ABI)) -I$(NDK)/sources/android/native_app_glue -D__ANDROID__ -D__ANDROID_API__=26 -D__ANDROID_MIN_SDK_VERSION__=26 -DBIONIC_IOCTL_NO_SIGNEDNESS_OVERLOAD -std=c11 -fPIC -Wall -Wextra -Werror $(OPT_$(PROFILE))
OBJECTS = $(OUT)/native/mathieu.o $(OUT)/native/session.o $(OUT)/native/checkpoint.o $(OUT)/native/render.o $(OUT)/android/activity.o $(OUT)/glue.o
.SECONDARY:
.PHONY: library FORCE
FORCE:
library: $(OUT)/libmathieu.so
$(OUT)/%.s: %.c native/mathieu.h native/session.h native/render.h FORCE
	mkdir -p $(dir $@)
	$(ICK_ANDROID) $(APP_FLAGS) -DMATHIEU_SOURCE_SHA='"$(SOURCE_SHA)"' -S $< -o $@
$(OUT)/%.o: $(OUT)/%.s
	$(NDK_CC) $(FLAGS_$(ABI)) -c $< -o $@
$(OUT)/glue.o: $(NDK)/sources/android/native_app_glue/android_native_app_glue.c
	mkdir -p $(dir $@)
	$(NDK_CC) -fPIC -O2 -c $< -o $@
$(OUT)/libmathieu.so: $(OBJECTS)
	$(NDK_CC) -shared -Wl,--no-undefined -Wl,-z,max-page-size=16384 $(OBJECTS) -landroid -llog -lm -o $@
