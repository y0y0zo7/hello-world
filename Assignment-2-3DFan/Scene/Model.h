#pragma once

enum ModelType {
    FanType = 0
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
