/**
 * @file Fan.cpp
 * @brief Implements fan geometry, animation, lighting, and per-part ray tests.
 * @author Zoey <t.zoey@digipen.edu>
 */

#define LOG_TAG "Fan"
#include "Fan.h"

#include "ShaderHelper.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
constexpr GLuint kPositionAttribute = 0;
constexpr GLuint kNormalAttribute = 1;

// Twenty-four vertices allow each cube face to keep a flat face normal.
constexpr GLfloat kCubePositions[] = {
    // Front
    -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
    // Right
     0.5f, -0.5f,  0.5f,   0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
    // Back
     0.5f, -0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
    // Left
    -0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,
    // Top
    -0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
    // Bottom
    -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
};

constexpr GLfloat kCubeNormals[] = {
     0,  0,  1,   0,  0,  1,   0,  0,  1,   0,  0,  1,
     1,  0,  0,   1,  0,  0,   1,  0,  0,   1,  0,  0,
     0,  0, -1,   0,  0, -1,   0,  0, -1,   0,  0, -1,
    -1,  0,  0,  -1,  0,  0,  -1,  0,  0,  -1,  0,  0,
     0,  1,  0,   0,  1,  0,   0,  1,  0,   0,  1,  0,
     0, -1,  0,   0, -1,  0,   0, -1,  0,   0, -1,  0,
};

constexpr GLushort kCubeIndices[] = {
     0,  1,  2,   0,  2,  3,
     4,  5,  6,   4,  6,  7,
     8,  9, 10,   8, 10, 11,
    12, 13, 14,  12, 14, 15,
    16, 17, 18,  16, 18, 19,
    20, 21, 22,  20, 22, 23,
};

constexpr GLfloat kBaseColor[] = {0.47f, 0.27f, 0.10f};
constexpr GLfloat kPoleColor[] = {0.56f, 0.59f, 0.64f};
constexpr GLfloat kHubColor[] = {0.10f, 0.13f, 0.18f};
constexpr GLfloat kBladeColors[][3] = {
    {0.92f, 0.12f, 0.12f},
    {0.10f, 0.40f, 0.98f},
    {1.00f, 0.48f, 0.05f},
    {0.06f, 0.78f, 0.30f},
};
constexpr GLfloat kHighlightColor[] = {1.00f, 0.73f, 0.08f};
} // namespace

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* manager) : assetManager(manager)
{
#else
Fan::Fan()
{
#endif
    modelType = FanType;
    for (glm::mat4& matrix : partWorldMatrices) matrix = glm::mat4(1.0f);
}

Fan::~Fan()
{
    if (ibo) glDeleteBuffers(1, &ibo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
}

void Fan::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif
    if (!program) {
        LOGE("Fan shader program could not be created");
        return;
    }

    mvpUniform = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    modelUniform = glGetUniformLocation(program, "MODELMATRIX");
    normalUniform = glGetUniformLocation(program, "NORMALMATRIX");
    partColorUniform = glGetUniformLocation(program, "PARTCOLOR");
    lightPositionUniform = glGetUniformLocation(program, "LIGHTPOSITION");
    cameraPositionUniform = glGetUniformLocation(program, "CAMERAPOSITION");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    constexpr GLsizeiptr positionBytes = sizeof(kCubePositions);
    constexpr GLsizeiptr normalBytes = sizeof(kCubeNormals);
    glBufferData(GL_ARRAY_BUFFER, positionBytes + normalBytes, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionBytes, kCubePositions);
    glBufferSubData(GL_ARRAY_BUFFER, positionBytes, normalBytes, kCubeNormals);

    glEnableVertexAttribArray(kPositionAttribute);
    glVertexAttribPointer(kPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kNormalAttribute);
    glVertexAttribPointer(
        kNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void*>(static_cast<std::size_t>(positionBytes)));

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCubeIndices), kCubeIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Preserve the supplied Transform implementation while avoiding any
    // camera state inside the model itself.
    transform.TransformInit();
    LOGI("Fan initialized with flat normals and seven pickable parts");
}

