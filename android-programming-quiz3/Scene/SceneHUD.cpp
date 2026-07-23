/**
 * @file SceneHUD.cpp
 * @brief Implements responsive HUD geometry, hit tests, and live UI feedback.
 * @author Zoey <t.zoey@digipen.edu>
 */

#define LOG_TAG "SceneHUD"
#include "SceneHUD.h"

#include "ShaderHelper.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace {
struct HudVertex {
    GLfloat position[2];
    GLfloat color[4];
};

using VertexList = std::vector<HudVertex>;

void addVertex(VertexList& vertices, float x, float y, const glm::vec4& color)
{
    vertices.push_back({{x, y}, {color.r, color.g, color.b, color.a}});
}

void addRect(VertexList& vertices, float left, float top, float right, float bottom,
             const glm::vec4& color)
{
    addVertex(vertices, left, top, color);
    addVertex(vertices, right, top, color);
    addVertex(vertices, right, bottom, color);
    addVertex(vertices, left, top, color);
    addVertex(vertices, right, bottom, color);
    addVertex(vertices, left, bottom, color);
}

std::array<std::uint8_t, 7> glyph(char character)
{
    switch (character) {
        case '+': return {0x04, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x04};
        case '-': return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
        case '1': return {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
        case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
        case '3': return {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
        case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
        case 'A': return {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
        case 'B': return {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
        case 'C': return {0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F};
        case 'H': return {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
        case 'I': return {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
        case 'K': return {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
        case 'L': return {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
        case 'N': return {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
        case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
        case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
        case 'D': return {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
        case 'Z': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
        case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
        case 'M': return {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
        case 'E': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
        case 'U': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
        default:  return {0, 0, 0, 0, 0, 0, 0};
    }
}

float textWidth(const std::string& text, float pixelSize)
{
    return text.empty() ? 0.0f : (static_cast<float>(text.size()) * 6.0f - 1.0f) * pixelSize;
}

void addText(VertexList& vertices, const std::string& text, float left, float top,
             float pixelSize, const glm::vec4& color)
{
    float cursor = left;
    for (char character : text) {
        const auto rows = glyph(character);
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 5; ++column) {
                if ((rows[static_cast<std::size_t>(row)] & (1u << (4 - column))) != 0u) {
                    const float x = cursor + static_cast<float>(column) * pixelSize;
                    const float y = top + static_cast<float>(row) * pixelSize;
                    addRect(vertices, x, y, x + pixelSize, y + pixelSize, color);
                }
            }
        }
        cursor += 6.0f * pixelSize;
    }
}

constexpr glm::vec4 kWhite(0.96f, 0.98f, 1.0f, 1.0f);
constexpr glm::vec4 kShadow(0.0f, 0.0f, 0.0f, 0.34f);
constexpr glm::vec4 kIdleSpeed(0.05f, 0.42f, 0.50f, 0.94f);
constexpr glm::vec4 kIdleZoom(0.20f, 0.27f, 0.58f, 0.94f);
constexpr glm::vec4 kActive(1.0f, 0.58f, 0.05f, 0.98f);
constexpr glm::vec4 kBarOn(0.10f, 0.90f, 0.42f, 1.0f);
constexpr glm::vec4 kBarOff(0.22f, 0.27f, 0.34f, 0.94f);
constexpr std::array<glm::vec4, 7> kPickColors = {
    glm::vec4(0.47f, 0.27f, 0.10f, 0.96f),
    glm::vec4(0.56f, 0.59f, 0.64f, 0.96f),
    glm::vec4(0.10f, 0.13f, 0.18f, 0.96f),
    glm::vec4(0.92f, 0.12f, 0.12f, 0.96f),
    glm::vec4(0.10f, 0.40f, 0.98f, 0.96f),
    glm::vec4(1.00f, 0.48f, 0.05f, 0.96f),
    glm::vec4(0.06f, 0.78f, 0.30f, 0.96f),
};
constexpr std::array<const char*, 7> kPickLabels = {
    "PICK BASE", "PICK POLE", "PICK HUB", "PICK BLADE 1",
    "PICK BLADE 2", "PICK BLADE 3", "PICK BLADE 4",
};
} // namespace

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* manager) : assetManager(manager)
{
#else
SceneHUD::SceneHUD()
{
#endif
    modelType = SceneHUDType;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    rebuildLayout(screenWidth, screenHeight);
}

SceneHUD::~SceneHUD()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
}

void SceneHUD::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("HUDVertex.glsl", "HUDFragment.glsl");
#endif
    if (!program) {
        LOGE("HUD shader program could not be created");
        return;
    }

    projectionUniform = glGetUniformLocation(program, "PROJECTIONMATRIX");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex),
        reinterpret_cast<const void*>(offsetof(HudVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, sizeof(HudVertex),
        reinterpret_cast<const void*>(offsetof(HudVertex, color)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    LOGI("HUD initialized: four buttons and a 20-block speed bar");
}

void SceneHUD::rebuildLayout(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    const float minimumSide = static_cast<float>(std::min(width, height));
    const float margin = std::clamp(minimumSide * 0.035f, 12.0f, 28.0f);
    const float buttonWidth =
        std::clamp(static_cast<float>(width) * 0.18f, 90.0f, 145.0f);
    const float buttonHeight = std::clamp(minimumSide * 0.075f, 44.0f, 58.0f);
    const float gap = std::clamp(minimumSide * 0.022f, 10.0f, 18.0f);
    const float right = static_cast<float>(width) - margin - buttonWidth;

    buttons[SpeedUp] = {margin, margin, margin + buttonWidth, margin + buttonHeight};
    buttons[SpeedDown] = {
        margin, margin + buttonHeight + gap,
        margin + buttonWidth, margin + buttonHeight * 2.0f + gap};
    buttons[ZoomIn] = {
        right, margin, right + buttonWidth, margin + buttonHeight};
    buttons[ZoomOut] = {
        right, margin + buttonHeight + gap,
        right + buttonWidth, margin + buttonHeight * 2.0f + gap};
}

void SceneHUD::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    // Pixel coordinates, top-left origin, Y growing down with mouse/touch.
    transform.TransformOrtho(
        0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    rebuildLayout(width, height);
}

void SceneHUD::Render()
{
    if (!program || !vao) return;

    VertexList vertices;
    vertices.reserve(2400);
    static constexpr const char* kLabels[] = {"+SPD", "-SPD", "+ZOOM", "-ZOOM"};
    const float margin = std::clamp(
        static_cast<float>(std::min(screenWidth, screenHeight)) * 0.035f, 12.0f, 28.0f);

    for (int index = 0; index < ButtonCount; ++index) {
        const Rect& rect = buttons[static_cast<std::size_t>(index)];
        const glm::vec4 fill = index == activeButton
            ? kActive
            : (index < ZoomIn ? kIdleSpeed : kIdleZoom);
        addRect(vertices, rect.left + 4.0f, rect.top + 5.0f,
                rect.right + 4.0f, rect.bottom + 5.0f, kShadow);
        addRect(vertices, rect.left, rect.top, rect.right, rect.bottom, kWhite);
        addRect(vertices, rect.left + 2.0f, rect.top + 2.0f,
                rect.right - 2.0f, rect.bottom - 2.0f, fill);

        const std::string label(kLabels[index]);
        const float pixelSize = (rect.right - rect.left) < 115.0f ? 2.0f : 3.0f;
        const float labelX =
            rect.left + ((rect.right - rect.left) - textWidth(label, pixelSize)) * 0.5f;
        const float labelY =
            rect.top + ((rect.bottom - rect.top) - 7.0f * pixelSize) * 0.5f;
        addText(vertices, label, labelX, labelY, pixelSize, kWhite);
    }

    // Bonus feedback: report the selected 3D part in a colour-coded HUD strip.
    // The label is built from the same raw pixel quads as the button captions.
    const float statusWidth =
        std::clamp(static_cast<float>(screenWidth) * 0.28f, 160.0f, 260.0f);
    const float statusHeight = 40.0f;
    const float statusLeft = (static_cast<float>(screenWidth) - statusWidth) * 0.5f;
    const float statusTop = margin;
    const bool hasPick = pickedPart >= 0 && pickedPart < 7;
    const glm::vec4 statusColor =
        hasPick ? kPickColors[static_cast<std::size_t>(pickedPart)] : kBarOff;
    const std::string statusLabel =
        hasPick ? kPickLabels[static_cast<std::size_t>(pickedPart)] : "NO PICK";
    addRect(vertices, statusLeft + 4.0f, statusTop + 5.0f,
            statusLeft + statusWidth + 4.0f, statusTop + statusHeight + 5.0f, kShadow);
    addRect(vertices, statusLeft, statusTop,
            statusLeft + statusWidth, statusTop + statusHeight, kWhite);
    addRect(vertices, statusLeft + 2.0f, statusTop + 2.0f,
            statusLeft + statusWidth - 2.0f, statusTop + statusHeight - 2.0f, statusColor);
    const float statusPixelSize = statusLabel.size() > 10 ? 2.0f : 3.0f;
    addText(
        vertices, statusLabel,
        statusLeft + (statusWidth - textWidth(statusLabel, statusPixelSize)) * 0.5f,
        statusTop + (statusHeight - 7.0f * statusPixelSize) * 0.5f,
        statusPixelSize, kWhite);

    // The specification fixes each indicator segment at exactly 10 x 20 px.
    constexpr float blockWidth = 10.0f;
    constexpr float blockHeight = 20.0f;
    const float blockGap = std::clamp(
        (static_cast<float>(screenWidth) - 2.0f * margin - 20.0f * blockWidth) / 19.0f,
        1.0f, 4.0f);
    const float barWidth = 20.0f * blockWidth + 19.0f * blockGap;
    const float barLeft = (static_cast<float>(screenWidth) - barWidth) * 0.5f;
    const float barTop = static_cast<float>(screenHeight) - margin - blockHeight;

    constexpr float speedLabelPixel = 2.0f;
    const std::string speedLabel("SPEED");
    addText(vertices, speedLabel,
            (static_cast<float>(screenWidth) - textWidth(speedLabel, speedLabelPixel)) * 0.5f,
            barTop - 22.0f, speedLabelPixel, kWhite);

    for (int block = 0; block < 20; ++block) {
        const float left = barLeft + static_cast<float>(block) * (blockWidth + blockGap);
        addRect(vertices, left, barTop, left + blockWidth, barTop + blockHeight,
                block < speed ? kBarOn : kBarOff);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // The top-left orthographic projection reverses winding relative to the
    // 3D pass, so screen-space quads must not inherit Fan's back-face culling.
    glDisable(GL_CULL_FACE);
    glUseProgram(program);
    glUniformMatrix4fv(
        projectionUniform, 1, GL_FALSE,
        glm::value_ptr(*transform.TransformGetProjectionMatrix()));
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(HudVertex)),
        vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

bool SceneHUD::applyAction(int button)
{
    const int oldSpeed = speed;
    const int oldZoomSteps = pendingZoomSteps;
    switch (button) {
        case SpeedUp:
            speed = std::min(speed + 1, 20);
            LOGI("+SPD pressed: %d/20", speed);
            break;
        case SpeedDown:
            speed = std::max(speed - 1, 0);
            LOGI("-SPD pressed: %d/20", speed);
            break;
        case ZoomIn:
            --pendingZoomSteps;
            LOGI("+ZOOM pressed");
            break;
        case ZoomOut:
            ++pendingZoomSteps;
            LOGI("-ZOOM pressed");
            break;
        default:
            return false;
    }
    return oldSpeed != speed || oldZoomSteps != pendingZoomSteps;
}

bool SceneHUD::TouchDown(float x, float y)
{
    for (int button = 0; button < ButtonCount; ++button) {
        if (buttons[static_cast<std::size_t>(button)].contains(x, y)) {
            activeButton = button;
            pressStart = std::chrono::steady_clock::now();
            applyAction(button);
            return true;
        }
    }
    activeButton = -1;
    return false;
}

void SceneHUD::TouchRelease()
{
    activeButton = -1;
}

bool SceneHUD::UpdateHeldAction()
{
    if (activeButton < 0) return false;
    const auto now = std::chrono::steady_clock::now();
    const auto held = std::chrono::duration_cast<std::chrono::milliseconds>(now - pressStart);
    if (held.count() < 350) return false;
    // Once the deliberate initial hold is established, use the level-triggered
    // Scene Graph HUD pattern required by the bonus: one action every frame.
    return applyAction(activeButton);
}

int SceneHUD::ConsumeZoomSteps()
{
    const int steps = pendingZoomSteps;
    pendingZoomSteps = 0;
    return steps;
}

void SceneHUD::SetPickedPart(int part)
{
    pickedPart = part >= 0 && part < 7 ? part : -1;
}

glm::vec2 SceneHUD::ButtonCenter(int button) const
{
    if (button < 0 || button >= ButtonCount) return glm::vec2(-1.0f);
    const Rect& rect = buttons[static_cast<std::size_t>(button)];
    return glm::vec2((rect.left + rect.right) * 0.5f, (rect.top + rect.bottom) * 0.5f);
}
