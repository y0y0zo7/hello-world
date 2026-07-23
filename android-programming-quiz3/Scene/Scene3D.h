/**
 * @file Scene3D.h
 * @brief Declares the perspective scene, camera, lighting, and picking owner.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

/**
 * Scene3D owns all perspective-space state: Fan, Ground, camera, light, and
 * ray picking. It contains no HUD geometry or 2D hit testing.
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan;
class Ground;

class Scene3D final : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* assetManager);
#else
    Scene3D();
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render() override;
    void Resize(int width, int height) override;

    void SetCameraDistance(float distance);
    void AdjustCameraDistance(float delta);
    void SetFanSpeed(float speed);
    bool PickAt(float screenX, float screenY);

    float CameraDistance() const { return cameraDistance; }
    int SelectedPart() const;
    const Fan* GetFan() const { return fan; }

private:
    void updateCamera();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    Fan* fan = nullptr;
    Ground* ground = nullptr;
    Transform unprojectTransform;

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    glm::vec3 cameraPosition{0.0f, 1.8f, 8.0f};
    // Low, camera-facing key light produces an unmistakable specular response
    // on the flat fan faces while still shading the checkerboard naturally.
    glm::vec3 lightPosition{4.0f, 3.5f, 5.0f};

    float cameraDistance = 8.0f;
    int screenWidth = 800;
    int screenHeight = 700;
};
