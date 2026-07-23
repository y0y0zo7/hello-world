/**
 * @file Scene3D.cpp
 * @brief Implements camera updates, 3D rendering, and click-ray construction.
 * @author Zoey <t.zoey@digipen.edu>
 */

#define LOG_TAG "Scene3D"
#include "Scene3D.h"

#include "Fan.h"
#include "Ground.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* manager) : assetManager(manager)
{
    fan = new Fan(assetManager);
    ground = new Ground(assetManager);
#else
Scene3D::Scene3D()
{
    fan = new Fan();
    ground = new Ground();
#endif
    modelType = Scene3DType;
    updateCamera();
}

Scene3D::~Scene3D()
{
    delete ground;
    delete fan;
}

void Scene3D::InitModel()
{
    if (ground) ground->InitModel();
    if (fan) fan->InitModel();
    updateCamera();
}

void Scene3D::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    screenWidth = width;
    screenHeight = height;
    projectionMatrix = glm::perspective(
        glm::radians(55.0f), static_cast<float>(width) / static_cast<float>(height),
        0.1f, 100.0f);
    updateCamera();
}

void Scene3D::SetCameraDistance(float distance)
{
    cameraDistance = std::clamp(distance, 4.0f, 18.0f);
    updateCamera();
}

void Scene3D::AdjustCameraDistance(float delta)
{
    SetCameraDistance(cameraDistance + delta);
    LOGI("Camera distance: %.2f", cameraDistance);
}

void Scene3D::SetFanSpeed(float speed)
{
    if (fan) fan->SetSpeed(speed);
}

int Scene3D::SelectedPart() const
{
    return fan ? fan->GetSelectedPart() : Fan::NoPart;
}

void Scene3D::updateCamera()
{
    cameraPosition = glm::vec3(0.0f, 1.7f, cameraDistance);
    const glm::vec3 target(0.0f, -0.35f, 0.0f);
    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    viewMatrix = glm::lookAt(cameraPosition, target, up);

    if (fan) {
        fan->SetViewProjection(viewMatrix, projectionMatrix);
        fan->SetLighting(lightPosition, cameraPosition);
    }
    if (ground) {
        ground->SetViewProjection(viewMatrix, projectionMatrix);
        ground->SetLighting(lightPosition, cameraPosition);
    }
}

void Scene3D::Render()
{
    // Ground first so the fan naturally depth-tests over it.
    if (ground) ground->Render();
    if (fan) fan->Render();
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    if (!fan || screenWidth <= 0 || screenHeight <= 0) return false;

    int viewport[4] = {0, 0, screenWidth, screenHeight};
    const float glY = static_cast<float>(screenHeight) - screenY;
    glm::vec3 nearPoint;
    glm::vec3 farPoint;

    const bool nearOk = unprojectTransform.TransformUnproject(
        screenX, glY, 0.0f, &viewMatrix, &projectionMatrix, viewport,
        &nearPoint.x, &nearPoint.y, &nearPoint.z) != 0;
    const bool farOk = unprojectTransform.TransformUnproject(
        screenX, glY, 1.0f, &viewMatrix, &projectionMatrix, viewport,
        &farPoint.x, &farPoint.y, &farPoint.z) != 0;
    if (!nearOk || !farOk) {
        fan->SelectPart(Fan::NoPart);
        return false;
    }

    const glm::vec3 rayDirection = glm::normalize(farPoint - nearPoint);
    float distance = 0.0f;
    const int hit = fan->PickRay(nearPoint, rayDirection, distance);

    // Clicking the selected part again, or clicking empty space, clears it.
    const int nextSelection =
        (hit == Fan::NoPart || hit == fan->GetSelectedPart()) ? Fan::NoPart : hit;
    fan->SelectPart(nextSelection);
    LOGI("Pick: %s", fan->SelectedPartName());
    return hit != Fan::NoPart;
}
