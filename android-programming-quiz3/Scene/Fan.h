/**
 * @file Fan.h
 * @brief Declares the hierarchical, Phong-lit, and pickable fan model.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

/**
 * Fan owns the Quiz 2 hierarchical fan geometry and animation only.
 *
 * Scene3D supplies camera/light state and performs input routing. The fan
 * caches the world matrix for its base, pole, hub, and four blades so Scene3D
 * can perform accurate ray-vs-oriented-box picking without mixing HUD logic
 * into the model.
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"

#include <array>

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan final : public Model {
public:
    static constexpr int kPartCount = 7;

    enum Part {
        Base = 0,
        Pole,
        Hub,
        Blade0,
        Blade1,
        Blade2,
        Blade3,
        NoPart = -1
    };

#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetManager);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;

    void SetViewProjection(const glm::mat4& view, const glm::mat4& projection);
    void SetLighting(const glm::vec3& lightPosition, const glm::vec3& cameraPosition);
    void SetSpeed(float speed);

    int PickRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                float& nearestDistance) const;
    void SelectPart(int part);

    float GetSpeed() const { return speed; }
    int GetSelectedPart() const { return selectedPart; }
    const char* SelectedPartName() const;
    static const char* PartName(int part);

private:
    void drawPart(int part, const GLfloat color[3]);
    static bool intersectUnitBox(const glm::vec3& rayOrigin,
                                 const glm::vec3& rayDirection,
                                 const glm::mat4& worldMatrix,
                                 float& distance);

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    Transform transform;
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    glm::vec3 lightPosition{4.0f, 3.5f, 5.0f};
    glm::vec3 cameraPosition{0.0f, 1.8f, 8.0f};
    std::array<glm::mat4, kPartCount> partWorldMatrices{};

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLint mvpUniform = -1;
    GLint modelUniform = -1;
    GLint normalUniform = -1;
    GLint partColorUniform = -1;
    GLint lightPositionUniform = -1;
    GLint cameraPositionUniform = -1;

    float spinAngle = 0.0f;
    float speed = 8.0f;
    int selectedPart = NoPart;
};
