// include/maestro/core/config.hpp
#pragma once

#define MAESTRO_VERSION_MAJOR 1
#define MAESTRO_VERSION_MINOR 0
#define MAESTRO_VERSION_PATCH 0
#define MAESTRO_VERSION_STRING "1.0.0"

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define MAESTRO_PLATFORM_WINDOWS
    #define MAESTRO_EXPORT __declspec(dllexport)
    #define MAESTRO_IMPORT __declspec(dllimport)
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define MAESTRO_PLATFORM_IOS
    #else
        #define MAESTRO_PLATFORM_MACOS
    #endif
    #define MAESTRO_EXPORT __attribute__((visibility("default")))
    #define MAESTRO_IMPORT
#elif defined(__ANDROID__)
    #define MAESTRO_PLATFORM_ANDROID
    #define MAESTRO_EXPORT __attribute__((visibility("default")))
    #define MAESTRO_IMPORT
#elif defined(__linux__)
    #define MAESTRO_PLATFORM_LINUX
    #define MAESTRO_EXPORT __attribute__((visibility("default")))
    #define MAESTRO_IMPORT
#else
    #define MAESTRO_EXPORT
    #define MAESTRO_IMPORT
#endif

// MAESTRO_API definition
#if defined(MAESTRO_BUILD_SHARED) || defined(MAESTRO_EXPORT_API)
    #define MAESTRO_API MAESTRO_EXPORT
#else
    #define MAESTRO_API
#endif

// Audio configuration
namespace maestro::config {
    constexpr int DEFAULT_SAMPLE_RATE = 44100;
    constexpr int DEFAULT_BUFFER_SIZE = 256;
    constexpr int DEFAULT_CHANNELS = 2;
    constexpr int MAX_MIDI_CHANNELS = 16;
    constexpr int MAX_AUDIO_TRACKS = 256;
    constexpr int MAX_VOICES = 256;
    constexpr int MAX_PADS = 64;
    constexpr int MAX_STYLES_TRACKS = 16;
}
