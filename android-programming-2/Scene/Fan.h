#pragma once

/**
 * Fan.h
 *
 * A hierarchical table fan built from one indexed cube. The world transform is
 * shared by seven push/pop branches: base, pole, hub, and four blades. Each
 * branch uploads its own PARTCOLOR uniform before drawing the same VAO.
 *
 * A tap toggles the fan and logs Fan ON/Fan OFF. During a drag, movement speed
 * is measured with std::chrono and converted to a clamped spin boost. Releasing
 * the pointer removes the boost and returns the blades to their base speed.
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"

#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan final : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetManager);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int width, int height) override;

    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

    // Read-only state used by the headless interaction tests.
    bool IsFanOn() const { return fanOn; }
    float GetDragBoost() const { return dragBoost; }
    float GetMovedDistance() const { return movedDistance; }

private:
    void drawPart(const GLfloat color[3]);

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    Transform transform;

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLint mvpUniform = -1;
    GLint partColorUniform = -1;

    float spinAngle = 0.0f;
    bool fanOn = true;
    float dragBoost = 0.0f;

    static constexpr float kBaseSpeed = 1.5f;
    static constexpr float kMaxBoost = 20.0f;
    static constexpr float kBoostScale = 8.0f;
    static constexpr float kTapThreshold = 12.0f;

    float lastX = 0.0f;
    float lastY = 0.0f;
    float movedDistance = 0.0f;
    bool pointerDown = false;
    std::chrono::steady_clock::time_point lastMoveTime{};
};
