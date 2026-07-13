#define LOG_TAG "Fan3D"
#include "Renderer.h"

#include "Fan.h"

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
    for (Model* model : models) {
        model->InitModel();
    }
    return !models.empty();
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    models.push_back(new Fan(assetManager));
#else
    models.push_back(new Fan());
#endif
    LOGI("Renderer created %zu model(s)", models.size());
}

void Renderer::clearModels()
{
    for (Model* model : models) {
        delete model;
    }
    models.clear();
}

void Renderer::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    glViewport(0, 0, width, height);
    for (Model* model : models) {
        model->Resize(width, height);
    }
    LOGI("Viewport %d x %d", width, height);
}

void Renderer::render()
{
    glClearColor(0.025f, 0.035f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (Model* model : models) {
        model->Render();
    }
}

void Renderer::TouchEventDown(float x, float y)
{
    for (Model* model : models) model->TouchEventDown(x, y);
}

void Renderer::TouchEventMove(float x, float y)
{
    for (Model* model : models) model->TouchEventMove(x, y);
}

void Renderer::TouchEventRelease(float x, float y)
{
    for (Model* model : models) model->TouchEventRelease(x, y);
}
