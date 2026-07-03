#define LOG_TAG "Square"

/**
 * Square.cpp
 *
 * Independent square Model with its own shader program, VAO, and VBOs.
 */

#include "Square.h"
#include "ShaderHelper.h"

static constexpr GLfloat kSquareCenterX = 0.90f;
static constexpr GLfloat kSquareCenterY = 0.56f;
static constexpr GLfloat kSquareSizeOfShortSide = 0.20f;

static const GLfloat kSquareColors[] = {
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f
};

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Square::Square() = default;
#endif

Square::~Square()
{
    glDeleteBuffers(1, &vboColor);
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

void Square::InitModel()
{
    LOGI("Square::InitModel");

#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/BlueTriangleVertex.glsl",
        "shader/BlueTriangleFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile(
        "BlueTriangleVertex.glsl",
        "BlueTriangleFragment.glsl");
#endif

    if (!programID) {
        LOGE("Square: could not create program");
        return;
    }

    uRadianAngle = glGetUniformLocation(programID, "RadianAngle");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kSquareColors), kSquareColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    updateGeometry(800, 600);

    LOGI("Square::InitModel done");
}

void Square::Render()
{
    if (!programID) {
        return;
    }

    glUseProgram(programID);
    glUniform1f(uRadianAngle, 0.0f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Square::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
    updateGeometry(w, h);
}

void Square::updateGeometry(int w, int h)
{
    if (!vboPos || w <= 0 || h <= 0) {
        return;
    }

    const float shortSide = static_cast<float>(w < h ? w : h);
    const float halfW = (kSquareSizeOfShortSide * shortSide / static_cast<float>(w)) * 0.5f;
    const float halfH = (kSquareSizeOfShortSide * shortSide / static_cast<float>(h)) * 0.5f;

    const GLfloat positions[] = {
        kSquareCenterX - halfW, kSquareCenterY + halfH,
        kSquareCenterX + halfW, kSquareCenterY + halfH,
        kSquareCenterX + halfW, kSquareCenterY - halfH,
        kSquareCenterX - halfW, kSquareCenterY - halfH
    };

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
