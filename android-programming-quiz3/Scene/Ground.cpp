/**
 * @file Ground.cpp
 * @brief Implements the world-space ground quad and its lighting state.
 * @author Zoey <t.zoey@digipen.edu>
 */

#define LOG_TAG "Ground"
#include "Ground.h"

#include "ShaderHelper.h"

#include <cstddef>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
struct GroundVertex {
    GLfloat position[3];
    GLfloat normal[3];
};

constexpr float kGroundY = -1.925f;
constexpr float kGroundHalfExtent = 24.0f;
constexpr GroundVertex kVertices[] = {
    {{-kGroundHalfExtent, kGroundY, -kGroundHalfExtent}, {0.0f, 1.0f, 0.0f}},
    {{-kGroundHalfExtent, kGroundY,  kGroundHalfExtent}, {0.0f, 1.0f, 0.0f}},
    {{ kGroundHalfExtent, kGroundY,  kGroundHalfExtent}, {0.0f, 1.0f, 0.0f}},
    {{-kGroundHalfExtent, kGroundY, -kGroundHalfExtent}, {0.0f, 1.0f, 0.0f}},
    {{ kGroundHalfExtent, kGroundY,  kGroundHalfExtent}, {0.0f, 1.0f, 0.0f}},
    {{ kGroundHalfExtent, kGroundY, -kGroundHalfExtent}, {0.0f, 1.0f, 0.0f}},
};
} // namespace

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager* manager) : assetManager(manager)
{
#else
Ground::Ground()
{
#endif
    modelType = GroundType;
}

Ground::~Ground()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
}

void Ground::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("GroundVertex.glsl", "GroundFragment.glsl");
#endif
    if (!program) {
        LOGE("Ground shader program could not be created");
        return;
    }

    mvpUniform = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    modelUniform = glGetUniformLocation(program, "MODELMATRIX");
    normalUniform = glGetUniformLocation(program, "NORMALMATRIX");
    lightPositionUniform = glGetUniformLocation(program, "LIGHTPOSITION");
    cameraPositionUniform = glGetUniformLocation(program, "CAMERAPOSITION");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(GroundVertex),
        reinterpret_cast<const void*>(offsetof(GroundVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(GroundVertex),
        reinterpret_cast<const void*>(offsetof(GroundVertex, normal)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    LOGI("Procedural Phong checkerboard initialized");
}

void Ground::SetViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    viewMatrix = view;
    projectionMatrix = projection;
}

void Ground::SetLighting(const glm::vec3& light, const glm::vec3& camera)
{
    lightPosition = light;
    cameraPosition = camera;
}

void Ground::Render()
{
    if (!program || !vao) return;

    const glm::mat4 model(1.0f);
    const glm::mat4 mvp = projectionMatrix * viewMatrix * model;
    const glm::mat3 normal = glm::inverseTranspose(glm::mat3(model));

    glUseProgram(program);
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalUniform, 1, GL_FALSE, glm::value_ptr(normal));
    glUniform3fv(lightPositionUniform, 1, glm::value_ptr(lightPosition));
    glUniform3fv(cameraPositionUniform, 1, glm::value_ptr(cameraPosition));
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}
