/**
 * @file Ground.h
 * @brief Declares the Phong-lit procedural checkerboard ground model.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

/**
 * Ground owns one large world-space quad.
 *
 * Its fragment shader generates the checkerboard procedurally from world X/Z
 * coordinates and applies the same ambient/diffuse/specular lighting model as
 * the fan. No texture or image asset is used.
 */

#include "Model.h"
#include "Platform.h"

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground final : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* assetManager);
#else
    Ground();
#endif
    ~Ground() override;

    void InitModel() override;
    void Render() override;

    void SetViewProjection(const glm::mat4& view, const glm::mat4& projection);
    void SetLighting(const glm::vec3& lightPosition, const glm::vec3& cameraPosition);

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    glm::vec3 lightPosition{4.0f, 3.5f, 5.0f};
    glm::vec3 cameraPosition{0.0f, 1.8f, 8.0f};

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint mvpUniform = -1;
    GLint modelUniform = -1;
    GLint normalUniform = -1;
    GLint lightPositionUniform = -1;
    GLint cameraPositionUniform = -1;
};
