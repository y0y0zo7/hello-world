#pragma once

#include "Model.h"
#include "Platform.h"

#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

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

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif
    std::vector<Model*> models;
};
