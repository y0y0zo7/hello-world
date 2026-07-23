/**
 * @file SceneHUD.h
 * @brief Declares the orthographic controls, speed bar, and pick-status HUD.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

/**
 * SceneHUD owns screen-space UI state and geometry only.
 *
 * Four pixel-space button rectangles and a 20-block speed indicator are
 * rendered with an orthographic projection. Input is tested directly against
 * those rectangles before Renderer permits a 3D pick. Zoom presses are emitted
 * as signed actions so the camera distance remains owned exclusively by
 * Scene3D. A colour-coded strip reports the currently picked fan part.
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"

#include <array>
#include <chrono>

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class SceneHUD final : public Model {
public:
    enum Button {
        SpeedUp = 0,
        SpeedDown,
        ZoomIn,
        ZoomOut,
        ButtonCount
    };

#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* assetManager);
#else
    SceneHUD();
#endif
    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int width, int height) override;

    bool TouchDown(float x, float y);
    void TouchRelease();
    bool UpdateHeldAction();

    int CurrentSpeed() const { return speed; }
    int ConsumeZoomSteps();
    void SetPickedPart(int part);
    int PickedPart() const { return pickedPart; }
    int ActiveButton() const { return activeButton; }
    glm::vec2 ButtonCenter(int button) const;

private:
    struct Rect {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;

        bool contains(float x, float y) const
        {
            return x >= left && x <= right && y >= top && y <= bottom;
        }
    };

    void rebuildLayout(int width, int height);
    bool applyAction(int button);

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    Transform transform;
    std::array<Rect, ButtonCount> buttons{};
    int screenWidth = 800;
    int screenHeight = 700;

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint projectionUniform = -1;

    int speed = 8;
    int pendingZoomSteps = 0;
    int pickedPart = -1;
    int activeButton = -1;
    std::chrono::steady_clock::time_point pressStart{};
};
