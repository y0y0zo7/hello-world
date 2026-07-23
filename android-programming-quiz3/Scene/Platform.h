/**
 * @file Platform.h
 * @brief Centralizes platform detection, graphics headers, and logging macros.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

// Emscripten may also define _WIN32, so detect it first.
#ifdef __EMSCRIPTEN__
    #define PLATFORM_EMSCRIPTEN
#elif defined(__ANDROID__)
    #ifndef PLATFORM_ANDROID
        #define PLATFORM_ANDROID
    #endif
#elif defined(_WIN32) || defined(PLATFORM_WINDOWS)
    #ifndef PLATFORM_WINDOWS
        #define PLATFORM_WINDOWS
    #endif
#endif

#ifdef PLATFORM_EMSCRIPTEN
    #define GLFW_INCLUDE_ES3
    #include <GLFW/glfw3.h>
    #include <emscripten.h>
    #include <cstdio>
    #include <cstring>
    #define LOGI(...) do { std::printf("[Fan3D] " __VA_ARGS__); std::printf("\n"); } while (0)
    #define LOGE(...) do { std::fprintf(stderr, "[Fan3D] " __VA_ARGS__); std::fprintf(stderr, "\n"); } while (0)
#elif defined(PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
    #include <android/log.h>
    #include <cstring>
    #ifndef LOG_TAG
        #define LOG_TAG "Fan3D"
    #endif
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#elif defined(PLATFORM_WINDOWS)
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <cstdio>
    #include <cstring>
    #define LOGI(...) do { std::printf("[Fan3D] " __VA_ARGS__); std::printf("\n"); } while (0)
    #define LOGE(...) do { std::fprintf(stderr, "[Fan3D] " __VA_ARGS__); std::fprintf(stderr, "\n"); } while (0)
#else
    #error "Fan3D: unsupported platform"
#endif
