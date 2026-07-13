#define LOG_TAG "Fan3D"
#include "Fan.h"

#include "ShaderHelper.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include <glm/gtc/type_ptr.hpp>

namespace {
constexpr GLuint kPositionAttribute = 0;
constexpr GLuint kShadeAttribute = 1;

// Eight corners of the one shared unit cube.
constexpr GLfloat kCubePositions[] = {
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
};

// The front is bright and the back is darker, giving every part depth.
constexpr GLfloat kCornerShades[] = {
    1.0f, 1.0f, 1.0f, 1.0f,
    0.6f, 0.6f, 0.6f, 0.6f,
};

constexpr GLushort kCubeIndices[] = {
    0, 1, 2,  0, 2, 3,  // front
    1, 5, 6,  1, 6, 2,  // right
    5, 4, 7,  5, 7, 6,  // back
    4, 0, 3,  4, 3, 7,  // left
    3, 2, 6,  3, 6, 7,  // top
    4, 5, 1,  4, 1, 0,  // bottom
};

constexpr GLfloat kBaseColor[] = {0.45f, 0.28f, 0.12f};
constexpr GLfloat kPoleColor[] = {0.55f, 0.55f, 0.58f};
constexpr GLfloat kHubColor[] = {0.20f, 0.20f, 0.22f};
constexpr GLfloat kBladeColors[][3] = {
    {0.92f, 0.12f, 0.12f},
    {0.12f, 0.38f, 0.95f},
    {1.00f, 0.48f, 0.08f},
    {0.08f, 0.78f, 0.28f},
};
} // namespace

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* manager) : assetManager(manager)
{
#else
Fan::Fan()
{
#endif
    modelType = FanType;
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
    partColorUniform = glGetUniformLocation(program, "PARTCOLOR");
    if (mvpUniform < 0 || partColorUniform < 0) {
        LOGE("Fan shader uniforms are missing");
        return;
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // One VBO, allocated once, with positions and shades in two sub-regions.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    constexpr GLsizeiptr positionBytes = sizeof(kCubePositions);
    constexpr GLsizeiptr shadeBytes = sizeof(kCornerShades);
    glBufferData(GL_ARRAY_BUFFER, positionBytes + shadeBytes, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionBytes, kCubePositions);
    glBufferSubData(GL_ARRAY_BUFFER, positionBytes, shadeBytes, kCornerShades);

    glEnableVertexAttribArray(kPositionAttribute);
    glVertexAttribPointer(kPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kShadeAttribute);
    glVertexAttribPointer(kShadeAttribute, 1, GL_FLOAT, GL_FALSE, 0,
                          reinterpret_cast<const void*>(static_cast<std::size_t>(positionBytes)));

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCubeIndices), kCubeIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    transform.TransformInit();
    glEnable(GL_DEPTH_TEST);
    LOGI("Fan model initialized: one cube, one VAO, seven draw calls per frame");
}

void Fan::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    const float aspect = static_cast<float>(width) / static_cast<float>(height);

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);

    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();
}

void Fan::Render()
{
    if (!program || !vao) return;

    if (fanOn) {
        spinAngle += kBaseSpeed + dragBoost;
        if (spinAngle >= 360.0f) spinAngle -= 360.0f;
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(kBaseColor);
    transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    drawPart(kPoleColor);
    transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(kHubColor);
    transform.TransformPopMatrix();

    for (int blade = 0; blade < 4; ++blade) {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + blade * 90.0f), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        drawPart(kBladeColors[blade]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void Fan::drawPart(const GLfloat color[3])
{
    const glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniform3fv(partColorUniform, 1, color);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
}

void Fan::TouchEventDown(float x, float y)
{
    pointerDown = true;
    lastX = x;
    lastY = y;
    movedDistance = 0.0f;
    dragBoost = 0.0f;
    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    if (!pointerDown) return;

    const float dx = x - lastX;
    const float dy = y - lastY;
    const float distance = std::sqrt(dx * dx + dy * dy);
    movedDistance += distance;

    const auto now = std::chrono::steady_clock::now();
    const float elapsedMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();
    const float velocity = elapsedMs > 0.0f ? distance / elapsedMs : 0.0f;
    dragBoost = std::min(velocity * kBoostScale, kMaxBoost);

    lastX = x;
    lastY = y;
    lastMoveTime = now;
    LOGI("Swipe velocity %.3f px/ms, boost %.2f deg/frame", velocity, dragBoost);
}

void Fan::TouchEventRelease(float x, float y)
{
    if (!pointerDown) return;

    // Include any final distance between the last move event and pointer-up.
    const float dx = x - lastX;
    const float dy = y - lastY;
    movedDistance += std::sqrt(dx * dx + dy * dy);

    pointerDown = false;
    dragBoost = 0.0f;
    if (movedDistance < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
}
