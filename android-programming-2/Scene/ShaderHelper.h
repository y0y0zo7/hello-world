#pragma once

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
#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* manager, const char* path)
    {
        if (!manager || !path) return {};
        AAsset* asset = AAssetManager_open(manager, path, AASSET_MODE_BUFFER);
        if (!asset) {
            LOGE("Cannot open shader asset: %s", path);
            return {};
        }
        const auto size = static_cast<std::size_t>(AAsset_getLength(asset));
        std::string source(size, '\0');
        const int bytesRead = AAsset_read(asset, source.data(), size);
        AAsset_close(asset);
        if (bytesRead < 0 || static_cast<std::size_t>(bytesRead) != size) {
            LOGE("Cannot read shader asset: %s", path);
            return {};
        }
        return source;
    }

    static GLuint buildProgramFromAssets(AAssetManager* manager,
                                         const char* vertexPath,
                                         const char* fragmentPath)
    {
        const std::string vertexSource = loadAsset(manager, vertexPath);
        const std::string fragmentSource = loadAsset(manager, fragmentPath);
        if (vertexSource.empty() || fragmentSource.empty()) return 0;
        return buildProgram(vertexSource.c_str(), fragmentSource.c_str());
    }
#else
    static std::string loadFile(const char* filename)
    {
        const std::vector<std::string> searchPaths = {
            std::string("assets/shader/") + filename,
            std::string(filename),
            std::string("shader/") + filename,
        };
        for (const std::string& path : searchPaths) {
            std::ifstream stream(path);
            if (stream) {
                std::ostringstream buffer;
                buffer << stream.rdbuf();
                return buffer.str();
            }
        }
        LOGE("Cannot find shader: %s", filename);
        return {};
    }

    static GLuint buildProgramFromFile(const char* vertexFile, const char* fragmentFile)
    {
        const std::string vertexSource = loadFile(vertexFile);
        const std::string fragmentSource = loadFile(fragmentFile);
        if (vertexSource.empty() || fragmentSource.empty()) return 0;
        return buildProgram(vertexSource.c_str(), fragmentSource.c_str());
    }
#endif

private:
    static GLuint compileShader(GLenum type, const char* source)
    {
        const GLuint shader = glCreateShader(type);
        if (!shader) return 0;

        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (compiled == GL_TRUE) return shader;

        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        LOGE("Shader compilation failed: %s", log.c_str());
        glDeleteShader(shader);
        return 0;
    }

    static GLuint buildProgram(const char* vertexSource, const char* fragmentSource)
    {
        const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
        if (!vertex) return 0;
        const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (!fragment) {
            glDeleteShader(vertex);
            return 0;
        }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
            glGetProgramInfoLog(program, length, nullptr, log.data());
            LOGE("Program link failed: %s", log.c_str());
            glDeleteProgram(program);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return linked == GL_TRUE ? program : 0;
    }
};
