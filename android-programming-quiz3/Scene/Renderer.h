/**
 * @file Renderer.h
 * @brief Declares the cross-platform coordinator for Scene3D and SceneHUD.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* manager);
#endif

    bool initializeRenderer();
    void resize(int width, int height);
    void render();

    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer() = default;
    ~Renderer();

    void createModels();
    void clearModels();
    void syncHudState();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif
    Scene3D* scene3D = nullptr;
    SceneHUD* sceneHUD = nullptr;
};
