# Android Platform Configuration
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_HOME})
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_STL_TYPE c++_shared)
set(CMAKE_ANDROID_API 29)

# Android-specific sources
set(ANDROID_SOURCES
    src/platforms/android/audio_oboe.cpp
    src/platforms/android/midi_android.cpp
    src/platforms/android/jni_bridge.cpp
)
