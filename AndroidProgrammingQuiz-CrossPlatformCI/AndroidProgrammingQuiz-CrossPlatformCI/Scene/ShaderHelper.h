#pragma once

/**
 * ShaderHelper.h
 *
 * Shared OpenGL ES shader loading, compiling, and linking helpers for Models.
 * Supports inline source strings and runtime shader files/assets.
 */

#include "Platform.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper {
public:
    static GLuint compileShader(GLenum type, const char* src)
    {
        if (!src || !src[0]) {
            LOGE("Shader source is empty");
            return 0;
        }

        GLuint shader = glCreateShader(type);
        if (!shader) {
            LOGE("glCreateShader failed (type=0x%x)", type);
            return 0;
        }

        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len > 0 ? len : 1), '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            LOGE("Shader compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    static GLuint linkProgram(GLuint vert, GLuint frag)
    {
        GLuint prog = glCreateProgram();
        if (!prog) {
            LOGE("glCreateProgram failed");
            return 0;
        }

        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len > 0 ? len : 1), '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            LOGE("Program link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            return 0;
        }

        return prog;
    }

    static GLuint buildProgram(const char* vertSrc, const char* fragSrc)
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
        if (!vs) {
            return 0;
        }

        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!fs) {
            glDeleteShader(vs);
            return 0;
        }

        GLuint prog = linkProgram(vs, fs);
        if (prog) {
            glDetachShader(prog, vs);
            glDetachShader(prog, fs);
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
        return prog;
    }

#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* mgr, const char* path)
    {
        if (!mgr) {
            LOGE("Cannot load asset %s: asset manager is null", path);
            return "";
        }

        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) {
            LOGE("Shader asset not found: %s", path);
            return "";
        }

        const off_t length = AAsset_getLength(asset);
        std::string data(static_cast<size_t>(length), '\0');
        const int bytesRead = AAsset_read(asset, &data[0], static_cast<size_t>(length));
        AAsset_close(asset);

        if (bytesRead <= 0) {
            LOGE("Could not read shader asset: %s", path);
            return "";
        }

        data.resize(static_cast<size_t>(bytesRead));
        return data;
    }

    static GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath)
    {
        const std::string vertSrc = loadAsset(mgr, vertPath);
        const std::string fragSrc = loadAsset(mgr, fragPath);
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#else
    static std::string loadFile(const char* filename)
    {
        const std::vector<std::string> candidates = {
            std::string("assets/shader/") + filename,
            filename,
            std::string("shader/") + filename,
            std::string("assets/") + filename
        };

        for (const std::string& candidate : candidates) {
            std::ifstream file(candidate, std::ios::in | std::ios::binary);
            if (file) {
                std::ostringstream ss;
                ss << file.rdbuf();
                return ss.str();
            }
        }

        LOGE("Shader file not found: %s", filename);
        return "";
    }

    static GLuint buildProgramFromFile(const char* vertFile, const char* fragFile)
    {
        const std::string vertSrc = loadFile(vertFile);
        const std::string fragSrc = loadFile(fragFile);
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#endif
};