void Fan::SetViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    viewMatrix = view;
    projectionMatrix = projection;
}

void Fan::SetLighting(const glm::vec3& light, const glm::vec3& camera)
{
    lightPosition = light;
    cameraPosition = camera;
}

void Fan::SetSpeed(float newSpeed)
{
    speed = std::clamp(newSpeed, 0.0f, 20.0f);
}

void Fan::Render()
{
    if (!program || !vao) return;

    spinAngle = std::fmod(spinAngle + speed, 360.0f);

    glUseProgram(program);
    glUniform3fv(lightPositionUniform, 1, glm::value_ptr(lightPosition));
    glUniform3fv(cameraPositionUniform, 1, glm::value_ptr(cameraPosition));
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f, 0.8f, 0.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(Base, kBaseColor);
    transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    drawPart(Pole, kPoleColor);
    transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.34f, 0.34f, 0.34f);
    drawPart(Hub, kHubColor);
    transform.TransformPopMatrix();

    constexpr float kBladeSpacing = 90.0f;
    for (int blade = 0; blade < 4; ++blade) {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(
            glm::radians(spinAngle + blade * kBladeSpacing), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.62f, 0.0f);
        transform.TransformScale(0.22f, 0.95f, 0.07f);
        drawPart(Blade0 + blade, kBladeColors[blade]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void Fan::drawPart(int part, const GLfloat color[3])
{
    const glm::mat4 model = *transform.TransformGetModelMatrix();
    const glm::mat4 mvp = projectionMatrix * viewMatrix * model;
    const glm::mat3 normal = glm::inverseTranspose(glm::mat3(model));
    partWorldMatrices[static_cast<std::size_t>(part)] = model;

    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalUniform, 1, GL_FALSE, glm::value_ptr(normal));
    glUniform3fv(partColorUniform, 1, part == selectedPart ? kHighlightColor : color);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
}

bool Fan::intersectUnitBox(const glm::vec3& worldOrigin,
                           const glm::vec3& worldDirection,
                           const glm::mat4& worldMatrix,
                           float& distance)
{
    const glm::mat4 inverse = glm::inverse(worldMatrix);
    const glm::vec3 origin = glm::vec3(inverse * glm::vec4(worldOrigin, 1.0f));
    const glm::vec3 direction = glm::vec3(inverse * glm::vec4(worldDirection, 0.0f));

    float nearT = 0.0f;
    float farT = std::numeric_limits<float>::max();
    for (int axis = 0; axis < 3; ++axis) {
        if (std::abs(direction[axis]) < 1.0e-6f) {
            if (origin[axis] < -0.5f || origin[axis] > 0.5f) return false;
            continue;
        }
        float first = (-0.5f - origin[axis]) / direction[axis];
        float second = (0.5f - origin[axis]) / direction[axis];
        if (first > second) std::swap(first, second);
        nearT = std::max(nearT, first);
        farT = std::min(farT, second);
        if (nearT > farT) return false;
    }
    distance = nearT;
    return farT >= 0.0f;
}

int Fan::PickRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                 float& nearestDistance) const
{
    int nearestPart = NoPart;
    nearestDistance = std::numeric_limits<float>::max();
    for (int part = 0; part < kPartCount; ++part) {
        float distance = 0.0f;
        if (intersectUnitBox(rayOrigin, rayDirection,
                             partWorldMatrices[static_cast<std::size_t>(part)], distance)
            && distance < nearestDistance) {
            nearestDistance = distance;
            nearestPart = part;
        }
    }
    return nearestPart;
}

void Fan::SelectPart(int part)
{
    selectedPart = (part >= 0 && part < kPartCount) ? part : NoPart;
}

const char* Fan::PartName(int part)
{
    static constexpr const char* kNames[] = {
        "base", "pole", "hub", "blade 1", "blade 2", "blade 3", "blade 4"
    };
    return part >= 0 && part < kPartCount ? kNames[part] : "none";
}

const char* Fan::SelectedPartName() const
{
    return PartName(selectedPart);
}
