/**
 * @file Renderer.cpp
 * @brief Implements fixed render ordering and HUD-first input routing.
 * @author Zoey <t.zoey@digipen.edu>
 */

#define LOG_TAG "FanDashboardHUD"
#include "Renderer.h"

#include "Scene3D.h"
#include "SceneHUD.h"

Renderer& Renderer::Instance()
{
    static Renderer renderer;
    return renderer;
}

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* manager)
{
    assetManager = manager;
}
#endif

bool Renderer::initializeRenderer()
{
    createModels();
    if (!scene3D || !sceneHUD) return false;
    scene3D->InitModel();
    sceneHUD->InitModel();
    syncHudState();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D = new Scene3D(assetManager);
    sceneHUD = new SceneHUD(assetManager);
#else
    scene3D = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
    LOGI("Renderer created Scene3D and SceneHUD");
}

void Renderer::clearModels()
{
    delete sceneHUD;
    delete scene3D;
    sceneHUD = nullptr;
    scene3D = nullptr;
}

void Renderer::syncHudState()
{
    if (!scene3D || !sceneHUD) return;
    scene3D->SetFanSpeed(static_cast<float>(sceneHUD->CurrentSpeed()));
    const int zoomSteps = sceneHUD->ConsumeZoomSteps();
    if (zoomSteps != 0) {
        scene3D->AdjustCameraDistance(static_cast<float>(zoomSteps) * 0.75f);
    }
}

void Renderer::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    glViewport(0, 0, width, height);
    if (scene3D) scene3D->Resize(width, height);
    if (sceneHUD) sceneHUD->Resize(width, height);
    LOGI("Viewport %d x %d", width, height);
}

void Renderer::render()
{
    glClearColor(0.025f, 0.035f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (sceneHUD && sceneHUD->UpdateHeldAction()) syncHudState();

    // Required mixed-scene pipeline: 3D first with depth, then the HUD on top.
    glEnable(GL_DEPTH_TEST);
    if (scene3D) scene3D->Render();
    glDisable(GL_DEPTH_TEST);
    if (sceneHUD) sceneHUD->Render();
    glEnable(GL_DEPTH_TEST);
}

void Renderer::TouchEventDown(float x, float y)
{
    // Screen-space controls have priority and consume their presses.
    if (sceneHUD && sceneHUD->TouchDown(x, y)) {
        syncHudState();
        return;
    }
    if (scene3D) {
        scene3D->PickAt(x, y);
        if (sceneHUD) sceneHUD->SetPickedPart(scene3D->SelectedPart());
    }
}

void Renderer::TouchEventMove(float, float)
{
    // Quiz 2's drag-to-boost interaction is intentionally retired.
}

void Renderer::TouchEventRelease(float, float)
{
    if (sceneHUD) sceneHUD->TouchRelease();
}
