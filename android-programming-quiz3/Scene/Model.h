/**
 * @file Model.h
 * @brief Defines the shared lifecycle interface for renderable scene objects.
 * @author Zoey <t.zoey@digipen.edu>
 */

#pragma once

enum ModelType {
    FanType = 0,
    GroundType,
    Scene3DType,
    SceneHUDType
};

class Model {
public:
    virtual ~Model() = default;

    virtual void InitModel() = 0;
    virtual void Render() = 0;
    virtual void Resize(int, int) {}

    virtual void TouchEventDown(float, float) {}
    virtual void TouchEventMove(float, float) {}
    virtual void TouchEventRelease(float, float) {}

    ModelType GetModelType() const { return modelType; }

protected:
    ModelType modelType = FanType;
};
